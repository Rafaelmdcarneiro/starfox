#ifndef _WIN32SYSTEM_H_
#define _WIN32SYSTEM_H_

#include "gs/Base/Base.h"
#include "Win32Headers.h"
#include <winuser.h> // For VKEYs

// Each platform must define virtual key codes for each keyboard key.
// (keys with ascii values use the ascii value as their VKEY value)
typedef unsigned int VKEY;

class Win32System
{
public:
	static void Sleep(int ms)
	{
		::Sleep(ms);
	}

	static float64 GetElapsedTicks()
	{
		static LARGE_INTEGER li64Count;
		::QueryPerformanceCounter(&li64Count);		
		return static_cast<float64>(li64Count.QuadPart);
	}
	
	static float64 GetTicksPerSecond()
	{
		auto DoQueryPerformanceFrequency = []
		{
			LARGE_INTEGER li64Frequency;
			::QueryPerformanceFrequency(&li64Frequency);
			return static_cast<float64>(li64Frequency.QuadPart);
		};

		static float64 timerFrequency = DoQueryPerformanceFrequency();
		return timerFrequency;
	}

	static float64 GetElapsedSeconds()
	{
		return GetElapsedTicks() / GetTicksPerSecond();
	}

	static bool IsVKeyDown(VKEY vkey)
	{
		// If most-significant bit set, the key is down
		return (::GetAsyncKeyState(vkey) & 0x8000) != 0;
	}

	static void SetMouseVisible(bool bShow)
	{
		// Need this condition because ShowCursor() incs/decs a counter
		// to determine visibility, so multiple 'shows' would need to
		// be matched by multiple 'hides'
		if (bShow == IsMouseVisible())
			return;

		::ShowCursor(bShow? TRUE : FALSE);
	}

	static bool IsMouseVisible()
	{
		CURSORINFO cursInfo;
		cursInfo.cbSize = sizeof(cursInfo);
		::GetCursorInfo(&cursInfo);
		return (cursInfo.flags & CURSOR_SHOWING);
	}

	static bool IsDebuggerAttached()
	{
		return ::IsDebuggerPresent() == TRUE;
	}
};

#endif // _WIN32SYSTEM_H_
