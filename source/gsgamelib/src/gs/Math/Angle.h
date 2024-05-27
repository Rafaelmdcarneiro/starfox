#ifndef _GS_ANGLE_H_
#define _GS_ANGLE_H_

#include "gs/Base/Base.h"
#include "MathEx.h"

class Angle
{
public:
	float32 rads;

	Angle() {}
	Angle(float32 rads) : rads(rads)	{}
	
	// Implicit conversion so we can treat Angle like a float32
	operator float32&()					{ return rads; }
	operator const float32&() const		{ return rads; }

	static Angle FromDeg(float32 degs)	{ return MathEx::DegToRad(degs); }

	void SetFromDeg(float32 degs)		{ rads = MathEx::DegToRad(degs); }
	float32 ToDeg() const				{ return MathEx::RadToDeg(rads); }
	
	void NormalizePI()					{ rads = MathEx::WrapPI(rads); }
	void Normalize2PI()					{ rads = MathEx::Wrap2PI(rads); }
};

#endif // _GS_ANGLE_H_
