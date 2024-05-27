#include "SceneNode.h"
#include "SceneGraph.h"

static SceneGraph g_sceneGraph;

SceneNode::SceneNode(const private_constructor_tag&)
	: m_bComputeL2P(false)
	, m_bComputeL2W(false)
	, m_localToParent(Matrix43::Identity())
	, m_localToWorld(Matrix43::Identity())
{
}

SceneNode::~SceneNode()
{
	for (auto& pComponent : m_components)
		delete pComponent;
}

SceneNodeSharedPtr SceneNode::Create(const std::string& name)
{
	const auto& psNode = g_sceneGraph.CreateSceneNode();
	psNode->m_name = name;
	return psNode;
}

void SceneNode::Destroy(SceneNodeSharedPtr psNode)
{
	g_sceneGraph.DestroySceneNode(std::move(psNode));
}

void SceneNode::DestroyAllNodes()
{
	g_sceneGraph.DestroyAllSceneNodes();
}

void SceneNode::ValidateSceneGraph()
{
	g_sceneGraph.Validate();
}

std::vector<SceneNodeWeakPtr> SceneNode::GetAllNodesSnapshot()
{
	return g_sceneGraph.GetAllSceneNodesSnapshot();
}

void SceneNode::AttachChild(const SceneNodeSharedPtr& psNode)
{
	g_sceneGraph.AttachSceneNode(psNode, AsSharedPtr());
}

void SceneNode::DetachFromParent()
{
	g_sceneGraph.DetachSceneNodeFromParent(AsSharedPtr());
}

void SceneNode::DoAttachChild(const SceneNodeSharedPtr& psNode)
{
	assert(psNode->m_pwParent.expired());
	m_children.push_back(psNode);
	psNode->m_pwParent = AsSharedPtr();
	
	PostAttachChild_UpdateTransform(psNode);
}

void SceneNode::DoDetachChild(const SceneNodeSharedPtr& psNode)
{
	PreDetachChild_UpdateTransform(psNode);

	auto CompareWeakToSharedPtr  = [&](const SceneNodeWeakPtr& pwCurrNode) { return is_weak_to_shared_ptr(pwCurrNode, psNode); };
	auto iter = std::find_if(begin(m_children), end(m_children), CompareWeakToSharedPtr);
	assert(iter != end(m_children));
	m_children.erase(iter);
	psNode->m_pwParent.reset();
}


Matrix43& SceneNode::ModifyLocalToParent()
{
	// m_localToParent will be modified, so our L2W matrix and
	// those of our children must be invalidated
	RecurseSetComputeL2W();

	return m_localToParent;
}

Matrix43& SceneNode::ModifyLocalToWorld()
{
	// When we change a child's L2W matrix, we don't move the parent, we just update
	// the child's L2P to reflect it's new position. So we set the l2p dirty flag on
	// and recompute it on demand.
	SetComputeL2P();

	// However, our children's L2W matrices are now invalid, so recurse and mark them
	// as such
	for (std::size_t i=0; i<GetNumChildren(); ++i)
	{
		GetChild(i)->RecurseSetComputeL2W();
	}

	return m_localToWorld;
}

const Matrix43& SceneNode::GetLocalToParent() const
{
	if (m_bComputeL2P)
	{
		// This means the L2W matrix was modified.
		// When we change a child's L2W matrix, we don't move the parent, we just update
		// the child's L2P to reflect it's new position.
		// So Child's L2P = Child's desired L2W - Parent's L2W

		if (const ConstSceneNodeSharedPtr& pParent = GetParent())
		{
			Matrix43 parL2Winv = pParent->GetLocalToWorld();
			parL2Winv.InvertSRT();

			Matrix43& myL2P = const_cast<SceneNode*>(this)->ModifyLocalToParent();
			// Cl = Cw * P-1w
			myL2P = m_localToWorld * parL2Winv;
		}
		else // We have no parent, just set our L2P to match our L2W
		{
			Matrix43& myL2P = const_cast<SceneNode*>(this)->ModifyLocalToParent();
			myL2P = m_localToWorld;
		}

		m_bComputeL2P = false;
	}

	return m_localToParent;
}

const Matrix43& SceneNode::GetLocalToWorld() const
{
	if (m_bComputeL2W)
	{
		if (const ConstSceneNodeSharedPtr& pParent = GetParent())
		{
			// This recursively has our parents compute their L2W if they need to
			const Matrix43& parL2W = pParent->GetLocalToWorld();

			// L2W = L2P * par.L2W
			m_localToWorld = m_localToParent * parL2W;
		}
		else
		{
			m_localToWorld = GetLocalToParent();
		}

		m_bComputeL2W = false;
	}

	return m_localToWorld;
}

void SceneNode::PostAttachChild_UpdateTransform(const SceneNodeSharedPtr& psNode)
{
	// Node has a new parent, so set its world matrix to match its
	// previous local matrix. This updates the local matrix to reflect
	// the difference between the node and its new parent, effecively
	// not moving it from where it was.
	Matrix43& nodeL2W = psNode->ModifyLocalToWorld();
	nodeL2W = psNode->GetLocalToParent();
}

void SceneNode::PreDetachChild_UpdateTransform(const SceneNodeSharedPtr& psNode)
{
	// Node is about to become part of the world, so set its local
	// matrix to match its current world matrix so that it doesn't
	// move from where it is.

	Matrix43 currL2W = psNode->GetLocalToWorld();
	psNode->ModifyLocalToParent() = currL2W;

	//Matrix43& nodeL2P = psNode->ModifyLocalToParent();
	//nodeL2P = psNode->GetLocalToWorld();
}

void SceneNode::RecurseSetComputeL2W()
{
	// If node already has flag set, then all its children must already
	// have been set (from last call to this function)
	if (!m_bComputeL2W)
	{
		SetComputeL2W();

		for (std::size_t i=0; i<GetNumChildren(); ++i)
		{
			GetChild(i)->RecurseSetComputeL2W();
		}
	}
}

void SceneNode::SetComputeL2P()
{
	// Must set this flag before calling GetLocalToWorld() to avoid recursion
	m_bComputeL2P = true;

	// Cannot have both local _and_ world dirty on a node
	if (m_bComputeL2W)
		GetLocalToWorld();
}

void SceneNode::SetComputeL2W()
{
	m_bComputeL2W = true; // Again, to avoid recursion, set this first

	if (m_bComputeL2P)
		GetLocalToParent();
}

