#ifndef _BASIC_WIN32_WINDOW_H_
#define _BASIC_WIN32_WINDOW_H_

#include "gs/Base/Base.h"
#include "Win32Window.h"
#include <string>

// A basic window perfect for the final game. It manages full creation,
// message handling, and destruction of the window.

class BasicWin32Window : public Win32Window
{
public:
	virtual void Create(const char* title, int width, int height, int bpp, bool bFullScreen);
	virtual void Destroy();

	// Shows the window
	virtual void Show() const;
	
	// Hides the window
	virtual void Hide() const;

	// Call in main game loop to let Window process messages.
	// Returns true if message is processed, and sets bQuit to
	// true if quit message is received.
	virtual bool ProcessMessages(bool& bQuit);

	// Returns true if this is the currently active window
	virtual bool IsActive() const { return m_bActive; }

	// Returns true if fullscreen mode
	virtual bool IsFullScreen() const { return m_bFullScreen; }

	// Restores screen mode back to what it was before switching to fullscreen
	virtual void RestoreScreenMode();

	virtual void SetTitle(const char* title);

	// Accessors
	virtual HWND GetHWND() { return m_hWnd; }
	virtual HINSTANCE GetHINSTANCE() { return m_hInstance; }	
	virtual HDC GetHDC() { return ::GetDC(m_hWnd); }

	// Internal
	void __SetActive(bool bActive) { m_bActive = bActive; }

private:
	HWND m_hWnd;
	HINSTANCE m_hInstance;

	bool m_bFullScreen;
	bool m_bActive;
};

#endif // _BASIC_WIN32_WINDOW_H_
