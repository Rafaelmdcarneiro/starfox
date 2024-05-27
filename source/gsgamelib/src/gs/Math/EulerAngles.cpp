#include "EulerAngles.h"
#include "Quaternion.h"
#include "Matrix43.h"
#include <cassert>

void EulerAngles::SetFromQuaternion(const Quaternion& q)
{
	assert(false && "TO TEST");

	// Extract sin(pitch)
	float32 sp = -2.0f * (q.y*q.z - q.w*q.x);

	// Check for Gimbal lock, giving slight tolerance for numerical imprecision
	if (fabs(sp) > 0.9999f)
	{
		// Looking straight up or down
		pitch = kPiOver2 * sp;

		// Compute yaw, slam roll to zero
		yaw = MathEx::ATan2(-q.x*q.z + q.w*q.y, 0.5f - q.y*q.y - q.z*q.z);
		roll = 0.0f;
	}
	else
	{
		// Compute angles.  We don't have to use the "safe" asin function because we already checked for range errors when
		// checking for Gimbal lock
		pitch = MathEx::ASin(sp);
		yaw = MathEx::ATan2(q.x*q.z + q.w*q.y, 0.5f - q.x*q.x - q.y*q.y);
		roll = MathEx::ATan2(q.x*q.y + q.w*q.z, 0.5f - q.x*q.x - q.z*q.z);
	}
}

void EulerAngles::SetFromMatrix(const Matrix43& m)
{
	//assert(false && "TO TEST");

	// Extract sin(pitch) from m32.
	float32 sp = -m.m32;

	// Check for Gimbal lock	
	if (fabs(sp) > 0.9999f)
	{
		// Looking straight up or down
		pitch = kPiOver2 * sp;

		// Compute yaw, slam roll to zero
		yaw = atan2(-m.m23, m.m11);
		roll = 0.0f;
	}
	else
	{
		// Compute angles.  We don't have to use the "safe" asin function because we already checked for range errors when
		// checking for Gimbel lock
		pitch = MathEx::ASin(sp);
		yaw = MathEx::ATan2(m.m31, m.m33);
		roll = MathEx::ATan2(m.m12, m.m22);
	}
}

void EulerAngles::Canonize()
{
	using namespace MathEx;

	// First, wrap pitch in range [-kPi, kPi]
	pitch.NormalizePI();

	// If pitch is outside the canonical range of [-kPi/2, kPi/2],
	// we flip around yaw and roll and correct pitch
	if (pitch < -kPiOver2)
	{
		pitch = -kPi - pitch;
		yaw += kPi;
		roll += kPi;
	}
	else if (pitch > kPiOver2)
	{
		pitch = kPi - pitch;
		yaw += kPi;
		roll += kPi;
	}

	// OK, now check for the gimbal lock case (within a slight tolerance)
	if (MathEx::Abs(pitch) > kPiOver2 - 1e-4)
	{
		// We are in gimbal lock.  Assign all rotation about the vertical axis to yaw
		yaw += roll;
		roll = 0.0f;
	}
	else
	{
		// Not in gimbal lock.  Wrap the roll angle in canonical range 
		roll.NormalizePI();
	}

	// Wrap yaw in canonical range
	yaw.NormalizePI();
}
