#include "GraphicsEngine.h"
#include <memory>

#define USE_GL_RENDERER

#if defined(WIN32) && defined(USE_GL_RENDERER)

#include "gs/Platform/Win32/Win32GLGraphicsEngine.h"

GraphicsEngine& GraphicsEngine::Instance()
{
	static Win32GLGraphicsEngine* pInstance = nullptr;
	if (!pInstance)
	{
		pInstance= new Win32GLGraphicsEngine();
	}
	return *pInstance;
}

#else

#error GraphicsEngine class not defined for current platform

#endif
