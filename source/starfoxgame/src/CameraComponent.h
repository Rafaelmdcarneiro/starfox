#ifndef _CAMERA_COMPONENT_H_
#define _CAMERA_COMPONENT_H_

#include "gs/Scene/SceneNode.h"
#include "gs/Math/EulerAngles.h"

class OrbitTargetCameraComponent : public SceneNodeComponent
{
public:
	OrbitTargetCameraComponent()
		: m_angles(EulerAngles::Identity())
		, m_offset(250.f)
	{
	}

	void SetTarget(const SceneNodeWeakPtr& target)
	{
		m_pwTarget = target;
	}

	virtual void Update(float32 deltaTime);

private:
	SceneNodeWeakPtr m_pwTarget;
	float32 m_offset;
	EulerAngles m_angles;
};

class FollowShipCameraComponent : public SceneNodeComponent
{
public:
	FollowShipCameraComponent()
		: m_offset(350.f)
	{
	}

	void SetTarget(const SceneNodeWeakPtr& pwTarget)
	{
		m_pwTarget = pwTarget;
		UpdateTransform(0.f, false);
	}

	virtual void Update(float32 deltaTime)
	{
		UpdateTransform(deltaTime, true);
	}

private:
	void UpdateTransform(float32 deltaTime, bool damp);	

	SceneNodeWeakPtr m_pwTarget;
	float32 m_offset;
};


#endif // _CAMERA_COMPONENT_H_
