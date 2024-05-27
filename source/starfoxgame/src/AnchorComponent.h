#ifndef _ANCHOR_COMPONENT_H_
#define _ANCHOR_COMPONENT_H_

#include "gs/Scene/SceneNode.h"

// Ship and Ground attaches to Anchor
class AnchorComponent : public SceneNodeComponent
{
public:
	AnchorComponent();
	void Advance(float32 distance);

private:
	float32 m_totalDistance;
};

#endif // _ANCHOR_COMPONENT_H_

