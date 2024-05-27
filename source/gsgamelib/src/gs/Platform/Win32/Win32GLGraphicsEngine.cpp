#include "Win32GLGraphicsEngine.h"
#include "Win32MonitorInfo.h"
#include "BasicWin32Window.h"
#include "gs/Platform/GL/GLHeaders.h"
#include <cassert>

static Win32GLGraphicsEngine* g_pWin32GLGraphicsEngine = nullptr;

static void SetVSyncState(bool bEnable)
{
	typedef void (APIENTRY *PFNWGLEXTSWAPCONTROLPROC)(int);
	typedef int (*PFNWGLEXTGETSWAPINTERVALPROC)(void);

	static PFNWGLEXTSWAPCONTROLPROC wglSwapIntervalEXT = nullptr;
	static PFNWGLEXTGETSWAPINTERVALPROC wglGetSwapIntervalEXT = nullptr;

	if (wglSwapIntervalEXT == nullptr)
	{
		// Get extensions string from graphics card
		const char* extensions = (const char*)glGetString(GL_EXTENSIONS);

		// Is WGL_EXT_swap_control in the string? VSync switch possible?
		if (strstr(extensions, "WGL_EXT_swap_control"))
		{
			// Get address's of both functions and save them
			wglSwapIntervalEXT = (PFNWGLEXTSWAPCONTROLPROC)wglGetProcAddress("wglSwapIntervalEXT");
			wglGetSwapIntervalEXT = (PFNWGLEXTGETSWAPINTERVALPROC)wglGetProcAddress("wglGetSwapIntervalEXT");
		}
	}

	if (wglSwapIntervalEXT == nullptr)
		return;

	wglSwapIntervalEXT(bEnable? 1 : 0); // Interval 1 -> enable
}

static Win32Window* GetWin32Window()
{
	static BasicWin32Window s_window;
	return &s_window;
}

// Implement externed BasicWin32Window callback
void OnWindowResized(float32 newWidth, float32 newHeight)
{
	assert(g_pWin32GLGraphicsEngine);
	if (g_pWin32GLGraphicsEngine->m_windowResizedCallback)
	{
		g_pWin32GLGraphicsEngine->m_windowResizedCallback(*g_pWin32GLGraphicsEngine, newWidth, newHeight);
	}
	else
	{
		// If no user-custom callback, default is to maximize viewport size to window while
		// maintaining original aspect ratio

		const float32 aspectRatio = g_pWin32GLGraphicsEngine->m_initialWidth / g_pWin32GLGraphicsEngine->m_initialHeight;

		float32 x = 0;
		float32 y = 0;
		float32 viewWidth = newWidth;
		float32 viewHeight = newHeight;

		viewWidth = newHeight * aspectRatio;

		if (viewWidth > newWidth)
		{
			viewWidth = newWidth;
			viewHeight = newWidth / aspectRatio;
			y = (newHeight - viewHeight) / 2.f;
		}
		else
		{
			x = (newWidth - viewWidth) / 2.f;
		}

		Viewport vp(x, y, viewWidth, viewHeight);
		g_pWin32GLGraphicsEngine->SetActiveViewport(vp);

		// Use scissoring to make sure area outside of viewport in window is black
		glEnable(GL_SCISSOR_TEST);
		glScissor((int)x, (int)y, (int)viewWidth, (int)viewHeight);

	}
}


Win32GLGraphicsEngine::Win32GLGraphicsEngine()
: m_pWindow(nullptr)
{
}

void Win32GLGraphicsEngine::Initialize(const char* title, int width, int height, int bpp, ScreenMode::Type screenMode, VertSync::Type vertSync)
{
	g_pWin32GLGraphicsEngine = this;
	m_initialWidth = (float32)width;
	m_initialHeight = (float32)height;

	assert(m_pWindow == nullptr);
	m_pWindow = ::GetWin32Window();
	assert(m_pWindow);

	bool bFullScreen = (screenMode == ScreenMode::FullScreen);
	m_pWindow->Create(title, width, height, bpp, bFullScreen);

	// Get window device context
	m_hDC = m_pWindow->GetHDC();

	if (m_hDC == nullptr)
		throw std::runtime_error("Failed to retrieve HDC");

	// Set the window's pixel format
	PIXELFORMATDESCRIPTOR pfd = 
	{
		sizeof(PIXELFORMATDESCRIPTOR),	// Size of this pixel format descriptor
		1,								// Version number
		PFD_DRAW_TO_WINDOW |			// Format must support window
		PFD_SUPPORT_OPENGL |			// Format must support opengl
		PFD_DOUBLEBUFFER,				// Must support double buffering
		PFD_TYPE_RGBA,					// Request an rgba format
		static_cast<BYTE>(bpp),			// Select our color depth
		0, 0, 0, 0, 0, 0,				// Color bpp ignored
		0,								// No alpha buffer
		0,								// Shift bit ignored
		0,								// No accumulation buffer
		0, 0, 0, 0,						// Accumulation bpp ignored
		16,								// 16-bit z-buffer (depth buffer)  
		0,								// Stencil buffer depth
		0,								// Auxiliary buffer: not supported
		PFD_MAIN_PLANE,					// Layer type: ignored by OpenGL now
		0,								// Reserved
		0, 0, 0							// Layer masks ignored
	};

	GLuint pixFormat = ChoosePixelFormat(m_hDC, &pfd);

	if (!pixFormat)
		throw std::runtime_error("ChoosePixelFormat() failed");		

	if ( !SetPixelFormat(m_hDC, pixFormat, &pfd) )
		throw std::runtime_error("SetPixelFormat() failed");

	// Create an OpenGL rendering context
	m_hRC = wglCreateContext(m_hDC);

	if (!m_hRC)
		throw std::runtime_error("wglCreateContext(m_hDC) failed");

	// Make the rendering context active
	if ( !wglMakeCurrent(m_hDC, m_hRC) )
		throw std::runtime_error("wglMakeCurrent() failed");

	// Attempt to lock to vertical retrace
	if (vertSync == VertSync::Enable)
	{
		SetVSyncState(true);
	}

	m_pWindow->Show();

	// By default, set single viewport to fill the whole screen
	SetActiveViewport(Viewport(0.f, 0.f, static_cast<float32>(width), static_cast<float32>(height)));
}

void Win32GLGraphicsEngine::Shutdown()
{
	if (m_hRC)
	{
		wglMakeCurrent(nullptr, nullptr);	// Unset the current rendering context
		wglDeleteContext(m_hRC);	// Delete the rendering context
		m_hRC = nullptr;
	}

	if (m_hDC && !ReleaseDC(m_pWindow->GetHWND(), m_hDC))
	{
		m_hDC = nullptr;
	}

	m_pWindow->Destroy();

	g_pWin32GLGraphicsEngine = nullptr;
}

void Win32GLGraphicsEngine::Update(bool& bQuit)
{
	m_pWindow->ProcessMessages(bQuit);
	::SwapBuffers( m_hDC );
}

void Win32GLGraphicsEngine::SetTitle(const char* title)
{
	m_pWindow->SetTitle(title);
}

bool Win32GLGraphicsEngine::HasFocus() const
{
	return m_pWindow->IsActive();
}

void Win32GLGraphicsEngine::DoSetActiveViewport(const Viewport& viewport)
{
	// In GL, very simple, we just set the section of the current window we want to render to
	glViewport((GLint)viewport.X(), (GLint)viewport.Y(), (GLsizei)viewport.Width(), (GLsizei)viewport.Height());
}
