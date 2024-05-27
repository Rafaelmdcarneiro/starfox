#include "CameraComponent.h"
#include "GameInput.h"
#include "gs/Math/MathUtil.h"

void OrbitTargetCameraComponent::Update(float32 deltaTime)
{
	auto pTarget = m_pwTarget.lock();
	if (!pTarget)
		return;

	KeyboardMgr& kbMgr = KeyboardMgr::Instance();		
	static float32 cameraTurnRate = kPiOver2;
	static float32 zoomRate = 100.f;

	if (kbMgr[vkeyCamUp].IsDown() || kbMgr[vkeyCamDown].IsDown())
	{
		const float32 turnAngle = (kbMgr[vkeyCamUp].IsDown()? -1.f : 1.f) * (cameraTurnRate * deltaTime);
		m_angles.pitch += turnAngle;
	}
	if (kbMgr[vkeyCamLeft].IsDown() || kbMgr[vkeyCamRight].IsDown())
	{
		const float32 turnAngle = (kbMgr[vkeyCamLeft].IsDown()? -1.f : 1.f) * (cameraTurnRate * deltaTime);
		m_angles.yaw += turnAngle;
	}
	if (kbMgr[vkeyCamZoomIn].IsDown() || kbMgr[vkeyCamZoomOut].IsDown())
	{
		const float32 delta = (kbMgr[vkeyCamZoomIn].IsDown()? -1.f : 1.f) * (zoomRate * deltaTime);
		m_offset = MathEx::Max(0.f, m_offset + delta);
	}
	if (kbMgr[vkeyCamReset].JustPressed())
	{
		m_angles = EulerAngles::Zero();
		m_offset = 250.f;
	}
	//m_angles.Canonize();

	auto& mCamera = GetSceneNode()->ModifyLocalToParent();
	mCamera.SetFromEulerAngles(m_angles);
	mCamera.trans = pTarget->GetLocalToWorld().trans + DirectionVector(Vector3(0,0,-m_offset)) * mCamera;
}

void FollowShipCameraComponent::UpdateTransform(float32 deltaTime, bool damp)
{
	auto pTarget = m_pwTarget.lock();
	if (!pTarget)
		return;

	auto psAnchor = pTarget->GetParent();
	const Matrix43& mAnchorWorld = psAnchor->GetLocalToWorld();
	const Matrix43& mShipWorld = pTarget->GetLocalToWorld();		
	Matrix43& mCamWorld = GetSceneNode()->ModifyLocalToWorld();

	// First thing, we want the camera to be a fixed distance behind the ship. We do
	// this in anchor space since the ship yaws.
	Matrix43 mInvAnchorWorld; mInvAnchorWorld.SetInverseFromSRT(mAnchorWorld);
	Vector3 vCamPosInAnchorSpace = PositionVector(mCamWorld.trans) * mInvAnchorWorld;
	vCamPosInAnchorSpace.z = -m_offset;
	mCamWorld.trans = PositionVector(vCamPosInAnchorSpace) * mAnchorWorld;

	// Now approach the camera in the xy plane to be behind the ship
	TWEAKABLE float32 timeToTarget = 0.5f;
	Vector3 vCamDestPos = mShipWorld.trans - mAnchorWorld.axisZ * m_offset;
	Vector3 vCamFinalPos = damp? ApproachDamped(mCamWorld.trans, vCamDestPos, deltaTime, 0.99f, timeToTarget, 1.f) : vCamDestPos;

	//TWEAKABLE float32 approachSpeed = 100.f;
	//Vector3 vCamFinalPos = damp? ApproachLinear(mCamWorld.trans, vCamDestPos, deltaTime, approachSpeed) : vCamDestPos;


	//TWEAKABLE float32 maxAllowedDistX = 50.f;
	//TWEAKABLE float32 maxAllowedDistY = 50.f;
	//Vector3 vDelta = vCamDestPos - mCamWorld.trans;
	//assert(vDelta.z == 0.f);
	//Vector3 vCamFinalPos = mCamWorld.trans;
	//const bool outsideAllowedDistX = MathEx::Abs(vDelta.x) > maxAllowedDistX;
	//const bool outsideAllowedDistY = MathEx::Abs(vDelta.y) > maxAllowedDistY;
	//if (outsideAllowedDistX || outsideAllowedDistY)
	//{
	//	Vector3 vDeltaNorm = Normalize(vDelta);

	//	vCamDestPos = mCamWorld.trans;

	//	if (outsideAllowedDistX)
	//	{
	//		vCamDestPos.x = mCamWorld.trans.x + (vDelta.x - vDeltaNorm.x * maxAllowedDistX);
	//	}

	//	if (outsideAllowedDistY)
	//	{
	//		vCamDestPos.y = mCamWorld.trans.y + (vDelta.y - vDeltaNorm.y * maxAllowedDistX);
	//	}

	//	vCamFinalPos = damp? ApproachDamped(mCamWorld.trans, vCamDestPos, deltaTime, 0.99f, timeToTarget, 1.f) : vCamDestPos;
	//}

	////// Approach point that brings the ship into the allowable region
	//////vCamDestPos = mCamWorld.trans + (vDelta - (Normalize(vDelta) * maxAllowedDistXY));



	EulerAngles anchorAngles; anchorAngles.SetFromMatrix(mAnchorWorld);
	EulerAngles shipAngles; shipAngles.SetFromMatrix(mShipWorld);
	EulerAngles camAngles; camAngles.SetFromMatrix(mCamWorld);

	TWEAKABLE float32 camToShipRollRatio = 0.2f;
	TWEAKABLE float32 timeToTarget3 = 2.f;
	EulerAngles camFinalAngles = anchorAngles;
	camFinalAngles.roll = ApproachDamped(camAngles.roll, shipAngles.roll * camToShipRollRatio, deltaTime, 0.99f, timeToTarget3);

	mCamWorld.SetFromEulerAngles(camFinalAngles, vCamFinalPos);

	// Look at the ship
	//const Vector3 vCamCurrForward = mCamWorld.axisZ;
	//const Vector3 vCamTargetForward = Normalize(mShipWorld.trans - vCamFinalPos);
	//TWEAKABLE float32 timeToTarget2 = 2.f;
	//const Vector3 vCamFinalForward = ApproachDirectionDamped(vCamCurrForward, vCamTargetForward, deltaTime, 0.99f, timeToTarget2);//, 0.1f);

	//mCamWorld.SetFromLookAtDir(vCamFinalForward, Vector3::UnitY(), vCamFinalPos);
}

