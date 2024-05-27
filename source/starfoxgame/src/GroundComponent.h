#ifndef _GROUND_COMPONENT_H_
#define _GROUND_COMPONENT_H_

#include "gs/Scene/SceneNode.h"

// Attached to Anchor
class GroundComponent : public SceneNodeComponent
{
public:
	virtual void Render();	
};

#endif // _GROUND_COMPONENT_H_
