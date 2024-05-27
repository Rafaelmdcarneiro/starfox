#ifndef _GL_HEADERS_H_
#define _GL_HEADERS_H_

// OpenGL header includes

#ifdef WIN32
// Must include windows.h before GL headers on Win32
#include "gs/Platform/Win32/Win32Headers.h"
#endif

// Include OpenGL-specific headers
#include <gl/gl.h>
#include <gl/glu.h>

#ifdef WIN32
// Link with libs
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#endif

#endif // _GL_HEADERS_H_
