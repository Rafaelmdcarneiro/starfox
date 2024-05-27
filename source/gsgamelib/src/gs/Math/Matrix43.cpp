#include "Matrix43.h"
#include "MathEx.h"
#include "gs/Base/string_helpers.h"
#include "EulerAngles.h"
#include "Quaternion.h"

namespace
{
	void MulImpl(const Matrix43& lhs, const Matrix43& rhs, Matrix43& r)
	{
		r.m11 = lhs.m11*rhs.m11 + lhs.m12*rhs.m21 + lhs.m13*rhs.m31;
		r.m12 = lhs.m11*rhs.m12 + lhs.m12*rhs.m22 + lhs.m13*rhs.m32;
		r.m13 = lhs.m11*rhs.m13 + lhs.m12*rhs.m23 + lhs.m13*rhs.m33;

		r.m21 = lhs.m21*rhs.m11 + lhs.m22*rhs.m21 + lhs.m23*rhs.m31;
		r.m22 = lhs.m21*rhs.m12 + lhs.m22*rhs.m22 + lhs.m23*rhs.m32;
		r.m23 = lhs.m21*rhs.m13 + lhs.m22*rhs.m23 + lhs.m23*rhs.m33;

		r.m31 = lhs.m31*rhs.m11 + lhs.m32*rhs.m21 + lhs.m33*rhs.m31;
		r.m32 = lhs.m31*rhs.m12 + lhs.m32*rhs.m22 + lhs.m33*rhs.m32;
		r.m33 = lhs.m31*rhs.m13 + lhs.m32*rhs.m23 + lhs.m33*rhs.m33;

		// Compute the translation portion
		r.m41 = lhs.m41*rhs.m11 + lhs.m42*rhs.m21 + lhs.m43*rhs.m31 + rhs.m41;
		r.m42 = lhs.m41*rhs.m12 + lhs.m42*rhs.m22 + lhs.m43*rhs.m32 + rhs.m42;
		r.m43 = lhs.m41*rhs.m13 + lhs.m42*rhs.m23 + lhs.m43*rhs.m33 + rhs.m43;
	}
}

void Matrix43::SetRotFromScaleVector(const Vector3& scale)
{
	axisX.Set(scale.x, 0.f, 0.f);
	axisY.Set(0.f, scale.y, 0.f);
	axisZ.Set(0.f, 0.f, scale.z);	
}

void Matrix43::SetRotFromAxisAngle(const Vector3& axis, float32 angle)
{
	assert(axis.IsUnit());

	float32 s, c;
	MathEx::SinCos(angle, s, c);

	// Compute 1 - cos(theta) and some common subexpressions
	const float32 a = 1.0f - c;
	const float32 ax = a * axis.x;
	const float32 ay = a * axis.y;
	const float32 az = a * axis.z;

	m11 = ax*axis.x + c;
	m12 = ax*axis.y + axis.z*s;
	m13 = ax*axis.z - axis.y*s;

	m21 = ay*axis.x - axis.z*s;
	m22 = ay*axis.y + c;
	m23 = ay*axis.z + axis.x*s;

	m31 = az*axis.x + axis.y*s;
	m32 = az*axis.y - axis.x*s;
	m33 = az*axis.z + c;
}

void Matrix43::SetRotFromEulerAngles(const EulerAngles& eulerAngles)
{
	float32	sh,ch, sp,cp, sb,cb;
	MathEx::SinCos(eulerAngles.yaw.rads, sh, ch);
	MathEx::SinCos(eulerAngles.pitch.rads, sp, cp);
	MathEx::SinCos(eulerAngles.roll.rads, sb, cb);

	// Fill in the matrix elements
	m11 = ch * cb + sh * sp * sb;
	m12 = sb * cp;
	m13 = -sh * cb + ch * sp * sb;

	m21 = -ch * sb + sh * sp * cb;
	m22 = cb * cp;
	m23 = sb * sh + ch * sp * cb;
	
	m31 = sh * cp;
	m32 = -sp;
	m33 = ch * cp;
}

void Matrix43::SetRotFromQuaternion(const Quaternion& q)
{
	float32 xx = 2.0f * q.x;
	float32 yy = 2.0f * q.y;
	float32 zz = 2.0f * q.z;
	float32 ww = 2.0f * q.w;

	m11 = 1.0f - yy*q.y - zz*q.z;
	m12 = xx*q.y + ww*q.z;
	m13 = xx*q.z - ww*q.y;

	m21 = xx*q.y - ww*q.z;
	m22 = 1.0f - xx*q.x - zz*q.z;
	m23 = yy*q.z + ww*q.x;

	m31 = xx*q.z + ww*q.y;
	m32 = yy*q.z - ww*q.x;
	m33 = 1.0f - xx*q.x - yy*q.y;
}

void Matrix43::SetRotFromLookAtDir(const Vector3& lookAt, const Vector3& up)
{
	assert(MathEx::Abs(lookAt.Dot(up)) < 1.f - kEpsilon && "Input vectors cannot be parallel");
	assert(lookAt.IsUnit() && up.IsUnit() && "Input vectors must be unit length");

	axisX = Normalize(up.Cross(lookAt));
	axisY = lookAt.Cross(axisX);
	axisZ = lookAt;
}

void Matrix43::SetRotFromMatrix(const Matrix43& m)
{
	axisX = m.axisX;
	axisY = m.axisY;
	axisZ = m.axisZ;
}

