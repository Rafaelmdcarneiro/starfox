#ifndef _PLAYER_CONTROL_COMPONENT_H_
#define _PLAYER_CONTROL_COMPONENT_H_

#include "gs/Scene/SceneNode.h"
#include "gs/Math/EulerAngles.h"

class PlayerControlComponent : public SceneNodeComponent
{
public:
	PlayerControlComponent();

	virtual void Update(float32 deltaTime);	

private:
	void UpdateOrientation(float32 deltaTime);
	void UpdateMovement(float32 deltaTime);

	float32 m_speed;

	EulerAngles m_currAngles;
	EulerAngles m_targetAngles;
};


#endif // _PLAYER_CONTROL_COMPONENT_H_
