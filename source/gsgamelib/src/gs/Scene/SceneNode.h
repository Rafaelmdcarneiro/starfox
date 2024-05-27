#ifndef __SCENE_NODE_H__
#define __SCENE_NODE_H__

#include <string>
#include <memory>
#include <vector>
#include <algorithm>
#include <cassert>
#include "gs/Math/Matrix43.h"
#include "SceneNodeComponent.h"

// ps : shared pointer
// pw : weak pointer

enum class TreeTraversalType
{
	PreOrder, // Parents before children
	PostOrder, // Children before parents
};

class SceneNode;

typedef std::shared_ptr<SceneNode> SceneNodeSharedPtr;
typedef std::weak_ptr<SceneNode> SceneNodeWeakPtr;
typedef std::shared_ptr<const SceneNode> ConstSceneNodeSharedPtr;
typedef std::weak_ptr<const SceneNode> ConstSceneNodeWeakPtr;

class SceneNode : public std::enable_shared_from_this<SceneNode>
{
#pragma region SceneGraph
private:
	friend class SceneGraph;
	struct private_constructor_tag {}; // Workaround since we can't friend std::make_shared easily/portably

public:
	SceneNode(const private_constructor_tag&);
	virtual ~SceneNode();

	SceneNodeSharedPtr AsSharedPtr()
	{
		return shared_from_this();
	}

	ConstSceneNodeSharedPtr AsSharedPtr() const
	{
		return shared_from_this();
	}

	//////////////////////////////////////////////////////////////////////
	// SceneGraph functions
	//////////////////////////////////////////////////////////////////////

	// Creates a root node (no parent) and adds it to the scene graph
	static SceneNodeSharedPtr Create(const std::string& name);

	// Destroys node and all its children
	static void Destroy(SceneNodeSharedPtr psNode);

	static void DestroyAllNodes();

	// Debug: Call once per frame after all nodes have been updated
	static void ValidateSceneGraph();

	// Returns unordered list of weak pointers to all the nodes
	//@NOTE: Order of nodes is arbitrary right now because we simply copy from the master list,
	// which is unsorted. We could keep the master list sorted so that parents are always before
	// children (but not strictly), which would probably be the most useful ordering for the snapshot.
	// Otherwise, we could build the list by traversing the tree, but that would be more expensive.
	static std::vector<SceneNodeWeakPtr> GetAllNodesSnapshot();

	void AttachChild(const SceneNodeSharedPtr& psNode);

	void DetachFromParent();

	SceneNodeSharedPtr GetParent()
	{
		return m_pwParent.lock();
	}

	ConstSceneNodeSharedPtr GetParent() const
	{
		return m_pwParent.lock();
	}

	size_t GetNumChildren() const
	{
		return m_children.size();
	}

	SceneNodeSharedPtr GetChild(size_t index)
	{
		assert( !m_children.at(index).expired() ); // Two checks: index in range and valid shared_ptr
		return m_children[index].lock();
	}

	ConstSceneNodeSharedPtr GetChild(size_t index) const
	{
		assert( !m_children.at(index).expired() ); // Two checks: index in range and valid shared_ptr
		return m_children[index].lock();
	}

	const std::vector<SceneNodeWeakPtr>& GetChildren() const
	{
		return m_children;
	}

	// Traverses subtree starting from this node, invoking input Func on each node.
	// Func format: bool (const SceneNodeSharedPtr&) -> return false to stop traversal
	template <typename Func>
	bool TraverseSubtree(const Func& func, TreeTraversalType traversalType = TreeTraversalType::PreOrder)
	{
		if (traversalType == TreeTraversalType::PreOrder)
		{
			if ( !func(AsSharedPtr()) )
				return false;
		}

		for (const auto& pwChildNode : m_children)
		{
			if ( !pwChildNode.lock()->TraverseSubtree(func) )
				return false;
		}

		if (traversalType == TreeTraversalType::PostOrder)
		{
			if ( !func(AsSharedPtr()) )
				return false;
		}

		return true;
	}

	// Adds this node and descendents to input container.
	// Typically the container would be a std::vector<SceneNodeSharedPtr> or std::vector<SceneNodeWeakPtr>
	template <typename ContainerType>
	void FlattenSubtree(ContainerType& container)
	{
		auto AddNodeToContainer = [&](const SceneNodeSharedPtr& psNode)
		{
			container.push_back(psNode);
			return true;
		};
		TraverseSubtree(AddNodeToContainer, TreeTraversalType::PreOrder);
	}

private:
	void DoAttachChild(const SceneNodeSharedPtr& psNode);
	void DoDetachChild(const SceneNodeSharedPtr& psNode);	

	std::string m_name;
	SceneNodeWeakPtr m_pwParent;
	std::vector<SceneNodeWeakPtr> m_children;
#pragma endregion SceneGraph

#pragma region Transform
public:
	Matrix43& ModifyLocalToParent();
	Matrix43& ModifyLocalToWorld();

