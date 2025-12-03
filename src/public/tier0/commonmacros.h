//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#ifndef COMMONMACROS_H
#define COMMONMACROS_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"

// -------------------------------------------------------
//
// commonmacros.h
//
// This should contain ONLY general purpose macros that are 
// appropriate for use in engine/launcher/all tools
//
// -------------------------------------------------------

// Makes a 4-byte "packed ID" int out of 4 characters
#define MAKEID(d,c,b,a)					( ((int)(a) << 24) | ((int)(b) << 16) | ((int)(c) << 8) | ((int)(d)) )

// Compares a string with a 4-byte packed ID constant
#define STRING_MATCHES_ID( p, id )		( (*((int *)(p)) == (id) ) ? true : false )
#define ID_TO_STRING( id, p )			( (p)[3] = (((id)>>24) & 0xFF), (p)[2] = (((id)>>16) & 0xFF), (p)[1] = (((id)>>8) & 0xFF), (p)[0] = (((id)>>0) & 0xFF) )

#define SETBITS(iBitVector, bits)		((iBitVector) |= (bits))
#define CLEARBITS(iBitVector, bits)		((iBitVector) &= ~(bits))
#define FBitSet(iBitVector, bits)		((iBitVector) & (bits))

template <typename T>
inline bool IsPowerOfTwo( T value )
{
	return (value & ( value - (T)1 )) == (T)0;
}

// dimhotepus: ssize support.
#ifdef __cpp_lib_ssize
// C++20 ssize
using std::ssize;
#else
#include <type_traits>

template <class C>
constexpr auto ssize(const C& c) noexcept(noexcept(c.size()))
    -> std::common_type_t<std::ptrdiff_t,
                          std::make_signed_t<decltype(c.size())>> {
  using R = std::common_type_t<std::ptrdiff_t,
                               std::make_signed_t<decltype(c.size())>>;
  return static_cast<R>(c.size());
}

template <class T, std::ptrdiff_t N>
constexpr std::ptrdiff_t ssize(const T (&)[N]) noexcept {
  return N;
}
#endif

#ifndef REFERENCE
#define REFERENCE(arg) ((void)arg)
#endif

#define CONST_INTEGER_AS_STRING(x) #x //Wraps the integer in quotes, allowing us to form constant strings with it
#define __HACK_LINE_AS_STRING__(x) CONST_INTEGER_AS_STRING(x) //__LINE__ can only be converted to an actual number by going through this, otherwise the output is literally "__LINE__"
#define __LINE__AS_STRING __HACK_LINE_AS_STRING__(__LINE__) //Gives you the line number in constant string form

// Using ARRAYSIZE implementation from winnt.h:
#ifdef ARRAYSIZE
#undef ARRAYSIZE
#endif

// Return the number of elements in a statically sized array.
//   DWORD Buffer[100];
//   RTL_NUMBER_OF(Buffer) == 100
// This is also popularly known as: NUMBER_OF, ARRSIZE, _countof, NELEM, etc.
//
#define RTL_NUMBER_OF_V1(A) (sizeof(A)/sizeof((A)[0]))

#if defined(__cplusplus) && \
    !defined(MIDL_PASS) && \
    !defined(RC_INVOKED) && \
    (_MSC_FULL_VER >= 13009466) && \
    !defined(SORTPP_PASS)

// From crtdefs.h
#if !defined(UNALIGNED)
#if defined(_M_IA64) || defined(_M_AMD64)
#define UNALIGNED __unaligned
#else
#define UNALIGNED
#endif
#endif

// RtlpNumberOf is a function that takes a reference to an array of N Ts.
//
// typedef T array_of_T[N];
// typedef array_of_T &reference_to_array_of_T;
//
// RtlpNumberOf returns a pointer to an array of N chars.
// We could return a reference instead of a pointer but older compilers do not accept that.
//
// typedef char array_of_char[N];
// typedef array_of_char *pointer_to_array_of_char;
//
// sizeof(array_of_char) == N
// sizeof(*pointer_to_array_of_char) == N
//
// pointer_to_array_of_char RtlpNumberOf(reference_to_array_of_T);
//
// We never even call RtlpNumberOf, we just take the size of dereferencing its return type.
// We do not even implement RtlpNumberOf, we just decare it.
//
// Attempts to pass pointers instead of arrays to this macro result in compile time errors.
// That is the point.
extern "C++" // templates cannot be declared to have 'C' linkage
template <typename T, size_t N>
char (*RtlpNumberOf( UNALIGNED T (&)[N] ))[N];

#ifdef _PREFAST_
// The +0 is so that we can go:
// size = ARRAYSIZE(array) * sizeof(array[0]) without triggering a /analyze
// warning about multiplying sizeof.
#define RTL_NUMBER_OF_V2(A) (sizeof(*RtlpNumberOf(A))+0)
#else
#define RTL_NUMBER_OF_V2(A) (sizeof(*RtlpNumberOf(A)))
#endif

