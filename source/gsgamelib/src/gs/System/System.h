#ifndef _SYSTEM_H_
#define _SYSTEM_H_

// Include the platform-specific System class and typedef it to System
#ifdef WIN32

#include "gs/Platform/Win32/Win32System.h"
typedef Win32System System;

#else

#error No System class defined for current platform

#endif

#endif // _SYSTEM_H_
