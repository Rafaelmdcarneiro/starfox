#ifndef _MFC_WINDOW_H_
#define _MFC_WINDOW_H_

#ifdef GS_MFC_ENABLED

#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

// Little trick: we include Win32Window.h after afxwin.h so that
// windows.h is not included _before_ afxwin.h. Not perfect but it works.
#include <afxwin.h>
#include "Win32Window.h"

// Use this window to simply wrap an existing window in MFC. You must
// call SetHandles() before the GraphicsEngine is initialized. Apart
// from that, this class doesn't do much with the window since MFC
// is expected to take care of creation, message handling, and destruction.

class MFCWindow : public Win32Window
{
public:
	MFCWindow() : m_hWnd(nullptr), m_hInstance(nullptr) { }

	void SetHandles(HWND hWnd, HINSTANCE hInstance)
	{
		m_hWnd = hWnd;
		m_hInstance = hInstance;
	}

	virtual void Create(const char* title, int width, int height, int bpp, bool bFullScreen);
	virtual void Destroy() { }

	virtual void Show() const { }
	virtual void Hide() const { }
	virtual bool ProcessMessages(bool& /*bQuit*/) { return true; }
	virtual bool IsActive() const { return true; }
	virtual bool IsFullScreen() const { return false; }
	virtual void RestoreScreenMode() { }
	virtual void SetTitle(const char* /*title*/) { }
	virtual HWND GetHWND() { return m_hWnd; }
	virtual HINSTANCE GetHINSTANCE() { return m_hInstance; }	
	virtual HDC GetHDC() { return ::GetDC(m_hWnd); }

private:
	HWND m_hWnd;
	HINSTANCE m_hInstance;
};

#endif // GS_MFC_ENABLED
#endif // _MFC_WINDOW_H_
