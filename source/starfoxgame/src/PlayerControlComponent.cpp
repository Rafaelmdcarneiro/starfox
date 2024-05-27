#include "PlayerControlComponent.h"

#include "GameInput.h"
#include "AnchorComponent.h"
#include "gs/Math/MathUtil.h"

PlayerControlComponent::PlayerControlComponent()
	: m_speed(1000.f)
	, m_currAngles(EulerAngles::Identity())
	, m_targetAngles(EulerAngles::Identity())
{
}

void PlayerControlComponent::Update(float32 deltaTime)
{
	UpdateOrientation(deltaTime);
	UpdateMovement(deltaTime);
}

void PlayerControlComponent::UpdateOrientation(float32 deltaTime)
{
	KeyboardMgr& kbMgr = KeyboardMgr::Instance();

	TWEAKABLE float32 maxYaw = Angle::FromDeg(30.f);
	TWEAKABLE float32 maxPitch = Angle::FromDeg(30.f);
	TWEAKABLE float32 maxRoll = Angle::FromDeg(30.f);

	TWEAKABLE float32 timeToTarget = 1.25f;
	TWEAKABLE float32 factor = 0.99f;
	TWEAKABLE float32 tolerance = Angle::FromDeg(1.f);

	if (kbMgr[vkeyShipLeft].IsDown())
	{
		m_targetAngles.yaw = -maxYaw;
	}
	else if (kbMgr[vkeyShipRight].IsDown())
	{
		m_targetAngles.yaw = maxYaw;
	}
	else if (kbMgr[vkeyShipLeft].IsUp() && kbMgr[vkeyShipRight].IsUp())
	{
		m_targetAngles.yaw = 0.f;
	}

	if (kbMgr[vkeyShipDown].IsDown())
	{
		m_targetAngles.pitch = -maxPitch;
	}
	else if (kbMgr[vkeyShipUp].IsDown())
	{
		m_targetAngles.pitch = maxPitch;
	}
	else if (kbMgr[vkeyShipDown].IsUp() && kbMgr[vkeyShipUp].IsUp())
	{
		m_targetAngles.pitch = 0.f;
	}

	m_currAngles.yaw = ApproachDamped(m_currAngles.yaw, m_targetAngles.yaw, deltaTime, factor, timeToTarget, tolerance);
	m_currAngles.pitch = ApproachDamped(m_currAngles.pitch, m_targetAngles.pitch, deltaTime, factor, timeToTarget, tolerance);

	const float32 yawSign = MathEx::Sign<float32>(m_currAngles.yaw);
	m_currAngles.roll = MathEx::MapRangedValue(yawSign * m_currAngles.yaw, 0.f, maxYaw, 0.f, maxRoll) * -yawSign;

	auto& mLocal = GetSceneNode()->ModifyLocalToParent();
	mLocal.SetRotFromEulerAngles(m_currAngles);
}

void PlayerControlComponent::UpdateMovement(float32 deltaTime)
{
	auto& mLocal = GetSceneNode()->ModifyLocalToParent();

	Vector3 vMoveDelta = mLocal.axisZ * m_speed * deltaTime;

	// Move ship in local xy plane, but apply local z movement to anchor (parent)
		
	mLocal.trans.x += vMoveDelta.x;
	mLocal.trans.y += vMoveDelta.y;

	auto pAnchorComponent = GetSceneNode()->GetParent()->GetComponent<AnchorComponent>();
	pAnchorComponent->Advance(vMoveDelta.z);
}
