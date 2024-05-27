#ifndef _VECTOR3_H_
#define _VECTOR3_H_

#include "MathEx.h"
#include <cassert>
#include <string>

class Vector4;

class Vector3
{
public:
	union
	{
		struct { float32 x, y, z; };
		float32 v[3];
	};

	Vector3() {}
	Vector3(float32 x, float32 y, float32 z) : x(x), y(y), z(z) {}

	// Convert from Vector4
	explicit Vector3(const Vector4& v4);

	void Set(float32 x, float32 y, float32 z) { this->x = x; this->y = y; this->z = z; }
	void SetZero() { *this = Zero(); }

	static const Vector3& Zero()	{ static Vector3 v(0.f, 0.f, 0.f); return v; }
	static const Vector3& UnitX()	{ static Vector3 v(1.f, 0.f, 0.f); return v; }
	static const Vector3& UnitY()	{ static Vector3 v(0.f, 1.f, 0.f); return v; }
	static const Vector3& UnitZ()	{ static Vector3 v(0.f, 0.f, 1.f); return v; }

	float32 Length() const { return MathEx::Sqrt(x*x + y*y + z*z); }
	float32 LengthSquared() const { return x*x + y*y + z*z; }
	bool IsZero() const;
	bool IsUnit() const;
	bool AlmostEquals(const Vector3& rhs, float32 epsilon = kEpsilon);

	float32 Dot(const Vector3& rhs) const { return (x * rhs.x + y * rhs.y + z * rhs.z); }
	Vector3 Cross(const Vector3& rhs) const { return Vector3(y*rhs.z - z*rhs.y, z*rhs.x - x*rhs.z, x*rhs.y - y*rhs.x); }

	void Normalize()
	{
		float32 length = Length();
		assert(length > 0.f);
		*this /= length;
	}

	void SafeNormalize(const Vector3& vecIfZeroLength)
	{
		float32 length = Length();
		if (length > 0.f)
		{
			*this /= length;
		}
		else
		{
			*this = vecIfZeroLength;
		}
	}

	void operator*=(float32 k) { x *= k; y *= k; z *= k; }
	void operator/=(float32 k) { x /= k; y /= k; z /= k; }
	void operator+=(const Vector3& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; }
	void operator-=(const Vector3& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; }
	
	std::string ToString() const;
};

inline bool operator==(const Vector3& lhs, const Vector3& rhs)
{
	return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

inline bool operator!=(const Vector3& lhs, const Vector3& rhs)
{
	return !(lhs == rhs);
}

inline Vector3 operator-(const Vector3& v)
{
	return Vector3(-v.x, -v.y, -v.z);
}

#define MAKE_VECTOR_OP(op, v1, v2) Vector3(v1.x op v2.x, v1.y op v2.y, v1.z op v2.z)
#define MAKE_VECTOR_SCALE_OP(op, v, k) Vector3(v.x op k, v.y op k, v.z op k)

inline Vector3 operator*(float32 k, const Vector3& rhs) { return MAKE_VECTOR_SCALE_OP(*, rhs, k); }
inline Vector3 operator*(const Vector3& lhs, float32 k) { return MAKE_VECTOR_SCALE_OP(*, lhs, k); }
inline Vector3 operator/(const Vector3& rhs, float32 k) { return MAKE_VECTOR_SCALE_OP(/, rhs, k); }
inline Vector3 operator+(const Vector3& lhs, const Vector3& rhs) { return MAKE_VECTOR_OP(+, lhs, rhs); }
inline Vector3 operator-(const Vector3& lhs, const Vector3& rhs) { return MAKE_VECTOR_OP(-, lhs, rhs); }

#undef MAKE_VECTOR_OP
#undef MAKE_VECTOR_SCALE_OP

inline bool Vector3::IsZero() const
{
	return *this == Vector3::Zero();
}

inline bool Vector3::IsUnit() const
{
	return MathEx::AlmostEquals(LengthSquared(), 1.0f);
}

inline bool Vector3::AlmostEquals(const Vector3& rhs, float32 epsilon)
{
	return MathEx::AlmostEquals(x, rhs.x, epsilon)
		&& MathEx::AlmostEquals(y, rhs.y, epsilon)
		&& MathEx::AlmostEquals(z, rhs.z, epsilon);
}

inline Vector3 Normalize(const Vector3& v)
{
	Vector3 result = v;
	result.Normalize();
	return result;
}

inline Vector3 SafeNormalize(const Vector3& v, const Vector3& vecIfZeroLength)
{
	Vector3 result = v;
	result.SafeNormalize(vecIfZeroLength);
	return result;
}

#endif // _VECTOR3_H_
