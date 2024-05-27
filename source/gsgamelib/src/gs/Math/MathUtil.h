#ifndef __MATH_UTIL_H__
#define __MATH_UTIL_H__

#include "Vector3.h"
#include "Quaternion.h"

// Returns quaternion such that v1 * q = v2
inline Quaternion ComputeRotationBetweenDirections(const Vector3& v1, const Vector3& v2)
{
	assert(v1.IsUnit() && v2.IsUnit());

	const Vector3& vUp = v1.Cross(v2);
	const float32 sinAngle = vUp.Length();

	if ( MathEx::AlmostEquals(sinAngle, 0.f) || MathEx::AlmostEquals(sinAngle, kPi) )
		return Quaternion::Identity();	

	const float32 angle = MathEx::ASin(sinAngle);

	Quaternion q1;
	q1.SetFromAxisAngle(Normalize(vUp), angle);
	return q1;
}

// Approaches current towards target at constant speed, clamping to target upon (over)reaching it
inline float32 ApproachLinear(float32 current, float32 target, float32 deltaTime, float32 speed)
{
	const float32 delta = target - current;
	const float32 deltaSign = MathEx::Sign(delta);
	const float32 step = deltaSign * speed * deltaTime;

	if (MathEx::Abs(step) > MathEx::Abs(delta))
		return target;

	return current + step;
}

inline Vector3 ApproachLinear(Vector3 current, Vector3 target, float32 deltaTime, float32 speed)
{
	const Vector3 delta = target - current;
	const Vector3 deltaNorm = SafeNormalize(delta, Vector3::Zero());
	const Vector3 step = deltaNorm * speed * deltaTime;

	if (step.Length() > delta.Length())
		return target;

	return current + step;
}

// Approaches current towards target using a viscous damped approach; by default, it will reach
// 99% (factor) of target within 1 (timeToTarget) second, then will keep getting closer but never
// reach the (asymptotic) target. The tolerance can be set to make sure the target is reached, but
// keep in mind that this modifies the shape of the curve (slightly).
inline float32 ApproachDamped(float32 current, float32 target, float32 deltaTime, float32 factor = 0.99f, float32 timeToTarget = 1.f, float32 tolerance = 0.f)
{
	if (current == target || timeToTarget == 0.f)
		return target;

	const float32 delta = target - current;
	const float32 deltaSign = MathEx::Sign(delta);
	const float32 newTarget = target + deltaSign * tolerance;

	const float32 alpha = 1.f - MathEx::Pow(1.f - factor, deltaTime / timeToTarget);
	const float32 step = (newTarget - current) * alpha;

	if ( (step * deltaSign) > (delta * deltaSign) )
		return target;

	return current + step;
}

inline Vector3 ApproachDamped(const Vector3& current, const Vector3& target, float32 deltaTime, float32 factor = 0.99f, float32 timeToTarget = 1.f, float32 tolerance = 0.f)
{
	if (current == target || timeToTarget == 0.f)
		return target;

	const Vector3 delta = target - current;
	const Vector3 newTarget = target + Normalize(delta) * tolerance;

	const float32 alpha = 1.f - MathEx::Pow(1.f - factor, deltaTime / timeToTarget);
	const Vector3 step = (newTarget - current) * alpha;

	if (step.LengthSquared() > delta.LengthSquared())
		return target;

	return current + step;
}

// Use to "turn" a direction vector towards a target direction using a viscous damped approach
inline Vector3 ApproachDirectionDamped(const Vector3& current, const Vector3& target, float32 deltaTime, float32 factor = 0.99f, float32 timeToTarget = 1.f, float32 tolerance = 0.f)
{
	assert(current.IsUnit() && target.IsUnit());
	
	Quaternion qDelta = ComputeRotationBetweenDirections(current, target);
	
	Vector3 axis;
	float32 angle;
	qDelta.ToAxisAngle(axis, angle);

	if (angle == 0.f)
	{
		return target;
	}

	angle = ApproachDamped(0.f, angle, deltaTime, factor, timeToTarget, tolerance);
	qDelta.SetFromAxisAngle(axis, angle);
	return Normalize( current * qDelta );
}

#endif // __MATH_UTIL_H__
