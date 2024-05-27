#include "Vector4.h"
#include "gs/Base/string_helpers.h"
#include "Vector3.h"

Vector4::Vector4(const Vector3& v3, float32 w)
	: x(v3.x), y(v3.y), z(v3.z), w(w)
{
}

std::string Vector4::ToString() const
{
	return str_format("[%f, %f, %f, %f]", MathEx::AdjustZero(x), MathEx::AdjustZero(y), MathEx::AdjustZero(z), MathEx::AdjustZero(w));
}
