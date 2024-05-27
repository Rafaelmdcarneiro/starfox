#include "Vector3.h"
#include "Vector4.h"
#include "gs/Base/string_helpers.h"

Vector3::Vector3(const Vector4& v4)
	: x(v4.x), y(v4.y), z(v4.z)
{
}

std::string Vector3::ToString() const
{
	return str_format("|%f, %f, %f|", MathEx::AdjustZero(x), MathEx::AdjustZero(y), MathEx::AdjustZero(z));
}