void Matrix43::SetInverseFrom(const Matrix43& m)
{
	const float32 det = m.Determinant();
	assert(det != 0.0f && "Cannot invert singular matrix");

	const float32 invDet = 1.f / det;

	// Compute the 3x3 portion of the inverse, by dividing the adjoint by the determinant
	m11 = (m.m22 * m.m33 - m.m23 * m.m32) * invDet;
	m12 = (m.m13 * m.m32 - m.m12 * m.m33) * invDet;
	m13 = (m.m12 * m.m23 - m.m13 * m.m22) * invDet;
	m21 = (m.m23 * m.m31 - m.m21 * m.m33) * invDet;
	m22 = (m.m11 * m.m33 - m.m13 * m.m31) * invDet;
	m23 = (m.m13 * m.m21 - m.m11 * m.m23) * invDet;
	m31 = (m.m21 * m.m32 - m.m22 * m.m31) * invDet;
	m32 = (m.m12 * m.m31 - m.m11 * m.m32) * invDet;
	m33 = (m.m11 * m.m22 - m.m12 * m.m21) * invDet;

	// Back-transform translation
	m41 = -(m.m41 * m11 + m.m42 * m21 + m.m43 * m31);
	m42 = -(m.m41 * m12 + m.m42 * m22 + m.m43 * m32);
	m43 = -(m.m41 * m13 + m.m42 * m23 + m.m43 * m33);
}

void Matrix43::SetInverseFromSRT(const Matrix43& m)
{
	const float32 invScaleSquaredX = 1.f / (m.axisX.LengthSquared());
	const float32 invScaleSquaredY = 1.f / (m.axisY.LengthSquared());
	const float32 invScaleSquaredZ = 1.f / (m.axisZ.LengthSquared());

    // Transpose upper 3x3 and divide by scale
	m11 = m.m11 * invScaleSquaredX;
	m12 = m.m21 * invScaleSquaredY;
	m13 = m.m31 * invScaleSquaredZ;
	m21 = m.m12 * invScaleSquaredX;
	m22 = m.m22 * invScaleSquaredY;
	m23 = m.m32 * invScaleSquaredZ;
	m31 = m.m13 * invScaleSquaredX;
	m32 = m.m23 * invScaleSquaredY;
	m33 = m.m33 * invScaleSquaredZ;

	// Back-transform translation
	m41 = -(m.m41 * m11 + m.m42 * m21 + m.m43 * m31);
	m42 = -(m.m41 * m12 + m.m42 * m22 + m.m43 * m32);
	m43 = -(m.m41 * m13 + m.m42 * m23 + m.m43 * m33);
}

void Matrix43::SetInverseFromUniformSRT(const Matrix43& m)
{
	assert(m.HasUniformScale());

	const float32 invScaleSquared = 1.f / (m.axisX.LengthSquared());

    // Transpose upper 3x3 and divide by scale
	m11 = m.m11 * invScaleSquared;
	m12 = m.m21 * invScaleSquared;
	m13 = m.m31 * invScaleSquared;
	m21 = m.m12 * invScaleSquared;
	m22 = m.m22 * invScaleSquared;
	m23 = m.m32 * invScaleSquared;
	m31 = m.m13 * invScaleSquared;
	m32 = m.m23 * invScaleSquared;
	m33 = m.m33 * invScaleSquared;

	// Back-transform translation
	m41 = -(m.m41 * m11 + m.m42 * m21 + m.m43 * m31);
	m42 = -(m.m41 * m12 + m.m42 * m22 + m.m43 * m32);
	m43 = -(m.m41 * m13 + m.m42 * m23 + m.m43 * m33);
}

void Matrix43::SetInverseFromRT(const Matrix43& m)
{
	assert(MathEx::AlmostEquals(m.GetUniformScale(), 1.f) && "Matrix must not contain scale");
    
	// Transpose upper 3x3
	m11 = m.m11;
	m12 = m.m21;
	m13 = m.m31;
	m21 = m.m12;
	m22 = m.m22;
	m23 = m.m32;
	m31 = m.m13;
	m32 = m.m23;
	m33 = m.m33;

	// Back-transform translation
	m41 = -(m.m41 * m11 + m.m42 * m21 + m.m43 * m31);
	m42 = -(m.m41 * m12 + m.m42 * m22 + m.m43 * m32);
	m43 = -(m.m41 * m13 + m.m42 * m23 + m.m43 * m33);
}

void Matrix43::SetInverseFromR(const Matrix43& m)
{
	assert(MathEx::AlmostEquals(m.GetUniformScale(), 1.f) && "Matrix must not contain scale");
	assert(m.trans.IsZero());

    // Transpose upper 3x3
	m11 = m.m11;
	m12 = m.m21;
	m13 = m.m31;
	m21 = m.m12;
	m22 = m.m22;
	m23 = m.m32;
	m31 = m.m13;
	m32 = m.m23;
	m33 = m.m33;

	trans.SetZero();
}

Matrix43 Matrix43::Mul(const Matrix43& rhs) const
{
	Matrix43 r;
	MulImpl(*this, rhs, r);
	return r;
}

Matrix43& Matrix43::MulAssign(const Matrix43& rhs)
{
	MulImpl(Matrix43(*this), rhs, *this);
	return *this;
}

std::string Matrix43::ToString() const
{
	std::string output;
	for (int row=0; row<4; ++row)
	{
		output += "|";
		for (int col=0; col<3; ++col)
		{
			output += str_format("%.2f ", MathEx::AdjustZero(m[row][col]));
		}
		output += "|\n";
	}
	return output;
}
