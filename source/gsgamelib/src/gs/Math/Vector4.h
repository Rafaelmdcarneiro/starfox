#ifndef _VECTOR4_H_
#define _VECTOR4_H_

// Vector4 is provided mostly for convenience, like when storing data from third party libraries.
// Vector4 can store values, and a few useful operations are available for them, such as multiplying by Matrix43;
// however, Vector3 should be used for most math operations.

#include "MathEx.h"
#include <cassert>
#include <string>

class Vector3;

class Vector4
{
public:
	union
	{
		struct { float32 x, y, z, w; };
		float32 v[4];
	};

	Vector4() {}
	Vector4(float32 x, float32 y, float32 z, float32 w) : x(x), y(y), z(z), w(w) {}
	
	// Convert from Vector3
	Vector4(const Vector3& v3, float32 w);

	void Set(float32 x, float32 y, float32 z, float32 w) { this->x = x; this->y = y; this->z = z; this->w = w; }

	bool AlmostEquals(const Vector4& rhs, float32 epsilon = kEpsilon) const;
	
	std::string ToString() const;
};

inline bool Vector4::AlmostEquals(const Vector4& rhs, float32 epsilon) const
{
	return MathEx::AlmostEquals(x, rhs.x, epsilon)
		&& MathEx::AlmostEquals(y, rhs.y, epsilon)
		&& MathEx::AlmostEquals(z, rhs.z, epsilon)
		&& MathEx::AlmostEquals(w, rhs.w, epsilon);
}





/*
class ostream;

// Vector4 represents a homogenous 4-element vector

class Vector4
{
public:
	union
	{
		struct { float32 x, y, z, w; };
		float32 v[4];
	};

	///////////////////////////////////////////////////////////////////////////
	// Standard operations
	///////////////////////////////////////////////////////////////////////////

	Vector4() {}
	Vector4(float32 x, float32 y, float32 z, float32 w) : x(x), y(y), z(z), w(w) {}

	void Reset(float32 x, float32 y, float32 z, float32 w=1.0f) { this->x = x; this->y = y; this->z = z; this->w = w; }

	static const Vector4& ZeroPoint()	{ static Vector4 v(0,0,0,1); return v; }
	static const Vector4& Zero()		{ static Vector4 v(0,0,0,0); return v; }
	static const Vector4& UnitX()		{ static Vector4 v(1,0,0,0); return v; }
	static const Vector4& UnitY()		{ static Vector4 v(0,1,0,0); return v; }
	static const Vector4& UnitZ()		{ static Vector4 v(0,0,1,0); return v; }
	static const Vector4& UnitW()		{ static Vector4 v(0,0,0,1); return v; }

	bool AlmostEquals(const Vector4& rhs, float32 epsilon=kEpsilon)
	{
		return ( MathEx::AlmostEquals(x, rhs.x, epsilon) && MathEx::AlmostEquals(y, rhs.y, epsilon) && MathEx::AlmostEquals(z, rhs.z, epsilon) );
	}

	///////////////////////////////////////////////////////////////////////////
	// Operations that do not modify the vector state (const)
	///////////////////////////////////////////////////////////////////////////
	
	// Returns vector length (magnitude)
	float32 Length() const { return MathEx::Sqrt(x*x + y*y + z*z); }

	// Returns vector magnitude sqaured
	float32 LengthSquared() const { return x*x + y*y + z*z; }

	// Returns true if it's a unit vector
	bool IsUnit() const { return MathEx::AlmostEquals(LengthSquared(), 1.0f); }

	// Returns dot product
	float32 Dot(const Vector4& rhs) const { return (x * rhs.x + y * rhs.y + z * rhs.z + w * rhs.w); }

	// Returns resultant vector of cross product
	// Precondition: the two vectors must not be points (w must be 0.0f)
	Vector4 Cross(const Vector4& rhs)
	{
		assert(w == 0.0f && rhs.w == 0.0f && "Cross product not valid on points!");
		return Vector4(y*rhs.z - z*rhs.y, z*rhs.x - x*rhs.z, x*rhs.y - y*rhs.x, 0.0f);
	}

	// Returns normalized vector
	Vector4 Normalized() const
	{
		Vector4 vec(*this);
		vec.Normalize();
		return vec;
	}


	///////////////////////////////////////////////////////////////////////////
	// Operations that modify the vector state
	///////////////////////////////////////////////////////////////////////////

	// Normalizes the vector (scales down to unit length)
	void Normalize()
	{
		float32 length = Length();
		assert(length > 0.f);
		*this /= length;
	}

	void SafeNormalize(const Vector4& vecIfZeroLength)
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

	void operator*=(float32 k) { x *= k; y *= k; z *= k; w *= k; }
	void operator/=(float32 k) { x /= k; y /= k; z /= k; w /= k; }
	void operator+=(const Vector4& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w; }
	void operator-=(const Vector4& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; w -= rhs.w; }

	std::string ToString() const;
};

//inline std::ostream& operator<<(std::ostream& sout, const Vector4& rhs)
//{
//	sout << rhs.ToString(); 
//	return sout;
//}

inline bool operator==(const Vector4& lhs, const Vector4& rhs)
{
	return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

inline bool operator!=(const Vector4& lhs, const Vector4& rhs)
{
	return !(lhs == rhs);
}

inline Vector4 operator-(const Vector4& v) { return Vector4(-v.x, -v.y, -v.z, -v.w); }

#define MAKE_VECTOR_OP(op, v1, v2) Vector4(v1.x op v2.x, v1.y op v2.y, v1.z op v2.z, v1.w op v2.w)
#define MAKE_VECTOR_SCALE_OP(op, v, k) Vector4(v.x op k, v.y op k, v.z op k, v.w op k)

inline Vector4 operator*(float32 k, const Vector4& rhs) { return MAKE_VECTOR_SCALE_OP(*, rhs, k); }
inline Vector4 operator*(const Vector4& lhs, float32 k) { return MAKE_VECTOR_SCALE_OP(*, lhs, k); }
inline Vector4 operator/(const Vector4& rhs, float32 k) { return MAKE_VECTOR_SCALE_OP(/, rhs, k); }

inline Vector4 operator+(const Vector4& lhs, const Vector4& rhs) { return MAKE_VECTOR_OP(+, lhs, rhs); }
inline Vector4 operator-(const Vector4& lhs, const Vector4& rhs) { return MAKE_VECTOR_OP(-, lhs, rhs); }

#undef MAKE_VECTOR_OP
#undef MAKE_VECTOR_SCALE_OP
*/
#endif // _VECTOR4_H_
