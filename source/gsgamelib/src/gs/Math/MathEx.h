#ifndef _MATH_EX_H_
#define _MATH_EX_H_

// Contains all math-related stuff - extends the basic math lib

#include "gs/Base/Base.h"
#include <type_traits>
#include <cmath>
#include <cstdlib>

const float32 kPi = 3.141592654f;
const float32 k2Pi = 2.f * kPi;
const float32 kPiOver2 = kPi / 2.f;
const float32 kPiOver4 = kPi / 4.f;

const float32 kEpsilon = 1e-6f;

namespace MathEx
{
	template <typename T>
	inline T Clamp(T val, T min, T max)
	{
		if (val < min)
			return min;
		if (val > max)
			return max;
		return val;
	}

	template <typename T>
	inline T Max(T v1, T v2)
	{
		return v1 > v2? v1 : v2;
	}

	template <typename T>
	inline T Min(T v1, T v2)
	{
		return v1 < v2? v1 : v2;
	}

	// Allows scaling of different types (i.e. scaling an int by a float value)
	template <typename T, typename U>
	inline T Scale(T val, U scaleFactor)
	{
		return static_cast<T>((static_cast<U>(val) * scaleFactor));
	}

	// Converts degrees to radians
	template <typename T>
	inline T DegToRad(T val) { return (val * kPi) / static_cast<T>(180); }

	// Converts radians to degrees
	template <typename T>
	inline T RadToDeg(T val) { return (val * static_cast<T>(180)) / kPi; }

	// Performs lhs % rhs for floating point numbers (non-integer)
	template <typename T>
	T FMod(T lhs, T rhs) { return ::fmod(lhs, rhs); }

	// Compares two numbers, returning true if they are close within a constant-defined range, false otherwise
	template <typename T>
	bool AlmostEquals(T lhs, T rhs, T epsilon=kEpsilon) { return ( (lhs <= (rhs+epsilon)) && (lhs >= (rhs-epsilon)) ); }

	// Returns 0 if input value is approximately 0, otherwise returns the input value
	template <typename T>
	T AdjustZero(T val) { return (AlmostEquals<T>(val, 0)? 0 : val); }

	// Returns value to some exponent
	template <typename T>
	inline T Pow(T val, T exp) { return ::pow(val, exp); }

	// Returns value squared
	template <typename T>
	inline T Pow2(T val) { return val*val; }

	// Returns square root of value
	template <typename T>
	inline T Sqrt(T val) { return ::sqrt(val); }

	// Returns smallest integer that is >= to val
	template <typename T>
	inline T Ceil(T val) { return ::ceil(val); }

	// Returns largest integer that is <= to val
	template <typename T>
	inline T Floor(T val) { return ::floor(val); }

	// Returns absolute value of val
	template <typename T>
	inline T Abs(T val) { return ::abs(val); }

	// Returns sin and cosine of input angle (in radians)
	template <typename T>
	inline void SinCos(T rads, T& sinVal, T& cosVal)
	{
		sinVal = ::sin(rads);
		cosVal = ::cos(rads);
	}

	template <typename T> inline T Sin(T rads) { return ::sin(rads); }
	template <typename T> inline T Cos(T rads) { return ::cos(rads); }
	template <typename T> inline T Tan(T rads) { return ::tan(rads); }

	template <typename T> inline T ASin(T rads) { return ::asin(rads); }
	template <typename T> inline T ACos(T rads) { return ::acos(rads); }
	template <typename T> inline T ATan(T rads) { return ::atan(rads); }
	template <typename T> inline T ATan2(T y, T x) { return ::atan2(y, x); }


	// Clamps input to valid range so that acos(val) returns valid value
	// (return values are in range [0, kPi])
	template <typename T>
	T SafeACos(T val)
	{
		if (val <= (T)-1.0)
			return kPi;

		if (val >= (T)1.0)
			return (T)0.0;

		return acos(val); // Value is in valid range, forward to standard acos()
	}

	// Wraps value in periodic range [0, period] + phaseShift
	template <typename T>
	T WrapPeriodicValue(T val, T period, T phaseShift = (T)0)
	{
		val += phaseShift;
		T numPeriods = floor(val / period);
		val -= numPeriods * period;
		val -= phaseShift;
		return val;
	}

	template <typename T>
	inline T WrapPI(T rads)
	{
		return WrapPeriodicValue(rads, k2Pi, kPi);
	}

	template <typename T>
	inline T Wrap2PI(T rads)
	{
		return WrapPeriodicValue(rads, k2Pi);
	}

	template <typename T>
	inline T Wrap180(T degs)
	{
		return WrapPeriodicValue(degs, 360.0f, 180.0f);
	}

	// Returns true if input value is a power of two (2, 4, 8, 16, 32, etc.)
	template <typename T>
	bool IsPowerOfTwo(T val)
	{
		int v = static_cast<int>(val);
		return !(v & (v-1));
	}

	// Compile-time version of IsPowerOfTwo
	template <int Val>
	struct IsPowerOfTwoCT
	{
		enum { Value = !(Val&(Val-1)) }; //(CountBits<Val>::Value == 1) };
	};

	namespace Internal
	{
		template <typename T, bool IsFloat>
		struct RandImpl
		{
			static T DoIt(T min, T max)
			{
				float32 ret = rand()/(((T)RAND_MAX + 1) / (max-min));
				return ret + min;
			}
		};
		
		template <typename T>
		struct RandImpl<T, false>
		{
			static T DoIt(T min, T max) { return rand() % (max-min+1) + min; }
		};
	}

	// Returns a random value in the range [min, max]
	template <typename T>
	T Rand(T min, T max)
	{
		return Internal::RandImpl<T, std::is_floating_point<T>::value >::DoIt(min, max);
	}

	// Returns a random value in the range [0, max]
	template <typename T>
	T Rand(T max)
	{
		return Internal::RandImpl<T, std::is_floating_point<T>::value >::DoIt(0, max);
	}

	template <typename T>
	T Lerp(T a, T b, float32 t) { return a + (b-a)*t; }

	template <typename T>
	T Sign(T v, T resultIfZero = 0)
	{
		return (v > 0)? 1 : (v < 0)? -1 : resultIfZero;
	}

	// Returns mapped value in [minA,maxA] to [minB,maxB]
	template <typename T, typename U>
	U MapRangedValue(T value, T minA, T maxA, U minB, U maxB)
	{
		const float32 ratio = (static_cast<float32>(value) - minA) / (static_cast<float32>(maxA) - minA);
		return static_cast<U>( minB + ratio * (static_cast<float32>(maxB) - minB) );
	}

} // namespace MathEx

#endif // _MATH_EX_H_
