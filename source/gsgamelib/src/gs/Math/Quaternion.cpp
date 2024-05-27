#include "Quaternion.h"
#include "MathEx.h"
#include "Vector3.h"
#include "EulerAngles.h"
#include "Matrix43.h"
#include <cassert>

namespace
{
	float32 DotProduct(const Quaternion &a, const Quaternion &b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
	}
}

void Quaternion::SetFromAxisAngle(const Vector3& axis, float32 angle)
{
	assert(axis.IsUnit() && "Rotation axis must be normalized");

	const float32 halfAngle = angle * 0.5f;
	const float32 sinHalfAngle = MathEx::Sin(halfAngle);

	x = axis.x * sinHalfAngle;
	y = axis.y * sinHalfAngle;
	z = axis.z * sinHalfAngle;
	w = MathEx::Cos(halfAngle);
}

void Quaternion::SetFromEulerAngles(const EulerAngles& eulerAngles)
{
	float32 sy, sp, sr;
	float32	cy, cp, cr;
	MathEx::SinCos(eulerAngles.yaw * 0.5f, sy, cy);
	MathEx::SinCos(eulerAngles.pitch * 0.5f, sp, cp);
	MathEx::SinCos(eulerAngles.roll * 0.5f, sr, cr);

	w =  cy*cp*cr + sy*sp*sr;
	x =  cy*sp*cr + sy*cp*sr;
	y = -cy*sp*sr + sy*cp*cr;
	z = -sy*sp*cr + cy*cp*sr;
}

void Quaternion::SetFromMatrix(const Matrix43& m)
{
	// Determine which of w, x, y, or z has the largest absolute value
	float32 fourWSquaredMinus1 = m.m11 + m.m22 + m.m33;
	float32 fourXSquaredMinus1 = m.m11 - m.m22 - m.m33;
	float32 fourYSquaredMinus1 = m.m22 - m.m11 - m.m33;
	float32 fourZSquaredMinus1 = m.m33 - m.m11 - m.m22;
	
	int biggestIndex = 0;
	float32 fourBiggestSquaredMinus1 = fourWSquaredMinus1;
	if (fourXSquaredMinus1 > fourBiggestSquaredMinus1)
	{
		fourBiggestSquaredMinus1 = fourXSquaredMinus1;
		biggestIndex = 1;
	}
	if (fourYSquaredMinus1 > fourBiggestSquaredMinus1)
	{
		fourBiggestSquaredMinus1 = fourYSquaredMinus1;
		biggestIndex = 2;
	}
	if (fourZSquaredMinus1 > fourBiggestSquaredMinus1)
	{
		fourBiggestSquaredMinus1 = fourZSquaredMinus1;
		biggestIndex = 3;
	}
	
	// Perform square root and division
	float32 biggestVal = sqrt(fourBiggestSquaredMinus1 + 1.0f) * 0.5f;
	float32 mult = 0.25f / biggestVal;

	// Apply table to compute quaternion values
	switch (biggestIndex)
	{
	case 0:
		w = biggestVal;
		x = (m.m23 - m.m32) * mult;
		y = (m.m31 - m.m13) * mult;
		z = (m.m12 - m.m21) * mult;
		break;
	case 1:
		x = biggestVal;
		w = (m.m23 - m.m32) * mult;
		y = (m.m12 + m.m21) * mult;
		z = (m.m31 + m.m13) * mult;
		break;
	case 2:
		y = biggestVal;
		w = (m.m31 - m.m13) * mult;
		x = (m.m12 + m.m21) * mult;
		z = (m.m23 + m.m32) * mult;
		break;
	case 3:
		z = biggestVal;
		w = (m.m12 - m.m21) * mult;
		x = (m.m31 + m.m13) * mult;
		y = (m.m23 + m.m32) * mult;
		break;
	}
}

void Quaternion::ToAxisAngle(Vector3& axis, float32& angle) const
{
	angle = 2.f * MathEx::ACos(w);
	if (angle == 0)
	{
		axis.SetZero();
	}
	else
	{
		axis.Set(x, y, z);
		axis.Normalize();
	}
}

float32 Quaternion::GetAngle() const
{
	return 2.f * MathEx::ACos(w);
}

Quaternion Slerp(const Quaternion& q1, const Quaternion& _q2, float32 t)
{
	Quaternion q2 = _q2; //@TODO: pass in q2 by copy

	if (t <= 0.0f) return q1;
	if (t >= 1.0f) return q2;

	// Compute "cosine of angle between quaternions" using dot product
	const float32 cosAngle = DotProduct(q1, q2);
	assert(cosAngle < 1.1f); // Both quats should be unit length

	// Ensure shortest path
	if (cosAngle < 0.f)
		q2 = -q2;

	// Compute interpolation weights
	float32 w0, w1;
	if (cosAngle > 0.9999f)
	{
		// Quats are very close, just lerp to avoid divide by zero
		w0 = 1.f - t;
		w1 = t;
	}
	else
	{
		// Compute the sin of the angle using the trig identity sin^2(angle) + cos^2(angle) = 1
		const float32 sinAngle = MathEx::Sqrt(1.f - cosAngle*cosAngle);

		// Compute the angle from its sin and cosine
		const float32 angle = MathEx::ATan2(sinAngle, cosAngle);
		const float32 invAngle = 1.f / sinAngle;

		// Compute interpolation parameters
		w0 = sin((1.f - t) * angle) * invAngle;
		w1 = sin(t * angle) * invAngle;
	}

	// Interpolate
	return Quaternion(
		w0 * q1.x + w1 * q2.x,
		w0 * q1.y + w1 * q2.y,
		w0 * q1.z + w1 * q2.z,
		w0 * q1.w + w1 * q2.w
		);
}
