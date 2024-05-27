#include "BasicWin32Window.h"
#include "Win32MonitorInfo.h"
#include <cassert>
#include <stdlib.h>

extern void OnWindowResized(float32 newWidth, float32 newHeight);
extern void OnWindowActivate(bool enabled);

///////////////////////////////////////////////////////////////////////////////
const char* WND_CLASS_NAME = "GS_Win32Class";
const char* DEBUG_MULTIMONITOR_ENV_VAR = "DEBUG_MULTIMONITOR";

// Move to its own file
template <typename T>
struct Rect
{
	Rect() { }
	Rect(T x, T y, T width, T height) : x(x), y(y), width(width), height(height) { }

	T x, y;
	T width, height;
};

namespace
{
	// Globals
	Win32MonitorInfo g_mainMonitorInfo = Win32MonitorInfo::GetMonitorInfo(PRIMARY_MONITOR_INDEX);
	BasicWin32Window* g_pCurrBasicWin32Window = nullptr;

	// Prototypes
	LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// Utility function to adjust window position and size
	Rect<int> AdjustWindow(int width, int height, bool bCenter, DWORD dwStyle, DWORD dwExStyle)
	{
		// Determine window's new size
		RECT rect = {0, 0, width, height};
		::AdjustWindowRectEx(&rect, dwStyle, false, dwExStyle);

		// Deterine window's new position
		int x = CW_USEDEFAULT;
		int y = 0;

		if ( bCenter )
		{
			int monW = g_mainMonitorInfo.width;
			int monH = g_mainMonitorInfo.height;

			x = (monW - width)/2;
			y = (monH - height)/2;
		}

		return Rect<int>(x, y, rect.right-rect.left, rect.bottom-rect.top);
	}

}

void BasicWin32Window::Create(const char* title, int width, int height, int bpp, bool bFullScreen)
{
	g_pCurrBasicWin32Window = this;
	m_bActive = true; // Assume active on start (we miss the initial WM_ACTIVATE message)
	m_bFullScreen = bFullScreen;

	m_hInstance = ::GetModuleHandle(nullptr); // Grab an instance for the window

	WNDCLASS wc;
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw on size, and own dc for window.
	wc.lpfnWndProc		= (WNDPROC)WndProc;						// WndProc handles messages
	wc.cbClsExtra		= 0;									// No extra window data
	wc.cbWndExtra		= 0;									// No extra window data
	wc.hInstance		= m_hInstance;							// Set the instance
	wc.hIcon			= LoadIcon(nullptr, IDI_WINLOGO);			// Load the default icon
	wc.hCursor			= LoadCursor(nullptr, IDC_ARROW);			// Load the arrow pointer
	wc.hbrBackground	= nullptr;									// No background required for GL
	wc.lpszMenuName		= nullptr;									// We don't want a menu
	wc.lpszClassName	= WND_CLASS_NAME;						// Set the class name

	// Register the window class
	if ( !::RegisterClass(&wc) )
	{
		throw std::runtime_error("Failed to register the window class");
	}

	// g_mainMonitorInfo should be set to the first monitor
	assert(g_mainMonitorInfo.bAttached); // First monitor must always be attached

	// Check if multi monitor debugging is set to 1 in environment
	char* envVar = getenv(DEBUG_MULTIMONITOR_ENV_VAR);
	bool bDebugMultiMonitor = envVar && envVar[0]=='1';

	if (bDebugMultiMonitor)
	{
		// Try to get 2nd monitor, if available
		Win32MonitorInfo MonitorInfo2 = Win32MonitorInfo::GetMonitorInfo(SECONDARY_MONITOR_INDEX);
		if ( MonitorInfo2.bAttached )
			g_mainMonitorInfo = MonitorInfo2;
		else
			bDebugMultiMonitor = false;
	}

	// If full screen, we need to change the screen's display settings
	if (m_bFullScreen)
	{
		DEVMODE dmScreenSettings;								// Device mode
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));	// Makes sure memory's cleared
		dmScreenSettings.dmSize=sizeof(dmScreenSettings);		// Size of the devmode structure
		dmScreenSettings.dmPelsWidth	= width;				// Selected screen width
		dmScreenSettings.dmPelsHeight	= height;				// Selected screen height
		dmScreenSettings.dmBitsPerPel	= bpp;					// Selected bits per pixel
		dmScreenSettings.dmFields		= DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Set monitor frequency to 60 Hz
		dmScreenSettings.dmDisplayFrequency = 60;
		dmScreenSettings.dmFields |= DM_DISPLAYFREQUENCY;

		// Try to set selected mode and get results.  NOTE: CDS_FULLSCREEN gets rid of start bar.
		if (::ChangeDisplaySettingsEx(g_mainMonitorInfo.name.c_str(), &dmScreenSettings, nullptr, CDS_FULLSCREEN, nullptr) != DISP_CHANGE_SUCCESSFUL)
		{
			// Mode failed, so tell user we're switching to windowed mode

			::MessageBox(nullptr,
				"The requested fullscreen mode is not supported by\n"
				"your video card. Will use windowed mode instead",
				"Unsupported Fullscreen Mode", MB_OK|MB_ICONEXCLAMATION);

			m_bFullScreen = false;
		}
	}

	DWORD dwExStyle;	// Window extended style
	DWORD dwStyle;		// Window style

	if (m_bFullScreen)
	{
		dwExStyle = WS_EX_APPWINDOW;
		dwStyle = WS_POPUP;
		//::ShowCursor(FALSE); // Hide mouse pointer
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle = WS_OVERLAPPED | WS_CAPTION;

		// Allow maximize/minimize/close - has a side effect of overriding the ALT
		// key so that it brings up the control menu. There should be a way of stopping
		// that.
		dwStyle |= WS_SYSMENU | WS_MINIMIZEBOX| WS_MAXIMIZEBOX;

		// Allow window resizing
		dwStyle |= WS_THICKFRAME;
	}

	// Determine window position and size
	bool bCenterWindow = (m_bFullScreen==false); // Center window in windowed mode only
	Rect<int> wndRect = AdjustWindow(width, height, bCenterWindow, dwStyle, dwExStyle);

	if (bDebugMultiMonitor)
	{
		// Finally, set the start window position to top-left of second monitor
		// (now stored in g_mainMonitorInfo)

		// Since we may have switched to fullscreen mode, we must retrieve the
		// updated monitor info again
		Win32MonitorInfo MonitorInfo2 = Win32MonitorInfo::GetMonitorInfo(1);

		// Update window position
		wndRect.x = MonitorInfo2.x;
		wndRect.y = MonitorInfo2.y;
	}

	// Finally, create the window
	m_hWnd=::CreateWindowEx(dwExStyle,				// Extended style for the window
							WND_CLASS_NAME,			// Class name
							title,					// Window title
							dwStyle |				// Defined window style
							WS_CLIPSIBLINGS |		// Required window style
							WS_CLIPCHILDREN,		// Required window style
							wndRect.x,				// Window x position
							wndRect.y,				// Window y position
							wndRect.width,			// Window width
							wndRect.height,			// Window height
							nullptr,					// No parent window
							nullptr,					// No menu
							m_hInstance,			// Instance
							nullptr);					// Don't pass anything to WM_CREATE

	if (!m_hWnd)
	{
		Destroy();
		throw std::runtime_error("Failed to create window");
	}
}

