#ifndef _STATIC_MESH_COMPONENT_H_
#define _STATIC_MESH_COMPONENT_H_

#include "gs/Scene/SceneNode.h"

namespace gfx { struct StaticMesh; }

class StaticMeshComponent : public SceneNodeComponent
{
public:
	void Init(const std::shared_ptr<gfx::StaticMesh>& psStaticMesh)
	{
		m_pStaticMesh = std::move(psStaticMesh);
	}

	virtual void Render();	

	gfx::StaticMesh& GetMesh()
	{
		return *m_pStaticMesh;
	}

private:
	std::shared_ptr<gfx::StaticMesh> m_pStaticMesh;
};


#endif // _STATIC_MESH_COMPONENT_H_
