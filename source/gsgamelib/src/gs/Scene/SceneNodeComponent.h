#ifndef __SCENE_NODE_COMPONENT_H__
#define __SCENE_NODE_COMPONENT_H__

#include "gs/Base/Base.h"
#include <cassert>

class SceneNode;

// Base class for components
class SceneNodeComponent
{
public:
	SceneNodeComponent() : m_pSceneNode(nullptr), m_enabled(true) {}
	virtual ~SceneNodeComponent() {}

	SceneNode* GetSceneNode() { assert(m_pSceneNode); return m_pSceneNode; }

	template <typename ComponentT>
	ComponentT* TryGetSiblingComponent();

	template <typename ComponentT>
	ComponentT* GetSiblingComponent();

	void SetEnabled(bool enabled) { m_enabled = enabled; }
	bool IsEnabled() const { return m_enabled; }

	virtual void Update(float32 deltaTime) {}
	virtual void Render() {}

protected:
	virtual void OnPostAddComponent(SceneNode& owner) {}
	virtual void OnPreRemoveComponent(SceneNode& owner) {}

private:
	friend class SceneNode;
	SceneNode* m_pSceneNode;
	bool m_enabled;
};

#endif // __SCENE_NODE_COMPONENT_H__
