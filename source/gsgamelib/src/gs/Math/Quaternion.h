#ifndef _GS_QUATERNION_H_
#define _GS_QUATERNION_H_

#include "gs/Base/Base.h"
#include "MathEx.h"
#include "Vector3.h"

class Vector3;
class EulerAngles;
class Matrix43;

class Quaternion
{
public:
	float32 x, y, z, w;

	Quaternion() {}
	Quaternion(float32 x, float32 y, float32 z, float32 w) : x(x), y(y), z(z), w(w) {}

	static const Quaternion& Identity() { static Quaternion q(0.f, 0.f, 0.f, 1.f); return q; }

	void Set(float32 x, float32 y, float32 z, float32 w) { this->x = x; this->y = y; this->z = z; this->w = w; }
	void SetIdentity() { *this = Identity(); }

	void SetFromAxisAngle(const Vector3& axis, float32 angle);
	void SetFromEulerAngles(const EulerAngles& eulerAngles);
	void SetFromMatrix(const Matrix43& m);

	float32 Length() const { return MathEx::Sqrt(x*x + y*y + z*z + w*w); }

	void Normalize();

	// Sets to the (multiplicative) inverse. This function actually sets to the conjugate, but since we assume
	// quaternion is unit, then conjugate == inverse (inverse = conjugate / quat.length)
	void Invert();

	bool AlmostEquals(const Quaternion& rhs, float32 epsilon = kEpsilon);

	Vector3 AxisX() const;
	Vector3 AxisY() const;
	Vector3 AxisZ() const;

	void ToAxisAngle(Vector3& axis, float32& angle) const;
	float32 GetAngle() const;

private:
	friend Vector3 operator*(const Vector3& v, const Quaternion& q);
	Vector3 RotatedVector(Vector3 v) const;
};

// Quaternion "cross product": concatenates rotations
// *NOTE: Order of multiplication from left to right corresponds to order
// rotations are applied. This is backwards from "standard" definitions,
// but is done to conform to the left-to-right transform concatenation order
// in the rest of the math lib (like for matrices).
inline Quaternion operator*(const Quaternion& lhs, const Quaternion& rhs)
{
	return Quaternion(
		lhs.w*rhs.x + lhs.x*rhs.w + lhs.z*rhs.y - lhs.y*rhs.z,
		lhs.w*rhs.y + lhs.y*rhs.w + lhs.x*rhs.z - lhs.z*rhs.x,
		lhs.w*rhs.z + lhs.z*rhs.w + lhs.y*rhs.x - lhs.x*rhs.y,
		lhs.w*rhs.w - lhs.x*rhs.x - lhs.y*rhs.y - lhs.z*rhs.z
		);
}

// Transforms (rotates) vector by quaternion
inline Vector3 operator*(const Vector3& v, const Quaternion& q)
{
	return q.RotatedVector(v);
}

inline bool operator==(const Quaternion& lhs, const Quaternion& rhs)
{
	return (lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.w == rhs.w)
		|| (lhs.x == -rhs.x && lhs.y == -rhs.y && lhs.z == -rhs.z && lhs.w == -rhs.w);
}

inline bool operator!=(const Quaternion& lhs, const Quaternion& rhs)
{
	return !(lhs == rhs);
}

inline Quaternion operator-(const Quaternion& q)
{
	return Quaternion(-q.x, -q.y, -q.z, -q.w);
}

inline void Quaternion::Invert()
{
	assert( MathEx::AlmostEquals(Length(), 1.f) );
	// Flip axis of rotation
	x = -x;
	y = -y;
	z = -z;
}

inline bool Quaternion::AlmostEquals(const Quaternion& rhs, float32 epsilon)
{
	return (MathEx::AlmostEquals(x, rhs.x, epsilon) && MathEx::AlmostEquals(y, rhs.y, epsilon) && MathEx::AlmostEquals(z, rhs.z, epsilon) && MathEx::AlmostEquals(w, rhs.w, epsilon))
		|| (MathEx::AlmostEquals(x, -rhs.x, epsilon) && MathEx::AlmostEquals(y, -rhs.y, epsilon) && MathEx::AlmostEquals(z, -rhs.z, epsilon) && MathEx::AlmostEquals(w, -rhs.w, epsilon));
}

inline Quaternion Invert(const Quaternion& q)
{
	Quaternion result = q;
	result.Invert();
	return result;
}

inline Vector3 Quaternion::RotatedVector(Vector3 v) const
{
	float32 length = v.Length();
	v.Normalize();

	// Q-1 * v * Q
	Quaternion qR = ::Invert(*this) * Quaternion(v.x, v.y, v.z, 0.f) * (*this);
	assert( MathEx::AlmostEquals(qR.w, 0.f) );
	return Vector3(qR.x, qR.y, qR.z) * length;
}

inline Vector3 Quaternion::AxisX() const
{
	return RotatedVector(Vector3::UnitX());
}

inline Vector3 Quaternion::AxisY() const
{
	return RotatedVector(Vector3::UnitY());
}

inline Vector3 Quaternion::AxisZ() const
{
	return RotatedVector(Vector3::UnitZ());
}

Quaternion Slerp(const Quaternion& q1, const Quaternion& q2, float32 t);


#endif // _GS_QUATERNION_H_
