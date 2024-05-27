#ifndef _WIN32_WINDOW_H_
#define _WIN32_WINDOW_H_

#include "Win32Headers.h"

#ifndef WIN32
#error "Only for Win32 platform"
#endif

// Abstract base Win32 window class

class Win32Window
{
public:
	// Create the window
	virtual void Create(const char* title, int width, int height, int bpp, bool bFullScreen) = 0;
	
	// Destroy the window
	virtual void Destroy() = 0;

	// Shows the window
	virtual void Show() const = 0;
	
	// Hides the window
	virtual void Hide() const = 0;

	// Call in main game loop to let Window process messages.
	// Returns true if message is processed, and sets bQuit to
	// true if quit message is received.
	virtual bool ProcessMessages(bool& bQuit) = 0;

	// Returns true if this is the currently active window
	virtual bool IsActive() const = 0;

	// Returns true if fullscreen mode
	virtual bool IsFullScreen() const = 0;

	// Restores screen mode back to what it was before switching to fullscreen
	virtual void RestoreScreenMode() = 0;

	virtual void SetTitle(const char* title) = 0;

	// Accessors
	virtual HWND GetHWND() = 0;
	virtual HINSTANCE GetHINSTANCE() = 0;
	virtual HDC GetHDC() = 0;
};

#endif // _WIN32_WINDOW_H_
