#ifndef __FBX_LOADER_H__
#define __FBX_LOADER_H__

#include "gs/Base/Base.h"
#include <memory>

namespace gfx
{
	struct StaticMesh;
}

class FbxLoader
{
public:
	FbxLoader();
	~FbxLoader() { Shutdown(); }

	void Init();
	void Shutdown();

	std::shared_ptr<gfx::StaticMesh> LoadStaticMesh(const char* pFileName);

private:
	struct PIMPL;
	std::shared_ptr<PIMPL> m_pPimpl; // Want to use unique_ptr but MSVC's implementation doesn't work with incomplete types as it should
};


#endif // __FBX_LOADER_H__