void BasicWin32Window::Destroy()
{
	if ( m_bFullScreen )
		RestoreScreenMode();

	if (m_hWnd && ::DestroyWindow(m_hWnd))
	{
		m_hWnd = nullptr;
	}

	if (m_hInstance && ::UnregisterClass(WND_CLASS_NAME, m_hInstance))
	{
		m_hInstance = nullptr;
	}

	g_pCurrBasicWin32Window = nullptr;
}

void BasicWin32Window::Show() const
{
	::ShowWindow(m_hWnd, SW_SHOW);
}

void BasicWin32Window::Hide() const
{
	::ShowWindow(m_hWnd, SW_HIDE);
}

bool BasicWin32Window::ProcessMessages(bool& bQuit)
{
	bQuit = false;
	
	MSG msg;
	if (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) // Is there a message waiting?
	{
		if (msg.message == WM_QUIT)
		{
			bQuit = true;
		}
		else
		{
			//::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}

		return true; // Message processed
	}

	return false; // No messages processed
}

void BasicWin32Window::RestoreScreenMode()
{
	// Switch back to the desktop
	::ChangeDisplaySettingsEx(g_mainMonitorInfo.name.c_str(), nullptr, 0, 0, nullptr);
	::ShowCursor(TRUE);	// Show mouse pointer
}

void BasicWin32Window::SetTitle(const char* title)
{
	::SetWindowText(m_hWnd, title);
}

namespace
{
	LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_ACTIVATE:
			{
				if (g_pCurrBasicWin32Window)
				{
					g_pCurrBasicWin32Window->__SetActive( LOWORD(wParam)!=0 );
				}
				return 0;
			}

		case WM_SYSCOMMAND:
			{
				switch (wParam)
				{
				case SC_SCREENSAVE:
				case SC_MONITORPOWER:
					return 0;
				}
				break;
			}

		case WM_CLOSE:
			{
				PostQuitMessage(0);
				return 0;
			}

			/*
		case WM_KEYDOWN:
			{
				keys[wParam] = TRUE;
				return 0;
			}

		case WM_KEYUP:
			{
				keys[wParam] = FALSE;
				return 0;
			}
			*/

		case WM_SIZE:
			{
				OnWindowResized(LOWORD(lParam), HIWORD(lParam));
				return 0;
			}

		case WM_ERASEBKGND:
			return 0;
		}

		// Pass All Unhandled Messages To DefWindowProc
		return DefWindowProc(hWnd,uMsg,wParam,lParam);
	}
}
