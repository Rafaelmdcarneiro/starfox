#ifndef _GS_EULER_ANGLES_H_
#define _GS_EULER_ANGLES_H_

#include "Angle.h"
class Quaternion;
class Matrix43;

class EulerAngles
{
public:
	Angle yaw;		// Angle around Y+ (up)
	Angle pitch;	// Angle around X+ (right)
	Angle roll;		// Angle around Z+ (forward)

	EulerAngles() {}
	
	EulerAngles(Angle yaw, Angle pitch, Angle roll)
		: yaw(yaw), pitch(pitch), roll(roll)
	{
	}

	static const EulerAngles& Identity() { static EulerAngles e(0.f, 0.f, 0.f); return e; }

	void Set(Angle yaw, Angle pitch, Angle roll) { this->yaw = yaw; this->pitch = pitch; this->roll = roll; }
	void SetZero() { *this = Zero(); }

	void SetFromQuaternion(const Quaternion& q);
	void SetFromMatrix(const Matrix43& m);

	static EulerAngles Zero() { static EulerAngles e(0,0,0); return e; }

	// Sets canonicle Euler triple: yaw and roll are wrapped to [-kPi,kPi] and pitch to [-kPi/2,kPi/2]
	// and gimbal lock is removed if possible.
	void Canonize();
};

#endif // _GS_EULER_ANGLES_H_
