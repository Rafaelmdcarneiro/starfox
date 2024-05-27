#ifndef _COLOR4_H_
#define _COLOR4_H_

#include "gs/Math/MathEx.h"

// Forward declaration for typedefs
template <typename T, int MIN, int MAX> struct Color4;

// Some useful typedefs
typedef Color4<float32, 0, 1> Color4F; // F = float32
typedef Color4<unsigned char, 0, 255> Color4UB; // UB = unsigned byte

// Represents an RGBA color value
// T is the underlying color component data type,
// MIN and MAX define the minimum and maximum value for the color components.
// Note: template constant parameters must be integral types, so unfortunately,
// you cannot specify a value of say 1.5f for T=float32.
template <typename T, int MIN, int MAX>
struct Color4
{
	enum { Min = MIN, Max = MAX };
	typedef T ComponentType;

	union
	{
		struct { T r, g, b, a; };
		T v[4];
	};

	// Parameterless constructor - does not initialize members on purpose for efficiency
	Color4() { }

	// Constructor that intializes color components
	Color4(T red, T green, T blue, T alpha=MAX) : r(red), g(green), b(blue), a(alpha) { }

	// Conversion constructor from other Color4 template types
	template <typename T2, int MIN2, int MAX2>
	Color4(const Color4<T2,MIN2,MAX2>& color4)
	{
		*this = color4;
	}

	// Conversion constructor from other Color4 template types (on assignment)
	template <typename T2, int MIN2, int MAX2>
	void operator=(const Color4<T2,MIN2,MAX2>& color4)
	{
		r = static_cast<T>( color4.r * Max / color4.Max );
		g = static_cast<T>( color4.g * Max / color4.Max );
		b = static_cast<T>( color4.b * Max / color4.Max );
	}

	// Sets color components
	void Set(T red, T green, T blue, T alpha=MAX)
	{
		r = red;  g = green;  b = blue;  a = alpha;
	}

	// Clamps values to [MIN,MAX]
	void Clamp()
	{
		r = (r < MIN? MIN : (r > MAX? MAX : r));
		g = (g < MIN? MIN : (g > MAX? MAX : g));
		b = (b < MIN? MIN : (b > MAX? MAX : b));
		a = (a < MIN? MIN : (a > MAX? MAX : a));
	}

	
	// Multiplication by a scalar constant
	void operator*=(float32 k) { r = MathEx::Scale(r, k); g = MathEx::Scale(g, k); b = MathEx::Scale(b, k); a = MathEx::Scale(a, k); }
	
	// Component-wise operations with another color
	void operator*=(const Color4& rhs)	{ r *= rhs.r; g *= rhs.g; b *= rhs.b; a *= rhs.a; }
	void operator+=(const Color4& rhs)	{ r += rhs.r; g += rhs.g; b += rhs.b; a += rhs.a; }

	// These are non-member (free standing) functions (defined here using 'friend' so we don't
	// have to supply all the template parameters)
	friend Color4 operator*(float32 k, const Color4& rhs) { return Color4(MathEx::Scale(rhs.r, k), MathEx::Scale(rhs.g, k), MathEx::Scale(rhs.b, k)); }
	friend Color4 operator*(const Color4& rhs, float32 k) { return (k*rhs); }

	#define MAKE_COLOR_OP(op) Color4(lhs.r op rhs.r, lhs.g op rhs.g, lhs.b op rhs.b, lhs.a op rhs.a)
	friend Color4 operator+(const Color4& lhs, const Color4& rhs) { return MAKE_COLOR_OP(+); }
	friend Color4 operator*(const Color4& lhs, const Color4& rhs) { return MAKE_COLOR_OP(*); }
	
	// Functions that return specific and often-used colors. Since they return const
	// refs, these can be used efficiently as is or assigned to Color4 objects, in
	// which case a copy will be made.
	static const Color4& Black()	{ static Color4 color(MIN, MIN, MIN); return color; }
	static const Color4& White()	{ static Color4 color(MAX, MAX, MAX); return color; }
	static const Color4& Red()		{ static Color4 color(MAX, MIN, MIN); return color;  }
	static const Color4& Green()	{ static Color4 color(MIN, MAX, MIN); return color;  }
	static const Color4& Blue()		{ static Color4 color(MIN, MIN, MAX); return color;  }
	static const Color4& Magenta()	{ static Color4 color(MAX, MIN, MAX); return color;  }
	static const Color4& Teal()		{ static Color4 color(MIN, MAX, MAX); return color;  }
	static const Color4& Yellow()	{ static Color4 color(MAX, MAX, MIN); return color;  }
};


#endif // _COLOR_H_
