#include "gs/Math/Matrix43.h"
#include "gs/Math/Quaternion.h"
#include "gs/Math/Vector3.h"
#include "gs/Math/EulerAngles.h"
#include <cassert>

static Vector3 RandNormalizedVector3()
{
	return SafeNormalize( Vector3(MathEx::Rand(0.f, 1.f), MathEx::Rand(0.f, 1.f), MathEx::Rand(0.f, 1.f)), Vector3::UnitY() );
}

extern void UnitTest_Math()
{
	// Left-handed system with Z+ forward, Y+ up, and X+ right
	const Vector3& vForward = Vector3::UnitZ();
	const Vector3& vUp = Vector3::UnitY();
	const Vector3& vRight = Vector3::UnitX();

	const float32 maxScale = 5.f;

	Matrix43 m1, m2, m3, m4, m5;
	Matrix43 mInv1, mInv2, mInv3, mInv4;
	Vector3 v1, v2, v3, vAxis;
	Quaternion q1, q2, q3, q4, q5;
	Quaternion qInv1, qInv2, qInv3;
	Angle a1, a2, a3;
	
	// Matrix43::SetFromAxisAngle, SetFromAxisAngle
	{
		// Identity
		m1.SetFromAxisAngle(vUp, 0.f);
		assert(m1.AlmostEquals(Matrix43::Identity()));

		m1.SetFromAxisAngle(vForward, 0.f);
		assert(m1.AlmostEquals(Matrix43::Identity()));

		m1.SetFromAxisAngle(vRight, 0.f);
		assert(m1.AlmostEquals(Matrix43::Identity()));

		// Yaw
		m1.SetFromAxisAngle(vUp, Angle::FromDeg(90.0f));
		v1 = DirectionVector(vForward) * m1;
		assert(v1.AlmostEquals(vRight));

		m1.SetFromAxisAngle(vUp, Angle::FromDeg(-90.0f));
		v1 = DirectionVector(vForward) * m1;
		assert(v1.AlmostEquals(-vRight));

		m1.SetFromAxisAngle(vUp, Angle::FromDeg(180.0f));
		v1 = DirectionVector(vForward) * m1;
		assert(v1.AlmostEquals(-vForward));

		// Roll
		m1.SetFromAxisAngle(vForward, Angle::FromDeg(90.f));
		v1 = DirectionVector(vRight) * m1;
		assert(v1.AlmostEquals(vUp));

		m1.SetFromAxisAngle(vForward, Angle::FromDeg(90.f));
		v1 = DirectionVector(vUp) * m1;
		assert(v1.AlmostEquals(-vRight));

		// Pitch
		m1.SetFromAxisAngle(vRight, Angle::FromDeg(90.f));
		v1 = DirectionVector(vForward) * m1;
		assert(v1.AlmostEquals(-vUp));

		m1.SetFromAxisAngle(vRight, Angle::FromDeg(180));
		v1 = DirectionVector(vUp) * m1;
		assert(v1.AlmostEquals(-vUp));
	}

	// Matrix43::SetFromEulerAngles
	{
		// Identity
		m1.SetFromEulerAngles(EulerAngles::Identity());
		assert(m1.AlmostEquals(Matrix43::Identity()));

		m1.SetFromEulerAngles(EulerAngles(Angle::FromDeg(360.f), Angle::FromDeg(360.f * 2.f), Angle::FromDeg(-360.0f*5)));
		assert(m1.AlmostEquals(Matrix43::Identity()));

		for (uint32 i = 0; i < 100; ++i)
		{
			a1 = Angle::FromDeg( MathEx::Rand(0.f, 360.f) );
			m1.SetFromAxisAngle(vUp, a1);
			m2.SetFromEulerAngles(EulerAngles(a1, 0.f, 0.f));
			assert(m1.AlmostEquals(m2));

			a1 = Angle::FromDeg( MathEx::Rand(0.f, 360.f) );
			m1.SetFromAxisAngle(vRight, a1);
			m2.SetFromEulerAngles(EulerAngles(0.f, a1, 0.f));
			assert(m1.AlmostEquals(m2));

			a1 = Angle::FromDeg( MathEx::Rand(0.f, 360.f) );
			m1.SetFromAxisAngle(vForward, a1);
			m2.SetFromEulerAngles(EulerAngles(0.f, 0.f, a1));
			assert(m1.AlmostEquals(m2));
		}
	}

	// Matrix::SetFromQuaternion
	// Quaternion::SetFromAxis, SetFromEulerAngles, and SetFromMatrix
	{
		m1.SetFromQuaternion(Quaternion::Identity());
		assert(m1.AlmostEquals(Matrix43::Identity()));

		m1.SetFromQuaternion(-Quaternion::Identity());
		assert(m1.AlmostEquals(Matrix43::Identity()));

		for (uint32 i = 0; i < 100; ++i)
		{
			a1 = Angle::FromDeg( MathEx::Rand(0.f, 360.f) );
			m1.SetFromAxisAngle(vUp, a1);
			q2.SetFromAxisAngle(vUp, a1);
			m2.SetFromQuaternion(q2);
			assert(m1.AlmostEquals(m2));
			q2.SetFromEulerAngles(EulerAngles(a1, 0.f, 0.f));
			m2.SetFromQuaternion(q2);
			assert(m1.AlmostEquals(m2));
			q1.SetFromMatrix(m2);
			assert(q1.AlmostEquals(q2));

			a1 = Angle::FromDeg( MathEx::Rand(0.f, 360.f) );
			m1.SetFromAxisAngle(vRight, a1);
			q2.SetFromAxisAngle(vRight, a1);
			m2.SetFromQuaternion(q2);
			assert(m1.AlmostEquals(m2));
			q2.SetFromEulerAngles(EulerAngles(0.f, a1, 0.f));
			m2.SetFromQuaternion(q2);
			assert(m1.AlmostEquals(m2));
			q1.SetFromMatrix(m2);
			assert(q1.AlmostEquals(q2));

			a1 = Angle::FromDeg( MathEx::Rand(0.f, 360.f) );
			m1.SetFromAxisAngle(vForward, a1);
			q2.SetFromAxisAngle(vForward, a1);
			m2.SetFromQuaternion(q2);
			assert(m1.AlmostEquals(m2));
			q2.SetFromEulerAngles(EulerAngles(0.f, 0.f, a1));
			m2.SetFromQuaternion(q2);
			assert(m1.AlmostEquals(m2));
			q1.SetFromMatrix(m2);
			assert(q1.AlmostEquals(q2));
		}
	}

	// Concatenations
	// Matrix43::operator*
	// Quaternion::operator*
	{
		m1.SetFromTranslation(Vector3(1.f, 2.f, 3.f));
		m2.SetFromAxisAngle(vUp, kPiOver2);
		m3 = m2 * m1;
		assert(m3.trans.AlmostEquals(m1.trans));

		for (uint32 i = 0; i < 100; ++i)
		{
			// M(Angle) * M(-Angle) == Identity
			a1 = Angle::FromDeg( MathEx::Rand(0.f, 360.f) );
			vAxis = RandNormalizedVector3();
			v1 = RandNormalizedVector3();

			m1.SetFromAxisAngle(vAxis, a1);
			m1.trans = v1;
			m2.SetFromAxisAngle(vAxis, -a1);
			m2.trans = DirectionVector(-v1) * m2; // Reverse translation in rotated
			m3 = m1 * m2;
			assert(m3.AlmostEquals(Matrix43::Identity()));
			
			q1.SetFromAxisAngle(vAxis, a1);
			q2.SetFromAxisAngle(vAxis, -a1);
			q3 = q1 * q2;
			assert(q3.AlmostEquals(Quaternion::Identity()));

			// M(Angle) * M(Angle) = M(Angle + Angle)
			a1 = Angle::FromDeg( MathEx::Rand(0.f, 360.f) );
			a2 = Angle::FromDeg( MathEx::Rand(0.f, 360.f) );
			vAxis = RandNormalizedVector3();;

			m1.SetFromAxisAngle(vAxis, a1);
			m2.SetFromAxisAngle(vAxis, a2);
			m3 = m1 * m2;
			m4.SetFromAxisAngle(vAxis, a1 + a2);
			assert(m3.AlmostEquals(m4));

			q1.SetFromAxisAngle(vAxis, a1);
			q2.SetFromAxisAngle(vAxis, a2);
			q3 = q1 * q2;
			q4.SetFromAxisAngle(vAxis, a1 + a2);
			assert(q3.AlmostEquals(q4));
		}
	}

	// Inversions
	// Matrix43::InvertR, InvertRT
	// Quaternion::Invert
	{
		for (uint32 i = 0; i < 100; ++i)
		{
			// M(Angle) * M-1(Angle) == Identity
			a1 = Angle::FromDeg( MathEx::Rand(0.f, 360.f) );
			m1.SetFromAxisAngle(vAxis, a1);
			m2.SetInverseFromR(m1);
			m3 = m1 * m2;
			assert(m3.AlmostEquals(Matrix43::Identity()));

			// Quat
			q1.SetFromMatrix(m1);
			q2 = Invert(q1);
			q3 = q1 * q2;
			assert(q3.AlmostEquals(Quaternion::Identity()));

			// M1*M2*M3*M3-1*M2-1*M1-1 == Identity
			vAxis = RandNormalizedVector3();
			a1 = Angle::FromDeg( MathEx::Rand(0.f, 360.f) );
			m1.SetFromAxisAngle(vAxis, a1);
			m1.trans = RandNormalizedVector3();
		
			vAxis = RandNormalizedVector3();
			a1 = Angle::FromDeg( MathEx::Rand(0.f, 360.f) );
			m2.SetFromAxisAngle(vAxis, a1);
			m2.trans = RandNormalizedVector3();
		
			vAxis = RandNormalizedVector3();
			a1 = Angle::FromDeg( MathEx::Rand(0.f, 360.f) );
			m3.SetFromAxisAngle(vAxis, a1);
			m3.trans = RandNormalizedVector3();
		
			mInv1.SetInverseFromRT(m1);
			mInv2.SetInverseFromRT(m2);
			mInv3.SetInverseFromRT(m3);

			m4 = m1 * m2 * m3;
			m5 = m4 * mInv3 * mInv2 * mInv1;
			assert(m5.AlmostEquals(Matrix43::Identity(), 1e-4f));

			// Quat
			q1.SetFromMatrix(m1);
			q2.SetFromMatrix(m2);
			q3.SetFromMatrix(m3);
			qInv1 = Invert(q1);
			qInv2 = Invert(q2);
			qInv3 = Invert(q3);
			q4 = q1 * q2 * q3;
			q5 = q4 * qInv3 * qInv2 * qInv1;
			assert(q5.AlmostEquals(Quaternion::Identity()));
		}

		// Test variants of Matrix43 inverse
		for (int i = 0; i < 100; ++i)
		{
			// M1*M2*M3*M3-1*M2-1*M1-1 == Identity
			vAxis = RandNormalizedVector3();
			a1 = Angle::FromDeg( MathEx::Rand(0.f, 360.f) );
			m1.SetFromAxisAngle(vAxis, a1);
		
			vAxis = RandNormalizedVector3();
			a1 = Angle::FromDeg( MathEx::Rand(0.f, 360.f) );
			m2.SetFromAxisAngle(vAxis, a1);
		
			vAxis = RandNormalizedVector3();
			a1 = Angle::FromDeg( MathEx::Rand(0.f, 360.f) );
			m3.SetFromAxisAngle(vAxis, a1);
		
			// InvertR
			mInv1.SetInverseFromR(m1);
			mInv2.SetInverseFromR(m2);
			mInv3.SetInverseFromR(m3);
			m4 = m1 * m2 * m3;
			m5 = m4 * mInv3 * mInv2 * mInv1;
			assert(m5.AlmostEquals(Matrix43::Identity()));

			// InvertRT
			m1.trans = RandNormalizedVector3();
			m2.trans = RandNormalizedVector3();
			m3.trans = RandNormalizedVector3();
			mInv1.SetInverseFromRT(m1);
			mInv2.SetInverseFromRT(m2);
			mInv3.SetInverseFromRT(m3);
			m4 = m1 * m2 * m3;
			m5 = m4 * mInv3 * mInv2 * mInv1;
			m5.Orthogonalize();
			assert(m5.AlmostEquals(Matrix43::Identity(), 1e-4f));

			// InvertUniformSRT
			float32 s1 = MathEx::Rand(1.f, maxScale);
			m1.axisX *= s1;
			m1.axisY *= s1;
			m1.axisZ *= s1;
			float32 s2 = MathEx::Rand(1.f, maxScale);
			m2.axisX *= s2;
			m2.axisY *= s2;
			m2.axisZ *= s2;
			float32 s3 = MathEx::Rand(1.f, maxScale);
			m3.axisX *= s3;
			m3.axisY *= s3;
			m3.axisZ *= s3;
			mInv1.SetInverseFromUniformSRT(m1);
			mInv2.SetInverseFromUniformSRT(m2);
			mInv3.SetInverseFromUniformSRT(m3);
			m4 = m1 * m2 * m3;
			m5 = m4 * mInv3 * mInv2 * mInv1;
			assert(m5.AlmostEquals(Matrix43::Identity(), 1e-4f));

			// InvertSRT
			m1.axisX *= MathEx::Rand(1.f, maxScale);
			m1.axisY *= MathEx::Rand(1.f, maxScale);
			m1.axisZ *= MathEx::Rand(1.f, maxScale);
			m2.axisX *= MathEx::Rand(1.f, maxScale);
			m2.axisY *= MathEx::Rand(1.f, maxScale);
			m2.axisZ *= MathEx::Rand(1.f, maxScale);
			m3.axisX *= MathEx::Rand(1.f, maxScale);
			m3.axisY *= MathEx::Rand(1.f, maxScale);
			m3.axisZ *= MathEx::Rand(1.f, maxScale);
			mInv1.SetInverseFromSRT(m1);
			mInv2.SetInverseFromSRT(m2);
			mInv3.SetInverseFromSRT(m3);
			m4 = m1 * m2 * m3;
			m5 = m4 * mInv3 * mInv2 * mInv1;
			assert(m5.AlmostEquals(Matrix43::Identity(), 1e-4f));

			// Invert
			Matrix43 mShear = Matrix43::Identity();
			mShear.axisX.y = MathEx::Rand(1.f, maxScale);
			mShear.axisY.z = MathEx::Rand(1.f, maxScale);
			mShear.axisZ.x = MathEx::Rand(1.f, maxScale);
			m1 = m1 * mShear;
			m2 = m2 * mShear;
			m3 = m3 * mShear;
			mInv1.SetInverseFrom(m1);
			mInv2.SetInverseFrom(m2);
			mInv3.SetInverseFrom(m3);
			m4 = m1 * m2 * m3;
			m5 = m4 * mInv3 * mInv2 * mInv1;
			assert(m5.AlmostEquals(Matrix43::Identity(), 1e-3f));
		}
	}

	// Quat Slerp
	//@TODO: more complete Slerp tests
	{
		q1.SetIdentity();
		q2.SetFromAxisAngle(vUp, Angle::FromDeg(90.f));
		
		q3 = Slerp(q1, q2, 0.f);
		assert(q3.AlmostEquals(q1));

		q3 = Slerp(q1, q2, 1.f);
		assert(q3.AlmostEquals(q2));

		q3 = Slerp(q1, q2, 0.5f);
		q3.ToAxisAngle(vAxis, a1);
		assert(vAxis.AlmostEquals(vUp));
		assert(MathEx::AlmostEquals(a1.ToDeg(), 45.f, 1e-4f));

		q2.SetFromAxisAngle(vUp, Angle::FromDeg(-90.f));
		q3 = Slerp(q1, q2, 0.5f);
		q3.ToAxisAngle(vAxis, a1);
		assert(vAxis.AlmostEquals(vUp) && MathEx::AlmostEquals(a1.ToDeg(), -45.f, 1e-4f)
			|| vAxis.AlmostEquals(-vUp) && MathEx::AlmostEquals(a1.ToDeg(), 45.f, 1e-4f));
	}

	// Vector
	{
		// Cross-product
		v1 = Vector3::UnitX().Cross(Vector3::UnitY());
		assert( v1.AlmostEquals(Vector3::UnitZ()) );
	}
}
