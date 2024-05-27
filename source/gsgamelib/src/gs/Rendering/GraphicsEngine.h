#ifndef _GRAPHICS_ENGINE_H_
#define _GRAPHICS_ENGINE_H_

#include "gs/Base/Base.h"
#include <functional>

namespace ScreenMode
{ 
	enum Type { FullScreen, Windowed };
}

namespace VertSync
{
	enum Type { Enable, Disable };
}

class Viewport
{
public:
	Viewport(float32 x, float32 y, float32 width, float32 height) : m_x(x), m_y(y), m_width(width), m_height(height) {}

	float32 X() const		{ return m_x; }
	float32 Y() const		{ return m_y; }
	float32 Width() const	{ return m_width; }
	float32 Height() const	{ return m_height; }

private:
	float32 m_x, m_y, m_width, m_height;
};


// Base interface to graphics engine.
class GraphicsEngine
{
protected:
	GraphicsEngine() {}
public:
	virtual ~GraphicsEngine() {}

	// Returns singleton instance
	static GraphicsEngine& Instance();

	// Initialize graphics engine, sets resolution, and sets up single viewport to fill screen/window
	virtual void Initialize(const char* title, int width, int height, int bpp, ScreenMode::Type screenMode, VertSync::Type vertSync) = 0;
	
	// Shuts down graphics engine
	virtual void Shutdown() = 0 ;

	// Call once per frame, sets bQuit to true if user tries to quit, false otherwise
	virtual void Update(bool& bQuit) = 0;

	// (Optional) If set, when window is resized, callback will be invoked with new window dimensions.
	// Set before invoking Initialize() to get the initial callback when the window is created.
	typedef std::function<void (GraphicsEngine& gfxEngine, float32 newWidth, float32 newHeight)> WindowResizedCallback;
	void SetWindowResizedCallback(const WindowResizedCallback& callback)
	{
		m_windowResizedCallback = callback;
	}

	// Sets the active viewport (portion of screen to start rendering to)
	void SetActiveViewport(const Viewport& viewport)
	{
		DoSetActiveViewport(viewport);
	}

	// Sets window title (if applicable)
	virtual void SetTitle(const char* title) = 0;

	// Returns true if the window has input focus
	virtual bool HasFocus() const = 0;

protected:
	virtual void DoSetActiveViewport(const Viewport& viewport) = 0;
	
	WindowResizedCallback m_windowResizedCallback;
};

#endif // _GRAPHICS_ENGINE_H_
