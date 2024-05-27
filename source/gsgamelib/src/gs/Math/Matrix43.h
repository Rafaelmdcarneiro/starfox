#ifndef _MATRIX43_H_
#define _MATRIX43_H_

#include "Vector3.h"
#include "Vector4.h"

class EulerAngles;
class Quaternion;

// Matrix43 represents a 4x3 matrix that can contain any 3D affine
// transformation. It can be treated like a 4x4 where the last column
// is assumed to be [0,0,0,1].
// Coordinate system is left-handed with Z forward, Y up, and X right.

class Matrix43
{
public:
	union
	{
		float32 m[4][3];

		struct
		{
			float32 m11, m12, m13;
			float32 m21, m22, m23;
			float32 m31, m32, m33;
			float32 m41, m42, m43; // Translation row
		};

		// Useful representation for accessing basis and translation vectors as Vector3s
		struct
		{
			Vector3 axisX;
			Vector3 axisY;
			Vector3 axisZ;
			Vector3 trans;
		};
	};

	Matrix43() {}

	Matrix43(const Vector3& axisX, const Vector3& axisY, const Vector3& axisZ, const Vector3& trans)
		: axisX(axisX), axisY(axisY), axisZ(axisZ), trans(trans) {}

	static const Matrix43& Identity()
	{
		static Matrix43 m(Vector3::UnitX(), Vector3::UnitY(), Vector3::UnitZ(), Vector3::Zero());
		return m;
	}

	void SetIdentity() { *this = Identity(); }

	// Conversions (init entire matrix)
	void SetFromTranslation(const Vector3& t);
	void SetFromScaleVector(const Vector3& scale, const Vector3& translation = Vector3::Zero());
	void SetFromAxisAngle(const Vector3& axis, float32 angle, const Vector3& translation = Vector3::Zero());
	void SetFromEulerAngles(const EulerAngles& eulerAngles, const Vector3& translation = Vector3::Zero());
	void SetFromQuaternion(const Quaternion& q, const Vector3& translation = Vector3::Zero());
	void SetFromLookAtDir(const Vector3& lookAt, const Vector3& up = Vector3::UnitY(), const Vector3& translation = Vector3::Zero());
	void SetFromLookAtPos(const Vector3& from, const Vector3& to, const Vector3& up = Vector3::UnitY());

	// Sets upper 3x3, doesn't modify translation
	void SetRotFromScaleVector(const Vector3& scale);
	void SetRotFromAxisAngle(const Vector3& axis, float32 angle);
	void SetRotFromEulerAngles(const EulerAngles& eulerAngles);
	void SetRotFromQuaternion(const Quaternion& q);
	void SetRotFromLookAtDir(const Vector3& lookAt, const Vector3& up = Vector3::UnitY());
	void SetRotFromMatrix(const Matrix43& m);

	const Vector3& AxisX() const { return axisX; }
	const Vector3& AxisY() const { return axisY; }
	const Vector3& AxisZ() const { return axisZ; }
	const Vector3& Translation() const { return trans; }

	// Inverts this matrix. These are ordered from most computationally expensive to least
	void Invert(); // For any arbitrary affine transform
	void InvertSRT(); // ...without shear
	void InvertUniformSRT(); // ...and with uniform scale
	void InvertRT(); // ...or no scaling ("rigid body" transform)
	void InvertR(); // ...and no translation (transposes)

	//@TODO: Replace these with free-standing versions of above that return inverted copy
	void SetInverseFrom(const Matrix43& m);
	void SetInverseFromSRT(const Matrix43& m);
	void SetInverseFromUniformSRT(const Matrix43& m);
	void SetInverseFromRT(const Matrix43& m);
	void SetInverseFromR(const Matrix43& m);

	bool AlmostEquals(const Matrix43& rhs, float32 epsilon = kEpsilon);

	float32 Determinant() const;

	// Makes matrix orthogonal (makes basis vectors orthogonal unit vectors)
	bool IsOrthogonal() const;
	void Orthogonalize();

	bool HasUniformScale() const;
	Vector3 GetScale() const;
	float32 GetUniformScale() const;

	std::string ToString() const;

private:
	friend Matrix43 operator*(const Matrix43& lhs, const Matrix43& rhs);
	friend Matrix43& operator*=(Matrix43& lhs, const Matrix43& rhs);
	Matrix43 Mul(const Matrix43& rhs) const;
	Matrix43& MulAssign(const Matrix43& rhs);

	friend Vector3 operator*(const struct DirectionVector& dir, const Matrix43& m);
	friend Vector3 operator*(const struct PositionVector& dir, const Matrix43& m);
	friend Vector4 operator*(const Vector4& v, const Matrix43& m);
	Vector3 TransformedPos(const Vector3& v) const;
	Vector3 TransformedDir(const Vector3& v) const;
};

static_assert(sizeof(Matrix43) == sizeof(float32) * 4 * 3, "Invalid size");

inline bool Matrix43::AlmostEquals(const Matrix43& rhs, float32 epsilon)
{
	return axisX.AlmostEquals(rhs.axisX, epsilon)
		&& axisY.AlmostEquals(rhs.axisY, epsilon)
		&& axisZ.AlmostEquals(rhs.axisZ, epsilon)
		&& trans.AlmostEquals(rhs.trans, epsilon);
}

inline void Matrix43::SetFromTranslation(const Vector3& t)
{
	axisX = Vector3::UnitX();
	axisY = Vector3::UnitY();
	axisZ = Vector3::UnitZ();
	trans = t;
}

