#ifndef __SCENE_GRAPH_H__
#define __SCENE_GRAPH_H__

#include "SceneNode.h"
#include <memory>
#include <vector>
#include <list>

#ifdef _DEBUG
#define SCENEGRAPH_VALIDATION 1
#endif

// Private implementation of the SceneGraph. The functionality is exposed via SceneNode functions.
class SceneGraph
{
public:
	SceneGraph()
	{
	}

	SceneNodeSharedPtr CreateSceneNode()
	{
		const SceneNodeSharedPtr& psNode = std::make_shared<SceneNode>(SceneNode::private_constructor_tag());
		m_allNodes.push_back(psNode);
		m_rootNodes.push_back(psNode);
		return psNode;
	}

	void DestroySceneNode(SceneNodeSharedPtr psNode)
	{
		// Remove this node and its children from master list
		EraseNodeHierarchy(psNode);

		// Detach this node from its parent
		if (const auto& psParentNode = psNode->GetParent())
		{
			psParentNode->DoDetachChild(psNode);
		}
		else // No parent, remove from root node list
		{
			auto iter = std::find(begin(m_rootNodes), end(m_rootNodes), psNode);
			assert(iter != end(m_rootNodes));
			m_rootNodes.erase(iter);
		}
	}

	void DestroyAllSceneNodes()
	{
		m_allNodes.clear();
		m_rootNodes.clear();
	}

	void AttachSceneNode(const SceneNodeSharedPtr& psChildNode, const SceneNodeSharedPtr& psParentNode)
	{
		// Detach from current parent
		if (const auto& psOldParentNode = psChildNode->GetParent())
		{
			psOldParentNode->DoDetachChild(psChildNode);
		}
		else // No parent, remove from root node list
		{
			auto iter = std::find(begin(m_rootNodes), end(m_rootNodes), psChildNode);
			assert(iter != end(m_rootNodes));
			m_rootNodes.erase(iter);
		}

		// Attach to new parent
		psParentNode->DoAttachChild(psChildNode);
	}

	void DetachSceneNodeFromParent(const SceneNodeSharedPtr& psNode)
	{
		const auto& psParentNode = psNode->GetParent();
		assert(psParentNode && "Node has no parent!");
		psParentNode->DoDetachChild(psNode);

		// Add to root node list
		m_rootNodes.push_back(psNode);
	}

	std::vector<SceneNodeWeakPtr> GetAllSceneNodesSnapshot()
	{
		std::vector<SceneNodeWeakPtr> snapshot(m_allNodes.size());
		std::copy(begin(m_allNodes), end(m_allNodes), begin(snapshot));
		return snapshot;
	}

	void Validate()
	{
#if SCENEGRAPH_VALIDATION
		for (const auto& psNode : m_allNodes)
		{
			// Make sure the only strong references to a node are from m_allNodes and possibly m_rootNodes
			const size_t numStrongRefs = psNode.use_count();
			assert(numStrongRefs == 1 || numStrongRefs == 2 && "Do not store/cache strong references to SceneNodes");
			
			// Root nodes are in both arrays, non-root are only in m_allNodes
			const bool isRootNode = numStrongRefs == 2 && psNode->GetParent() == nullptr;
			const bool isNonRootNode = numStrongRefs == 1 && psNode->GetParent() != nullptr;
			assert(isRootNode || isNonRootNode && "SceneGraph consistency error");
		}

		// Make sure that all nodes that were destroyed have no strong references anymore
		for (const auto& pwNode : m_destroyedNodes)
		{
			assert(pwNode.expired() && "Destroyed SceneNode must not have any strong references to it");
		}
		m_destroyedNodes.clear();
#endif
	}
	
private:
	void EraseNodeHierarchy(const SceneNodeSharedPtr& psNode)
	{
		assert(psNode);

		//@TODO: If m_allNodes is sorted parent before children, we can pass ++iter as the iterator to
		// keep searching from when we recurse
		auto iter = std::find(begin(m_allNodes), end(m_allNodes), psNode);
		assert(iter != end(m_allNodes));
#if SCENEGRAPH_VALIDATION
		m_destroyedNodes.push_back(*iter);
#endif
		m_allNodes.erase(iter);

		for (const auto& pwNode : psNode->m_children)
		{
			EraseNodeHierarchy(pwNode.lock());
		}
	}

	std::list<SceneNodeSharedPtr> m_allNodes;
	std::list<SceneNodeSharedPtr> m_rootNodes;
#if SCENEGRAPH_VALIDATION
	std::vector<SceneNodeWeakPtr> m_destroyedNodes;
#endif
};

#endif // __SCENE_GRAPH_H__