	const Matrix43& GetLocalToParent() const;
	const Matrix43& GetLocalToWorld() const;

private:
	void PostAttachChild_UpdateTransform(const SceneNodeSharedPtr& psNode);
	void PreDetachChild_UpdateTransform(const SceneNodeSharedPtr& psNode);
	void RecurseSetComputeL2W();

	// NEVER set m_bComputeL2P/L2W directly, ALWAYS call these functions.
	// These make sure that both matrices are never dirty at the same time.
	void SetComputeL2P();
	void SetComputeL2W();

	// L2W and L2P matrices are cached version that are
	// recomputed when necessary
	mutable bool m_bComputeL2P;
	mutable bool m_bComputeL2W;
	mutable Matrix43 m_localToParent;
	mutable Matrix43 m_localToWorld;
#pragma endregion Transform

#pragma region Component
public:
	template <typename ComponentT>
	ComponentT* AddComponent()
	{
		auto pNewComponent = new ComponentT();
		pNewComponent->m_pSceneNode = this;
		m_components.push_back(pNewComponent);
		pNewComponent->OnPostAddComponent(*this);
		return pNewComponent;
	}

	void RemoveComponent(SceneNodeComponent* pComponent)
	{
		auto iter = std::find(begin(m_components), end(m_components), pComponent);
		assert(iter != m_components.end());
		(*iter)->OnPreRemoveComponent(*this);
		delete *iter;
		m_components.erase(iter);
	}

	// Returns first instance of component of type ComponentT in SceneNode or nullptr if not found
	template <typename ComponentT>
	ComponentT* TryGetComponent()
	{
		for (auto pComponent : m_components)
		{
			auto pInputComponent = dynamic_cast<ComponentT*>(pComponent);
			if (pInputComponent != nullptr)
				return pInputComponent;
		}
		return nullptr;
	}

	template <typename ComponentT>
	ComponentT* GetComponent()
	{
		auto pComponent = TryGetComponent<ComponentT>();
		assert(pComponent);
		return pComponent;
	}

	// Returns all instances of components of type ComponentT in SceneNode or empty array if none found
	template <typename ComponentT>
	std::vector<ComponentT*> TryGetComponents()
	{
		std::vector<ComponentT*> result;
		TryGetComponentsInto(result);
		return result;
	}

	template <typename ComponentT>
	std::vector<ComponentT*> GetComponents()
	{
		auto result = TryGetComponents<ComponentT>();
		assert(result.size() > 0);
		return result;
	}

	// Returns first instance of component of type ComponentT in SceneNode or any of its children, or nullptr if none found
	template <typename ComponentT>
	ComponentT* TryGetComponentInChildren()
	{
		ComponentT* pResult;

		auto f = [&](const SceneNodeSharedPtr& psNode)
		{
			pResult = psNode->TryGetComponent<ComponentT>();
			return pResult == nullptr; // Keep traversing until we find a valid one
		};

		TraverseSubtree(f);
		return pResult;
	}

	template <typename ComponentT>
	ComponentT* GetComponentInChildren()
	{
		auto pComponent = TryGetComponentInChildren<ComponentT>();
		assert(pComponent);
		return pComponent;
	}

	// Returns all instances of components of type ComponentT in SceneNode or any of its children, or empty array if none found
	template <typename ComponentT>
	std::vector<ComponentT*> TryGetComponentsInChildren()
	{
		std::vector<ComponentT*> result;

		auto f = [&](const SceneNodeSharedPtr& psNode)
		{
			psNode->TryGetComponentsInto(result);
			return true;
		};

		TraverseSubtree(f);
		return result;
	}

	template <typename ComponentT>
	std::vector<ComponentT*> GetComponentsInChildren()
	{
		auto result = TryGetComponentsInChildren<ComponentT>();
		assert(result.size() > 0);
		return result;
	}


	void Update(float32 deltaTime)
	{
		for (auto pComponent : m_components)
		{
			if (pComponent->IsEnabled())
				pComponent->Update(deltaTime);
		}
	}

	void Render()
	{
		for (auto pComponent : m_components)
		{
			if (pComponent->IsEnabled())
				pComponent->Render();
		}
	}

private:
	template <typename ComponentT>
	void TryGetComponentsInto(std::vector<ComponentT*>& result)
	{
		for (auto pComponent : m_components)
		{
			auto pInputComponent = dynamic_cast<ComponentT*>(pComponent);
			if (pInputComponent != nullptr)
				result.push_back(pInputComponent);
		}
	}

	std::vector<SceneNodeComponent*> m_components;
#pragma endregion Component
};

template <typename ComponentT>
ComponentT* SceneNodeComponent::TryGetSiblingComponent()
{
	return m_pSceneNode->TryGetComponent<ComponentT>();
}

template <typename ComponentT>
ComponentT* SceneNodeComponent::GetSiblingComponent()
{
	return m_pSceneNode->GetComponent<ComponentT>();
}

#endif // __SCENE_NODE_H__