inline void Matrix43::SetFromScaleVector(const Vector3& scale, const Vector3& translation)
{
	SetRotFromScaleVector(scale);
	trans = translation;
}

inline void Matrix43::SetFromAxisAngle(const Vector3& axis, float32 angle, const Vector3& translation)
{
	SetRotFromAxisAngle(axis, angle);
	trans = translation;
}

inline void Matrix43::SetFromEulerAngles(const EulerAngles& eulerAngles, const Vector3& translation)
{
	SetRotFromEulerAngles(eulerAngles);
	trans = translation;
}

inline void Matrix43::SetFromQuaternion(const Quaternion& q, const Vector3& translation)
{
	SetRotFromQuaternion(q);
	trans = translation;
}

inline void Matrix43::SetFromLookAtDir(const Vector3& lookAt, const Vector3& up, const Vector3& translation)
{
	SetRotFromLookAtDir(lookAt, up);
	trans = translation;
}

inline void Matrix43::SetFromLookAtPos(const Vector3& from, const Vector3& to, const Vector3& up)
{
	SetFromLookAtDir( Normalize(to - from), up, from );
}

inline void Matrix43::Invert()
{
	SetInverseFrom(Matrix43(*this));
}

inline void Matrix43::InvertSRT()
{
	SetInverseFromSRT(Matrix43(*this));
}

inline void Matrix43::InvertUniformSRT()
{
	SetInverseFromUniformSRT(Matrix43(*this));
}

inline void Matrix43::InvertRT()
{
	SetInverseFromRT(Matrix43(*this));
}

inline void Matrix43::InvertR()
{
	SetInverseFromR(Matrix43(*this));
}

inline bool Matrix43::IsOrthogonal() const
{
	return HasUniformScale()
		&& MathEx::AlmostEquals(axisX.Dot(axisY), 0.f)
		&& MathEx::AlmostEquals(axisY.Dot(axisZ), 0.f);
}

inline void Matrix43::Orthogonalize()
{
	// Make axes orthogonal
	axisY = axisZ.Cross(axisX);
	axisZ = axisX.Cross(axisY);
	// Makes axes unit length
	axisX = Normalize(axisX);
	axisY = Normalize(axisY);
	axisZ = Normalize(axisZ);
}

inline bool Matrix43::HasUniformScale() const
{
	return MathEx::AlmostEquals(axisX.Length(), axisY.Length()) && MathEx::AlmostEquals(axisY.Length(), axisZ.Length());
}

inline Vector3 Matrix43::GetScale() const
{
	return Vector3(axisX.Length(), axisY.Length(), axisZ.Length());
}

inline float32 Matrix43::GetUniformScale() const
{
	assert(HasUniformScale() && "Matrix does not contain uniform scale");
	return axisX.Length();
}

inline float32 Matrix43::Determinant() const
{
	// Determinant of 3x3
	return (m11 * (m22*m33 - m23*m32) + m12 * (m23*m31 - m21*m33) + m13 * (m21*m32 - m22*m31));
}

inline Vector3 Matrix43::TransformedPos(const Vector3& v) const
{
	return Vector3(
		v.x * m11 + v.y * m21 + v.z * m31 + m41,
		v.x * m12 + v.y * m22 + v.z * m32 + m42,
		v.x * m13 + v.y * m23 + v.z * m33 + m43
		);
}

inline Vector3 Matrix43::TransformedDir(const Vector3& v) const
{
	return Vector3(
		v.x * m11 + v.y * m21 + v.z * m31,
		v.x * m12 + v.y * m22 + v.z * m32,
		v.x * m13 + v.y * m23 + v.z * m33
		);
}

inline Matrix43 operator*(const Matrix43& lhs, const Matrix43& rhs)
{
	return lhs.Mul(rhs);
}

inline Matrix43& operator*=(Matrix43& lhs, const Matrix43& rhs)
{
	return lhs.MulAssign(rhs);
}

inline bool operator==(const Matrix43& lhs, const Matrix43& rhs)
{
	return lhs.axisX == rhs.axisX && lhs.axisY == rhs.axisY && lhs.axisZ == rhs.axisZ && lhs.trans == rhs.trans;
}

inline bool operator!=(const Matrix43& lhs, const Matrix43& rhs)
{
	return !(lhs == rhs);
}

// Vector transformation

// Proxy for a direction vector
struct DirectionVector
{
	const Vector3& v;
	DirectionVector(const Vector3& v) : v(v) {}

private:
	DirectionVector(const DirectionVector& rhs);
	DirectionVector& operator=(const DirectionVector& rhs);
};

// Proxy for a position vector
struct PositionVector
{
	const Vector3& v;
	PositionVector(const Vector3& v) : v(v) {}

private:
	PositionVector(const PositionVector& rhs);
	PositionVector& operator=(const PositionVector& rhs);
};

inline Vector3 operator*(const DirectionVector& dir, const Matrix43& m)
{
	return m.TransformedDir(dir.v);
}

inline Vector3 operator*(const PositionVector& pos, const Matrix43& m)
{
	return m.TransformedPos(pos.v);
}

inline Vector4 operator*(const Vector4& v, const Matrix43& m)
{
	return Vector4(
		v.x * m.m11 + v.y * m.m21 + v.z * m.m31 + v.w * m.m41,
		v.x * m.m12 + v.y * m.m22 + v.z * m.m32 + v.w * m.m42,
		v.x * m.m13 + v.y * m.m23 + v.z * m.m33 + v.w * m.m43,
		v.w // * 1
		);
}

#endif // _MATRIX43_H_