// This does not work with:
//
// void Foo()
// {
//    struct { int x; } y[2];
//    RTL_NUMBER_OF_V2(y); // illegal use of anonymous local type in template instantiation
// }
//
// You must instead do:
//
// struct Foo1 { int x; };
//
// void Foo()
// {
//    Foo1 y[2];
//    RTL_NUMBER_OF_V2(y); // ok
// }
//
// OR
//
// void Foo()
// {
//    struct { int x; } y[2];
//    RTL_NUMBER_OF_V1(y); // ok
// }
//
// OR
//
// void Foo()
// {
//    struct { int x; } y[2];
//    _ARRAYSIZE(y); // ok
// }

#else
#define RTL_NUMBER_OF_V2(A) RTL_NUMBER_OF_V1(A)
#endif

// ARRAYSIZE is more readable version of RTL_NUMBER_OF_V2
// _ARRAYSIZE is a version useful for anonymous types
#define ARRAYSIZE(A)    RTL_NUMBER_OF_V2(A)
#define _ARRAYSIZE(A)   RTL_NUMBER_OF_V1(A)

#define Q_ARRAYSIZE(p)		ARRAYSIZE(p)
#define V_ARRAYSIZE(p)		ARRAYSIZE(p)

template< typename IndexType, typename T, unsigned int N >
IndexType ClampedArrayIndex( const T (&buffer)[N], IndexType index )
{
	NOTE_UNUSED( buffer );
	return clamp( index, 0, (IndexType)N - 1 );
}

template< typename T, unsigned int N >
T ClampedArrayElement( const T (&buffer)[N], unsigned int uIndex )
{
	// Put index in an unsigned type to halve the clamping.
	if ( uIndex >= N )
		uIndex = N - 1;
	return buffer[ uIndex ];
}

// MSVC specific.
#ifdef COMPILER_MSVC
/*
 * @brief Begins MSVC warning override scope.
 */
#define MSVC_BEGIN_WARNING_OVERRIDE_SCOPE() __pragma(warning(push))

/*
 * @brief Disables MSVC warning.
 */
#define MSVC_DISABLE_WARNING(warning_level) \
  __pragma(warning(disable : warning_level))

/*
 * @brief Ends MSVC warning override scope.
 */
#define MSVC_END_WARNING_OVERRIDE_SCOPE() __pragma(warning(pop))

/*
 * @brief Disable MSVC warning for code.
 */
#define MSVC_SCOPED_DISABLE_WARNING(warning_level, code) \
  MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()                    \
  MSVC_DISABLE_WARNING(warning_level)                    \
  code MSVC_END_WARNING_OVERRIDE_SCOPE()
#endif

#if defined(__clang__) || defined(__GCC__)
/*
 * @brief Begins GCC / Clang warning override scope.
 */
#define SRC_GCC_BEGIN_WARNING_OVERRIDE_SCOPE() _Pragma("GCC diagnostic push")

/*
 * @brief Disables GCC / Clang cast-function-type-mismatch.
 */
#define SRC_GCC_DISABLE_CAST_FUNCTION_TYPE_MISMATCH_WARNING() \
  _Pragma("GCC diagnostic ignored \"-Wcast-function-type-mismatch\"")

/*
 * @brief Disables GCC / Clang overloaded-virtual.
 */
#define SRC_GCC_DISABLE_OVERLOADED_VIRTUAL_WARNING() \
  _Pragma("GCC diagnostic ignored \"-Woverloaded-virtual\"")

/*
 * @brief Disables GCC / Clang switch warning.
 */
#define SRC_GCC_DISABLE_SWITCH_WARNING() \
  _Pragma("GCC diagnostic ignored \"-Wswitch\"")

/*
 * @brief Ends GCC / Clang warning override scope.
 */
#define SRC_GCC_END_WARNING_OVERRIDE_SCOPE() _Pragma("GCC diagnostic pop")
#else
/*
 * @brief Do nothing.
 */
#define SRC_GCC_BEGIN_WARNING_OVERRIDE_SCOPE()

/*
 * @brief Do nothing.
 */
#define SRC_GCC_DISABLE_CAST_FUNCTION_TYPE_MISMATCH_WARNING()

/*
 * @brief Do nothing.
 */
#define SRC_GCC_DISABLE_OVERLOADED_VIRTUAL_WARNING()

/*
 * @brief Do nothing.
 */
#define SRC_GCC_DISABLE_SWITCH_WARNING()

/*
 * @brief Do nothing.
 */
#define SRC_GCC_END_WARNING_OVERRIDE_SCOPE()
#endif

#endif // COMMONMACROS_H
