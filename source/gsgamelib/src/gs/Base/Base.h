#ifndef _BASE_H_
#define _BASE_H_

#include <limits>
#include <memory>

typedef char				int8;
typedef short				int16;
typedef int					int32;
typedef long long			int64;

typedef unsigned char		uint8;
typedef unsigned short		uint16;
typedef unsigned int		uint32;
typedef unsigned long long	uint64;

typedef float				float32;
typedef double				float64;

typedef unsigned char		byte;

static_assert(sizeof(int8)==1, "Wrong type on this platform");
static_assert(sizeof(int16)==2, "Wrong type on this platform");
static_assert(sizeof(int32)==4, "Wrong type on this platform");
static_assert(sizeof(int64)==8, "Wrong type on this platform");
static_assert(sizeof(uint8)==1, "Wrong type on this platform");
static_assert(sizeof(uint16)==2, "Wrong type on this platform");
static_assert(sizeof(uint32)==4, "Wrong type on this platform");
static_assert(sizeof(uint64)==8, "Wrong type on this platform");
static_assert(sizeof(float32)==4, "Wrong type on this platform");
static_assert(sizeof(float64)==8, "Wrong type on this platform");

#define NUM_MAX(variable) (std::numeric_limits< decltype(variable) >::max())
#define NUM_MIN(variable) (std::numeric_limits< decltype(variable) >::min())


// Validates cast in debug
template <class To, class From>
To safe_static_cast(From& v)
{
#if defined (_DEBUG) && defined(_CPPRTTI)
	return dynamic_cast<To>(v);
#else
	return static_cast<To>(v);
#endif
}

template <class To, class From>
To safe_static_cast(const From& v)
{
	return safe_static_cast<To>( const_cast<From&>(v) );
}

// More efficient but non-portable way to return whether pw.lock() == ps
template <typename T>
inline bool is_weak_to_shared_ptr(const std::weak_ptr<T>& pw, const std::shared_ptr<T>& ps)
{
#ifdef _MSC_VER
	return pw._Get() != nullptr && pw._Get() == ps._Get();
#else
	#error "Implement for this platform"
#endif
}

// Returns number of elements of C-style array
#define ARRAY_SIZE(array) (sizeof(array)/sizeof(array[0]))

// Use to flag unused variables (shuts up compiler).
// This will work for undefined types as well (i.e. forward declared types)
#define UNUSED_VAR(var) { { const void* p = (const void*)&var; (void)p; } }

#ifdef _DEBUG
#define TWEAKABLE static
#else
#define TWEAKABLE const
#endif

#define FORCE_INLINE __forceinline

// Disable warnings
#ifdef _MSC_VER

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#pragma warning(disable : 4201) // nonstandard extension used : nameless struct/union
#pragma warning(disable : 4100) // unreferenced formal parameter

#endif // _MSC_VER

#endif // _BASE_H_
