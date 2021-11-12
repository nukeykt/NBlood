/*
   Copyright (C) 2006, 2007 Sony Computer Entertainment Inc.
   All rights reserved.

   Redistribution and use in source and binary forms,
   with or without modification, are permitted provided that the
   following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the Sony Computer Entertainment Inc nor the names
      of its contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
*/

// ================================================================================================
// -*- C++ -*-
// File: vectormath/vectormath.hpp
// Author: Guilherme R. Lampert
// Created on: 30/12/16
// Brief: This header exposes the Sony Vectormath library types and functions into the global scope.
// ================================================================================================

#ifndef VECTORMATH_HPP
#define VECTORMATH_HPP

#if (!defined(VECTORMATH_DEBUG) && (defined(DEBUG) || defined(_DEBUG)))
    #define VECTORMATH_DEBUG 1
#endif // DEBUG || _DEBUG

// Detecting the availability of SSE at compile-time is a bit more involving with Visual Studio...
#ifdef _MSC_VER
    #if (defined(__AVX__) || defined(__AVX2__) || defined(_M_AMD64) || defined(_M_X64) || (_M_IX86_FP == 1) || (_M_IX86_FP == 2))
        #define VECTORMATH_CPU_HAS_SSE1_OR_BETTER 1
    #else // SSE support
        #define VECTORMATH_CPU_HAS_SSE1_OR_BETTER 0
    #endif // SSE support
#else // !_MSC_VER
    #if defined(__SSE__)
        #define VECTORMATH_CPU_HAS_SSE1_OR_BETTER 1
    #else // !__SSE__
        #define VECTORMATH_CPU_HAS_SSE1_OR_BETTER 0
    #endif // __SSE__
#endif // _MSC_VER

// Sony's library includes:
#if (VECTORMATH_CPU_HAS_SSE1_OR_BETTER && !VECTORMATH_FORCE_SCALAR_MODE)

#ifndef VECTORMATH_SSE_VECTORMATH_HPP
#define VECTORMATH_SSE_VECTORMATH_HPP

#include <cmath>
#include <xmmintrin.h>
#include <emmintrin.h>

#ifdef VECTORMATH_DEBUG
    #include <cstdio>
#endif // VECTORMATH_DEBUG

#if defined(_MSC_VER)
    // Visual Studio (MS compiler)
    #define VECTORMATH_ALIGNED(type)      __declspec(align(16)) type
    #define VECTORMATH_ALIGNED_TYPE_PRE   __declspec(align(16))
    #define VECTORMATH_ALIGNED_TYPE_POST  /* nothing */
#elif defined(__GNUC__)
    // GCC or Clang
    #define VECTORMATH_ALIGNED(type)      type __attribute__((aligned(16)))
    #define VECTORMATH_ALIGNED_TYPE_PRE   /* nothing */
    #define VECTORMATH_ALIGNED_TYPE_POST  __attribute__((aligned(16)))
#else
    // Unknown compiler
    #error "Define VECTORMATH_ALIGNED for your compiler or platform!"
#endif

#ifndef VECTORMATH_SSE_INTERNAL_HPP
#define VECTORMATH_SSE_INTERNAL_HPP

namespace Vectormath
{
namespace SSE
{

// ========================================================
// Helper constants
// ========================================================

// Small epsilon value
static const float VECTORMATH_SLERP_TOL = 0.999f;

// Common constants used to evaluate sseSinf/cosf4/tanf4
static const float VECTORMATH_SINCOS_CC0 = -0.0013602249f;
static const float VECTORMATH_SINCOS_CC1 =  0.0416566950f;
static const float VECTORMATH_SINCOS_CC2 = -0.4999990225f;
static const float VECTORMATH_SINCOS_SC0 = -0.0001950727f;
static const float VECTORMATH_SINCOS_SC1 =  0.0083320758f;
static const float VECTORMATH_SINCOS_SC2 = -0.1666665247f;
static const float VECTORMATH_SINCOS_KC1 =  1.57079625129f;
static const float VECTORMATH_SINCOS_KC2 =  7.54978995489e-8f;

// Shorthand functions to get the unit vectors as __m128
static inline __m128 sseUnitVec1000() { return _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f); }
static inline __m128 sseUnitVec0100() { return _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f); }
static inline __m128 sseUnitVec0010() { return _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f); }
static inline __m128 sseUnitVec0001() { return _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f); }

// ========================================================
// Internal helper types and functions
// ========================================================

typedef __m128 SSEFloat4V;
typedef __m128 SSEUint4V;
typedef __m128 SSEInt4V;

union SSEFloat
{
    __m128 m128;
    float f[4];
};

// These have to be macros because _MM_SHUFFLE() requires compile-time constants.
#define sseRor(vec, i)       (((i) % 4) ? (_mm_shuffle_ps(vec, vec, _MM_SHUFFLE((unsigned char)(i + 3) % 4, (unsigned char)(i + 2) % 4, (unsigned char)(i + 1) % 4, (unsigned char)(i + 0) % 4))) : (vec))
#define sseSplat(x, e)       _mm_shuffle_ps(x, x, _MM_SHUFFLE(e, e, e, e))
#define sseSld(vec, vec2, x) sseRor(vec, ((x) / 4))

static inline __m128 sseUintToM128(unsigned int x)
{
    union
    {
        unsigned int u;
        float f;
    } tmp;

    tmp.u = x;
    return _mm_set1_ps(tmp.f);
}

static inline __m128 sseMAdd(__m128 a, __m128 b, __m128 c)
{
    return _mm_add_ps(c, _mm_mul_ps(a, b));
}

static inline __m128 sseMSub(__m128 a, __m128 b, __m128 c)
{
    return _mm_sub_ps(c, _mm_mul_ps(a, b));
}

static inline __m128 sseMergeH(__m128 a, __m128 b)
{
    return _mm_unpacklo_ps(a, b);
}

static inline __m128 sseMergeL(__m128 a, __m128 b)
{
    return _mm_unpackhi_ps(a, b);
}

static inline __m128 sseSelect(__m128 a, __m128 b, __m128 mask)
{
    return _mm_or_ps(_mm_and_ps(mask, b), _mm_andnot_ps(mask, a));
}

static inline __m128 sseSelect(__m128 a, __m128 b, const unsigned int * mask)
{
    return sseSelect(a, b, _mm_load_ps((const float *)mask));
}

static inline __m128 sseSelect(__m128 a, __m128 b, unsigned int mask)
{
    return sseSelect(a, b, _mm_set1_ps(*(float *)&mask));
}

static inline SSEInt4V sseCvtToSignedInts(SSEFloat4V x)
{
    // Only 2^0 supported
    __m128i result = _mm_cvtps_epi32(x);
    return (__m128 &)result;
}

static inline SSEFloat4V sseCvtToFloats(SSEInt4V x)
{
    // Only 2^0 supported
    return _mm_cvtepi32_ps((__m128i &)x);
}

static inline __m128 sseSqrtf(__m128 x)
{
    return _mm_sqrt_ps(x);
}

static inline __m128 sseRSqrtf(__m128 x)
{
    return _mm_rsqrt_ps(x);
}

static inline __m128 sseRecipf(__m128 x)
{
    return _mm_rcp_ps(x);
}

static inline __m128 sseNegatef(__m128 x)
{
    return _mm_sub_ps(_mm_setzero_ps(), x);
}

static inline __m128 sseFabsf(__m128 x)
{
    return _mm_and_ps(x, sseUintToM128(0x7FFFFFFF));
}

static inline __m128 sseNewtonrapsonRSqrtf(__m128 x)
{
    const __m128 halfs  = _mm_setr_ps(0.5f, 0.5f, 0.5f, 0.5f);
    const __m128 threes = _mm_setr_ps(3.0f, 3.0f, 3.0f, 3.0f);
    const __m128 approx = _mm_rsqrt_ps(x);
    const __m128 muls   = _mm_mul_ps(_mm_mul_ps(x, approx), approx);
    return _mm_mul_ps(_mm_mul_ps(halfs, approx), _mm_sub_ps(threes, muls));
}

static inline __m128 sseACosf(__m128 x)
{
    const __m128 xabs = sseFabsf(x);
    const __m128 select = _mm_cmplt_ps(x, _mm_setzero_ps());
    const __m128 t1 = sseSqrtf(_mm_sub_ps(_mm_set1_ps(1.0f), xabs));

    /* Instruction counts can be reduced if the polynomial was
     * computed entirely from nested (dependent) fma's. However,
     * to reduce the number of pipeline stalls, the polygon is evaluated
     * in two halves (hi and lo).
     */
    const __m128 xabs2 = _mm_mul_ps(xabs, xabs);
    const __m128 xabs4 = _mm_mul_ps(xabs2, xabs2);

    const __m128 hi = sseMAdd(sseMAdd(sseMAdd(_mm_set1_ps(-0.0012624911f),
                                                 xabs, _mm_set1_ps(0.0066700901f)),
                                        xabs, _mm_set1_ps(-0.0170881256f)),
                               xabs, _mm_set1_ps(0.0308918810f));

    const __m128 lo = sseMAdd(sseMAdd(sseMAdd(_mm_set1_ps(-0.0501743046f),
                                                 xabs, _mm_set1_ps(0.0889789874f)),
                                        xabs, _mm_set1_ps(-0.2145988016f)),
                               xabs, _mm_set1_ps(1.5707963050f));

    const __m128 result = sseMAdd(hi, xabs4, lo);

    // Adjust the result if x is negative.
    return sseSelect(_mm_mul_ps(t1, result),                             // Positive
                     sseMSub(t1, result, _mm_set1_ps(3.1415926535898f)), // Negative
                     select);
}

static inline __m128 sseSinf(SSEFloat4V x)
{
    SSEFloat4V xl, xl2, xl3, res;

    // Range reduction using : xl = angle * TwoOverPi;
    xl = _mm_mul_ps(x, _mm_set1_ps(0.63661977236f));

    // Find the quadrant the angle falls in
    // using:  q = (int) (ceil(abs(xl))*sign(xl))
    SSEInt4V q = sseCvtToSignedInts(xl);

    // Compute an offset based on the quadrant that the angle falls in
    SSEInt4V offset = _mm_and_ps(q, sseUintToM128(0x3));

    // Remainder in range [-pi/4..pi/4]
    SSEFloat4V qf = sseCvtToFloats(q);
    xl = sseMSub(qf, _mm_set1_ps(VECTORMATH_SINCOS_KC2), sseMSub(qf, _mm_set1_ps(VECTORMATH_SINCOS_KC1), x));

    // Compute x^2 and x^3
    xl2 = _mm_mul_ps(xl, xl);
    xl3 = _mm_mul_ps(xl2, xl);

    // Compute both the sin and cos of the angles
    // using a polynomial expression:
    //   cx = 1.0f + xl2 * ((C0 * xl2 + C1) * xl2 + C2), and
    //   sx = xl + xl3 * ((S0 * xl2 + S1) * xl2 + S2)
    const SSEFloat4V cx =
        sseMAdd(
        sseMAdd(
        sseMAdd(_mm_set1_ps(VECTORMATH_SINCOS_CC0), xl2, _mm_set1_ps(VECTORMATH_SINCOS_CC1)), xl2, _mm_set1_ps(VECTORMATH_SINCOS_CC2)),
        xl2, _mm_set1_ps(1.0f));
    const SSEFloat4V sx =
        sseMAdd(
        sseMAdd(
        sseMAdd(_mm_set1_ps(VECTORMATH_SINCOS_SC0), xl2, _mm_set1_ps(VECTORMATH_SINCOS_SC1)), xl2, _mm_set1_ps(VECTORMATH_SINCOS_SC2)),
        xl3, xl);

    // Use the cosine when the offset is odd and the sin
    // when the offset is even
    res = sseSelect(cx, sx, _mm_cmpeq_ps(_mm_and_ps(offset, sseUintToM128(0x1)), _mm_setzero_ps()));

    // Flip the sign of the result when (offset mod 4) = 1 or 2
    return sseSelect(_mm_xor_ps(sseUintToM128(0x80000000U), res), // Negative
                     res,                                         // Positive
                     _mm_cmpeq_ps(_mm_and_ps(offset, sseUintToM128(0x2)), _mm_setzero_ps()));
}

static inline void sseSinfCosf(SSEFloat4V x, SSEFloat4V * s, SSEFloat4V * c)
{
    SSEFloat4V xl, xl2, xl3;
    SSEInt4V offsetSin, offsetCos;

    // Range reduction using : xl = angle * TwoOverPi;
    xl = _mm_mul_ps(x, _mm_set1_ps(0.63661977236f));

    // Find the quadrant the angle falls in
    // using:  q = (int) (ceil(abs(xl))*sign(xl))
    SSEInt4V q = sseCvtToSignedInts(xl);

    // Compute the offset based on the quadrant that the angle falls in.
    // Add 1 to the offset for the cosine.
    offsetSin = _mm_and_ps(q, sseUintToM128((int)0x3));
    __m128i temp = _mm_add_epi32(_mm_set1_epi32(1), (__m128i &)offsetSin);
    offsetCos = (__m128 &)temp;

    // Remainder in range [-pi/4..pi/4]
    SSEFloat4V qf = sseCvtToFloats(q);
    xl = sseMSub(qf, _mm_set1_ps(VECTORMATH_SINCOS_KC2), sseMSub(qf, _mm_set1_ps(VECTORMATH_SINCOS_KC1), x));

    // Compute x^2 and x^3
    xl2 = _mm_mul_ps(xl, xl);
    xl3 = _mm_mul_ps(xl2, xl);

    // Compute both the sin and cos of the angles
    // using a polynomial expression:
    //   cx = 1.0f + xl2 * ((C0 * xl2 + C1) * xl2 + C2), and
    //   sx = xl + xl3 * ((S0 * xl2 + S1) * xl2 + S2)
    const SSEFloat4V cx =
        sseMAdd(
        sseMAdd(
        sseMAdd(_mm_set1_ps(VECTORMATH_SINCOS_CC0), xl2, _mm_set1_ps(VECTORMATH_SINCOS_CC1)), xl2, _mm_set1_ps(VECTORMATH_SINCOS_CC2)),
        xl2, _mm_set1_ps(1.0f));
    const SSEFloat4V sx =
        sseMAdd(
        sseMAdd(
        sseMAdd(_mm_set1_ps(VECTORMATH_SINCOS_SC0), xl2, _mm_set1_ps(VECTORMATH_SINCOS_SC1)), xl2, _mm_set1_ps(VECTORMATH_SINCOS_SC2)),
        xl3, xl);

    // Use the cosine when the offset is odd and the sin
    // when the offset is even
    SSEUint4V sinMask = (SSEUint4V)_mm_cmpeq_ps(_mm_and_ps(offsetSin, sseUintToM128(0x1)), _mm_setzero_ps());
    SSEUint4V cosMask = (SSEUint4V)_mm_cmpeq_ps(_mm_and_ps(offsetCos, sseUintToM128(0x1)), _mm_setzero_ps());
    *s = sseSelect(cx, sx, sinMask);
    *c = sseSelect(cx, sx, cosMask);

    // Flip the sign of the result when (offset mod 4) = 1 or 2
    sinMask = _mm_cmpeq_ps(_mm_and_ps(offsetSin, sseUintToM128(0x2)), _mm_setzero_ps());
    cosMask = _mm_cmpeq_ps(_mm_and_ps(offsetCos, sseUintToM128(0x2)), _mm_setzero_ps());

    *s = sseSelect((SSEFloat4V)_mm_xor_ps(sseUintToM128(0x80000000), (SSEUint4V)*s), *s, sinMask);
    *c = sseSelect((SSEFloat4V)_mm_xor_ps(sseUintToM128(0x80000000), (SSEUint4V)*c), *c, cosMask);
}

static inline __m128 sseVecDot3(__m128 vec0, __m128 vec1)
{
    const __m128 result = _mm_mul_ps(vec0, vec1);
    return _mm_add_ps(sseSplat(result, 0), _mm_add_ps(sseSplat(result, 1), sseSplat(result, 2)));
}

static inline __m128 sseVecDot4(__m128 vec0, __m128 vec1)
{
    const __m128 result = _mm_mul_ps(vec0, vec1);
    return _mm_add_ps(_mm_shuffle_ps(result, result, _MM_SHUFFLE(0, 0, 0, 0)),
                      _mm_add_ps(_mm_shuffle_ps(result, result, _MM_SHUFFLE(1, 1, 1, 1)),
                                 _mm_add_ps(_mm_shuffle_ps(result, result, _MM_SHUFFLE(2, 2, 2, 2)), _mm_shuffle_ps(result, result, _MM_SHUFFLE(3, 3, 3, 3)))));
}

static inline __m128 sseVecCross(__m128 vec0, __m128 vec1)
{
    __m128 tmp0, tmp1, tmp2, tmp3, result;
    tmp0 = _mm_shuffle_ps(vec0, vec0, _MM_SHUFFLE(3, 0, 2, 1));
    tmp1 = _mm_shuffle_ps(vec1, vec1, _MM_SHUFFLE(3, 1, 0, 2));
    tmp2 = _mm_shuffle_ps(vec0, vec0, _MM_SHUFFLE(3, 1, 0, 2));
    tmp3 = _mm_shuffle_ps(vec1, vec1, _MM_SHUFFLE(3, 0, 2, 1));
    result = _mm_mul_ps(tmp0, tmp1);
    result = sseMSub(tmp2, tmp3, result);
    return result;
}

static inline __m128 sseVecInsert(__m128 dst, __m128 src, int slot)
{
    SSEFloat d;
    SSEFloat s;
    d.m128 = dst;
    s.m128 = src;
    d.f[slot] = s.f[slot];
    return d.m128;
}

static inline void sseVecSetElement(__m128 & vec, float scalar, int slot)
{
    ((float *)&(vec))[slot] = scalar;
}

} // namespace SSE
} // namespace Vectormath

#endif // VECTORMATH_SSE_INTERNAL_HPP


#ifndef VECTORMATH_SSE_FLOATINVEC_HPP
#define VECTORMATH_SSE_FLOATINVEC_HPP

namespace Vectormath
{
namespace SSE
{

class BoolInVec;

// ========================================================
// FloatInVec
// ========================================================

// Vectorized scalar float.
VECTORMATH_ALIGNED_TYPE_PRE class FloatInVec
{
    __m128 mData;

public:

    inline FloatInVec() { }
    inline FloatInVec(__m128 vec);
    inline FloatInVec(const FloatInVec & vec) = default;

    // matches standard type conversions
    inline FloatInVec(const BoolInVec & vec);

    // construct from a slot of __m128
    inline FloatInVec(__m128 vec, int slot);

    // explicit cast from float
    explicit inline FloatInVec(float scalar);

#ifdef VECTORMATH_NO_SCALAR_CAST
    // explicit cast to float
    inline float getAsFloat() const;
#else // !VECTORMATH_NO_SCALAR_CAST
    // implicit cast to float
    inline operator float() const;
#endif // VECTORMATH_NO_SCALAR_CAST

    // get vector data
    // float value is splatted across all word slots of vector
    inline __m128 get128() const;

    // operators
    inline const FloatInVec operator ++ (int);
    inline const FloatInVec operator -- (int);
    inline FloatInVec & operator ++ ();
    inline FloatInVec & operator -- ();
    inline const FloatInVec operator - () const;
    inline FloatInVec & operator =  (const FloatInVec & vec);
    inline FloatInVec & operator *= (const FloatInVec & vec);
    inline FloatInVec & operator /= (const FloatInVec & vec);
    inline FloatInVec & operator += (const FloatInVec & vec);
    inline FloatInVec & operator -= (const FloatInVec & vec);

    // friend functions
    friend inline const FloatInVec operator * (const FloatInVec & vec0, const FloatInVec & vec1);
    friend inline const FloatInVec operator / (const FloatInVec & vec0, const FloatInVec & vec1);
    friend inline const FloatInVec operator + (const FloatInVec & vec0, const FloatInVec & vec1);
    friend inline const FloatInVec operator - (const FloatInVec & vec0, const FloatInVec & vec1);
    friend inline const FloatInVec select(const FloatInVec & vec0, const FloatInVec & vec1, BoolInVec select_vec1);

} VECTORMATH_ALIGNED_TYPE_POST;

// ========================================================
// FloatInVec functions
// ========================================================

// operators
//
inline const FloatInVec operator *  (const FloatInVec & vec0, const FloatInVec & vec1);
inline const FloatInVec operator /  (const FloatInVec & vec0, const FloatInVec & vec1);
inline const FloatInVec operator +  (const FloatInVec & vec0, const FloatInVec & vec1);
inline const FloatInVec operator -  (const FloatInVec & vec0, const FloatInVec & vec1);
inline const BoolInVec  operator <  (const FloatInVec & vec0, const FloatInVec & vec1);
inline const BoolInVec  operator <= (const FloatInVec & vec0, const FloatInVec & vec1);
inline const BoolInVec  operator >  (const FloatInVec & vec0, const FloatInVec & vec1);
inline const BoolInVec  operator >= (const FloatInVec & vec0, const FloatInVec & vec1);
inline const BoolInVec  operator == (const FloatInVec & vec0, const FloatInVec & vec1);
inline const BoolInVec  operator != (const FloatInVec & vec0, const FloatInVec & vec1);

// select between vec0 and vec1 using BoolInVec.
// false selects vec0, true selects vec1
//
inline const FloatInVec select(const FloatInVec & vec0, const FloatInVec & vec1, const BoolInVec & select_vec1);

} // namespace SSE
} // namespace Vectormath

// ========================================================
// FloatInVec implementation
// ========================================================

#ifndef VECTORMATH_SSE_BOOLINVEC_HPP
#define VECTORMATH_SSE_BOOLINVEC_HPP

namespace Vectormath
{
namespace SSE
{

class FloatInVec;

// ========================================================
// BoolInVec
// ========================================================

// Vectorized boolean value.
VECTORMATH_ALIGNED_TYPE_PRE class BoolInVec
{
    __m128 mData;

    inline BoolInVec(__m128 vec);

public:

    inline BoolInVec() { }

    // matches standard type conversions
    inline BoolInVec(const FloatInVec & vec);

    // explicit cast from bool
    explicit inline BoolInVec(bool scalar);

#ifdef VECTORMATH_NO_SCALAR_CAST
    // explicit cast to bool
    inline bool getAsBool() const;
#else // !VECTORMATH_NO_SCALAR_CAST
    // implicit cast to bool
    inline operator bool() const;
#endif // VECTORMATH_NO_SCALAR_CAST

    // get vector data
    // bool value is splatted across all word slots of vector as 0 (false) or -1 (true)
    inline __m128 get128() const;

    // operators
    inline const BoolInVec operator ! () const;
    inline BoolInVec & operator =  (const BoolInVec & vec);
    inline BoolInVec & operator &= (const BoolInVec & vec);
    inline BoolInVec & operator ^= (const BoolInVec & vec);
    inline BoolInVec & operator |= (const BoolInVec & vec);

    // friend functions
    friend inline const BoolInVec operator == (const BoolInVec  & vec0, const BoolInVec  & vec1);
    friend inline const BoolInVec operator != (const BoolInVec  & vec0, const BoolInVec  & vec1);
    friend inline const BoolInVec operator <  (const FloatInVec & vec0, const FloatInVec & vec1);
    friend inline const BoolInVec operator <= (const FloatInVec & vec0, const FloatInVec & vec1);
    friend inline const BoolInVec operator >  (const FloatInVec & vec0, const FloatInVec & vec1);
    friend inline const BoolInVec operator >= (const FloatInVec & vec0, const FloatInVec & vec1);
    friend inline const BoolInVec operator == (const FloatInVec & vec0, const FloatInVec & vec1);
    friend inline const BoolInVec operator != (const FloatInVec & vec0, const FloatInVec & vec1);
    friend inline const BoolInVec operator &  (const BoolInVec  & vec0, const BoolInVec  & vec1);
    friend inline const BoolInVec operator ^  (const BoolInVec  & vec0, const BoolInVec  & vec1);
    friend inline const BoolInVec operator |  (const BoolInVec  & vec0, const BoolInVec  & vec1);
    friend inline const BoolInVec select(const BoolInVec & vec0, const BoolInVec & vec1, const BoolInVec & select_vec1);

} VECTORMATH_ALIGNED_TYPE_POST;

// ========================================================
// BoolInVec functions
// ========================================================

// operators
//
inline const BoolInVec operator == (const BoolInVec & vec0, const BoolInVec & vec1);
inline const BoolInVec operator != (const BoolInVec & vec0, const BoolInVec & vec1);
inline const BoolInVec operator &  (const BoolInVec & vec0, const BoolInVec & vec1);
inline const BoolInVec operator ^  (const BoolInVec & vec0, const BoolInVec & vec1);
inline const BoolInVec operator |  (const BoolInVec & vec0, const BoolInVec & vec1);

// select between vec0 and vec1 using BoolInVec.
// false selects vec0, true selects vec1
//
inline const BoolInVec select(const BoolInVec & vec0, const BoolInVec & vec1, const BoolInVec & select_vec1);

} // namespace SSE
} // namespace Vectormath

// ========================================================
// BoolInVec implementation
// ========================================================

namespace Vectormath
{
namespace SSE
{

inline BoolInVec::BoolInVec(__m128 vec)
{
    mData = vec;
}

inline BoolInVec::BoolInVec(const FloatInVec & vec)
{
    *this = (vec != FloatInVec(0.0f));
}

inline BoolInVec::BoolInVec(bool scalar)
{
    union
    {
        unsigned int mask;
        float f;
    } tmp;

    tmp.mask = -(int)scalar;
    mData = _mm_set1_ps(tmp.f);
}

#ifdef VECTORMATH_NO_SCALAR_CAST
inline bool BoolInVec::getAsBool() const
#else
inline BoolInVec::operator bool() const
#endif
{
    return *(const bool *)&mData;
}

inline __m128 BoolInVec::get128() const
{
    return mData;
}

inline const BoolInVec BoolInVec::operator ! () const
{
    return BoolInVec(_mm_andnot_ps(mData, _mm_cmpneq_ps(_mm_setzero_ps(), _mm_setzero_ps())));
}

inline BoolInVec & BoolInVec::operator = (const BoolInVec & vec)
{
    mData = vec.mData;
    return *this;
}

inline BoolInVec & BoolInVec::operator &= (const BoolInVec & vec)
{
    *this = *this & vec;
    return *this;
}

inline BoolInVec & BoolInVec::operator ^= (const BoolInVec & vec)
{
    *this = *this ^ vec;
    return *this;
}

inline BoolInVec & BoolInVec::operator |= (const BoolInVec & vec)
{
    *this = *this | vec;
    return *this;
}

inline const BoolInVec operator == (const BoolInVec & vec0, const BoolInVec & vec1)
{
    return BoolInVec(_mm_cmpeq_ps(vec0.get128(), vec1.get128()));
}

inline const BoolInVec operator != (const BoolInVec & vec0, const BoolInVec & vec1)
{
    return BoolInVec(_mm_cmpneq_ps(vec0.get128(), vec1.get128()));
}

inline const BoolInVec operator & (const BoolInVec & vec0, const BoolInVec & vec1)
{
    return BoolInVec(_mm_and_ps(vec0.get128(), vec1.get128()));
}

inline const BoolInVec operator | (const BoolInVec & vec0, const BoolInVec & vec1)
{
    return BoolInVec(_mm_or_ps(vec0.get128(), vec1.get128()));
}

inline const BoolInVec operator ^ (const BoolInVec & vec0, const BoolInVec & vec1)
{
    return BoolInVec(_mm_xor_ps(vec0.get128(), vec1.get128()));
}

inline const BoolInVec select(const BoolInVec & vec0, const BoolInVec & vec1, const BoolInVec & select_vec1)
{
    return BoolInVec(sseSelect(vec0.get128(), vec1.get128(), select_vec1.get128()));
}

} // namespace SSE
} // namespace Vectormath

#endif // VECTORMATH_SSE_BOOLINVEC_HPP

namespace Vectormath
{
namespace SSE
{

inline FloatInVec::FloatInVec(__m128 vec)
{
    mData = vec;
}

inline FloatInVec::FloatInVec(const BoolInVec & vec)
{
    mData = sseSelect(_mm_setzero_ps(), _mm_set1_ps(1.0f), vec.get128());
}

inline FloatInVec::FloatInVec(__m128 vec, int slot)
{
    SSEFloat v;
    v.m128 = vec;
    mData = _mm_set1_ps(v.f[slot]);
}

inline FloatInVec::FloatInVec(float scalar)
{
    mData = _mm_set1_ps(scalar);
}

#ifdef VECTORMATH_NO_SCALAR_CAST
inline float FloatInVec::getAsFloat() const
#else
inline FloatInVec::operator float() const
#endif
{
    return *((const float *)&mData);
}

inline __m128 FloatInVec::get128() const
{
    return mData;
}

inline const FloatInVec FloatInVec::operator ++ (int)
{
    __m128 olddata = mData;
    operator++();
    return FloatInVec(olddata);
}

inline const FloatInVec FloatInVec::operator -- (int)
{
    __m128 olddata = mData;
    operator--();
    return FloatInVec(olddata);
}

inline FloatInVec & FloatInVec::operator ++ ()
{
    *this += FloatInVec(_mm_set1_ps(1.0f));
    return *this;
}

inline FloatInVec & FloatInVec::operator -- ()
{
    *this -= FloatInVec(_mm_set1_ps(1.0f));
    return *this;
}

inline const FloatInVec FloatInVec::operator - () const
{
    return FloatInVec(_mm_sub_ps(_mm_setzero_ps(), mData));
}

inline FloatInVec & FloatInVec::operator = (const FloatInVec & vec)
{
    mData = vec.mData;
    return *this;
}

inline FloatInVec & FloatInVec::operator *= (const FloatInVec & vec)
{
    *this = *this * vec;
    return *this;
}

inline FloatInVec & FloatInVec::operator /= (const FloatInVec & vec)
{
    *this = *this / vec;
    return *this;
}

inline FloatInVec & FloatInVec::operator += (const FloatInVec & vec)
{
    *this = *this + vec;
    return *this;
}

inline FloatInVec & FloatInVec::operator -= (const FloatInVec & vec)
{
    *this = *this - vec;
    return *this;
}

inline const FloatInVec operator * (const FloatInVec & vec0, const FloatInVec & vec1)
{
    return FloatInVec(_mm_mul_ps(vec0.get128(), vec1.get128()));
}

inline const FloatInVec operator / (const FloatInVec & num, const FloatInVec & den)
{
    return FloatInVec(_mm_div_ps(num.get128(), den.get128()));
}

inline const FloatInVec operator + (const FloatInVec & vec0, const FloatInVec & vec1)
{
    return FloatInVec(_mm_add_ps(vec0.get128(), vec1.get128()));
}

inline const FloatInVec operator - (const FloatInVec & vec0, const FloatInVec & vec1)
{
    return FloatInVec(_mm_sub_ps(vec0.get128(), vec1.get128()));
}

inline const BoolInVec operator < (const FloatInVec & vec0, const FloatInVec & vec1)
{
    return BoolInVec(_mm_cmpgt_ps(vec1.get128(), vec0.get128()));
}

inline const BoolInVec operator <= (const FloatInVec & vec0, const FloatInVec & vec1)
{
    return BoolInVec(_mm_cmpge_ps(vec1.get128(), vec0.get128()));
}

inline const BoolInVec operator > (const FloatInVec & vec0, const FloatInVec & vec1)
{
    return BoolInVec(_mm_cmpgt_ps(vec0.get128(), vec1.get128()));
}

inline const BoolInVec operator >= (const FloatInVec & vec0, const FloatInVec & vec1)
{
    return BoolInVec(_mm_cmpge_ps(vec0.get128(), vec1.get128()));
}

inline const BoolInVec operator == (const FloatInVec & vec0, const FloatInVec & vec1)
{
    return BoolInVec(_mm_cmpeq_ps(vec0.get128(), vec1.get128()));
}

inline const BoolInVec operator != (const FloatInVec & vec0, const FloatInVec & vec1)
{
    return BoolInVec(_mm_cmpneq_ps(vec0.get128(), vec1.get128()));
}

inline const FloatInVec select(const FloatInVec & vec0, const FloatInVec & vec1, const BoolInVec & select_vec1)
{
    return FloatInVec(sseSelect(vec0.get128(), vec1.get128(), select_vec1.get128()));
}

} // namespace SSE
} // namespace Vectormath

#endif // VECTORMATH_SSE_FLOATINVEC_HPP

#ifndef VECTORMATH_SSE_VECIDX_HPP
#define VECTORMATH_SSE_VECIDX_HPP

namespace Vectormath
{
namespace SSE
{

// ========================================================
// VecIdx
// ========================================================

// Used in setting elements of Vector3, Vector4, Point3, or Quat
// with the subscripting operator [].
VECTORMATH_ALIGNED_TYPE_PRE class VecIdx
{
    __m128 & ref;
    int i;

public:

    inline VecIdx(__m128 & vec, int idx) : ref(vec), i(idx) { }

    // implicitly casts to float unless VECTORMATH_NO_SCALAR_CAST defined
    // in which case, implicitly casts to FloatInVec, and one must call
    // getAsFloat() to convert to float.
#ifdef VECTORMATH_NO_SCALAR_CAST
    inline operator FloatInVec() const;
    inline float getAsFloat() const;
#else // !VECTORMATH_NO_SCALAR_CAST
    inline operator float() const;
#endif // VECTORMATH_NO_SCALAR_CAST

    inline float operator = (float scalar);
    inline FloatInVec operator =  (const FloatInVec & scalar);
    inline FloatInVec operator =  (const VecIdx & scalar);
    inline FloatInVec operator *= (float scalar);
    inline FloatInVec operator *= (const FloatInVec & scalar);
    inline FloatInVec operator /= (float scalar);
    inline FloatInVec operator /= (const FloatInVec & scalar);
    inline FloatInVec operator += (float scalar);
    inline FloatInVec operator += (const FloatInVec & scalar);
    inline FloatInVec operator -= (float scalar);
    inline FloatInVec operator -= (const FloatInVec & scalar);

} VECTORMATH_ALIGNED_TYPE_POST;

} // namespace SSE
} // namespace Vectormath

#endif // VECTORMATH_SSE_VECIDX_HPP

namespace Vectormath
{
namespace SSE
{

// ========================================================
// Forward Declarations
// ========================================================

class Vector3;
class Vector4;
class Point3;
class Quat;
class Matrix3;
class Matrix4;
class Transform3;

// ========================================================
// A 3-D vector in array-of-structures format
// ========================================================

VECTORMATH_ALIGNED_TYPE_PRE class Vector3
{
    __m128 mVec128;

public:

    // Default constructor; does no initialization
    inline Vector3() { }

    // Copy constructor
    inline Vector3 (const Vector3 & vec) = default;

    // Construct a 3-D vector from x, y, and z elements
    inline Vector3(float x, float y, float z);

    // Construct a 3-D vector from x, y, and z elements (scalar data contained in vector data type)
    inline Vector3(const FloatInVec & x, const FloatInVec & y, const FloatInVec & z);

    // Copy elements from a 3-D point into a 3-D vector
    explicit inline Vector3(const Point3 & pnt);

    // Set all elements of a 3-D vector to the same scalar value
    explicit inline Vector3(float scalar);

    // Set all elements of a 3-D vector to the same scalar value (scalar data contained in vector data type)
    explicit inline Vector3(const FloatInVec & scalar);

    // Set vector float data in a 3-D vector
    explicit inline Vector3(__m128 vf4);

    // Get vector float data from a 3-D vector
    inline __m128 get128() const;

    // Assign one 3-D vector to another
    inline Vector3 & operator = (const Vector3 & vec);

    // Set the x element of a 3-D vector
    inline Vector3 & setX(float x);

    // Set the y element of a 3-D vector
    inline Vector3 & setY(float y);

    // Set the z element of a 3-D vector
    inline Vector3 & setZ(float z);

    // Set the w element of a padded 3-D vector
    // NOTE:
    // You are free to use the additional w component - if never set, it's value is undefined.
    inline Vector3 & setW(float w);

    // Set the x element of a 3-D vector (scalar data contained in vector data type)
    inline Vector3 & setX(const FloatInVec & x);

    // Set the y element of a 3-D vector (scalar data contained in vector data type)
    inline Vector3 & setY(const FloatInVec & y);

    // Set the z element of a 3-D vector (scalar data contained in vector data type)
    inline Vector3 & setZ(const FloatInVec & z);

    // Set the w element of a padded 3-D vector
    // NOTE:
    // You are free to use the additional w component - if never set, it's value is undefined.
    inline Vector3 & setW(const FloatInVec & w);

    // Get the x element of a 3-D vector
    inline const FloatInVec getX() const;

    // Get the y element of a 3-D vector
    inline const FloatInVec getY() const;

    // Get the z element of a 3-D vector
    inline const FloatInVec getZ() const;

    // Get the w element of a padded 3-D vector
    // NOTE:
    // You are free to use the additional w component - if never set, it's value is undefined.
    inline const FloatInVec getW() const;

    // Set an x, y, or z element of a 3-D vector by index
    inline Vector3 & setElem(int idx, float value);

    // Set an x, y, or z element of a 3-D vector by index (scalar data contained in vector data type)
    inline Vector3 & setElem(int idx, const FloatInVec & value);

    // Get an x, y, or z element of a 3-D vector by index
    inline const FloatInVec getElem(int idx) const;

    // Subscripting operator to set or get an element
    inline VecIdx operator[](int idx);

    // Subscripting operator to get an element
    inline const FloatInVec operator[](int idx) const;

    // Add two 3-D vectors
    inline const Vector3 operator + (const Vector3 & vec) const;

    // Subtract a 3-D vector from another 3-D vector
    inline const Vector3 operator - (const Vector3 & vec) const;

    // Add a 3-D vector to a 3-D point
    inline const Point3 operator + (const Point3 & pnt) const;

    // Multiply a 3-D vector by a scalar
    inline const Vector3 operator * (float scalar) const;

    // Divide a 3-D vector by a scalar
    inline const Vector3 operator / (float scalar) const;

    // Multiply a 3-D vector by a scalar (scalar data contained in vector data type)
    inline const Vector3 operator * (const FloatInVec & scalar) const;

    // Divide a 3-D vector by a scalar (scalar data contained in vector data type)
    inline const Vector3 operator / (const FloatInVec & scalar) const;

    // Perform compound assignment and addition with a 3-D vector
    inline Vector3 & operator += (const Vector3 & vec);

    // Perform compound assignment and subtraction by a 3-D vector
    inline Vector3 & operator -= (const Vector3 & vec);

    // Perform compound assignment and multiplication by a scalar
    inline Vector3 & operator *= (float scalar);

    // Perform compound assignment and division by a scalar
    inline Vector3 & operator /= (float scalar);

    // Perform compound assignment and multiplication by a scalar (scalar data contained in vector data type)
    inline Vector3 & operator *= (const FloatInVec & scalar);

    // Perform compound assignment and division by a scalar (scalar data contained in vector data type)
    inline Vector3 & operator /= (const FloatInVec & scalar);

    // Negate all elements of a 3-D vector
    inline const Vector3 operator - () const;

    // Construct x axis
    static inline const Vector3 xAxis();

    // Construct y axis
    static inline const Vector3 yAxis();

    // Construct z axis
    static inline const Vector3 zAxis();

} VECTORMATH_ALIGNED_TYPE_POST;

// Multiply a 3-D vector by a scalar
//
inline const Vector3 operator * (float scalar, const Vector3 & vec);

// Multiply a 3-D vector by a scalar (scalar data contained in vector data type)
//
inline const Vector3 operator * (const FloatInVec & scalar, const Vector3 & vec);

// Multiply two 3-D vectors per element
//
inline const Vector3 mulPerElem(const Vector3 & vec0, const Vector3 & vec1);

// Divide two 3-D vectors per element
// NOTE:
// Floating-point behavior matches standard library function divf4.
//
inline const Vector3 divPerElem(const Vector3 & vec0, const Vector3 & vec1);

// Compute the reciprocal of a 3-D vector per element
// NOTE:
// Floating-point behavior matches standard library function recipf4.
//
inline const Vector3 recipPerElem(const Vector3 & vec);

// Compute the absolute value of a 3-D vector per element
//
inline const Vector3 absPerElem(const Vector3 & vec);

// Copy sign from one 3-D vector to another, per element
//
inline const Vector3 copySignPerElem(const Vector3 & vec0, const Vector3 & vec1);

// Maximum of two 3-D vectors per element
//
inline const Vector3 maxPerElem(const Vector3 & vec0, const Vector3 & vec1);

// Minimum of two 3-D vectors per element
//
inline const Vector3 minPerElem(const Vector3 & vec0, const Vector3 & vec1);

// Maximum element of a 3-D vector
//
inline const FloatInVec maxElem(const Vector3 & vec);

// Minimum element of a 3-D vector
//
inline const FloatInVec minElem(const Vector3 & vec);

// Compute the sum of all elements of a 3-D vector
//
inline const FloatInVec sum(const Vector3 & vec);

// Compute the dot product of two 3-D vectors
//
inline const FloatInVec dot(const Vector3 & vec0, const Vector3 & vec1);

// Compute the square of the length of a 3-D vector
//
inline const FloatInVec lengthSqr(const Vector3 & vec);

// Compute the length of a 3-D vector
//
inline const FloatInVec length(const Vector3 & vec);

// Normalize a 3-D vector
// NOTE:
// The result is unpredictable when all elements of vec are at or near zero.
//
inline const Vector3 normalize(const Vector3 & vec);

// Compute cross product of two 3-D vectors
//
inline const Vector3 cross(const Vector3 & vec0, const Vector3 & vec1);

// Outer product of two 3-D vectors
//
inline const Matrix3 outer(const Vector3 & vec0, const Vector3 & vec1);

// Pre-multiply a row vector by a 3x3 matrix
// NOTE:
// Slower than column post-multiply.
//
inline const Vector3 rowMul(const Vector3 & vec, const Matrix3 & mat);

// Cross-product matrix of a 3-D vector
//
inline const Matrix3 crossMatrix(const Vector3 & vec);

// Create cross-product matrix and multiply
// NOTE:
// Faster than separately creating a cross-product matrix and multiplying.
//
inline const Matrix3 crossMatrixMul(const Vector3 & vec, const Matrix3 & mat);

// Linear interpolation between two 3-D vectors
// NOTE:
// Does not clamp t between 0 and 1.
//
inline const Vector3 lerp(float t, const Vector3 & vec0, const Vector3 & vec1);

// Linear interpolation between two 3-D vectors (scalar data contained in vector data type)
// NOTE:
// Does not clamp t between 0 and 1.
//
inline const Vector3 lerp(const FloatInVec & t, const Vector3 & vec0, const Vector3 & vec1);

// Spherical linear interpolation between two 3-D vectors
// NOTE:
// The result is unpredictable if the vectors point in opposite directions.
// Does not clamp t between 0 and 1.
//
inline const Vector3 slerp(float t, const Vector3 & unitVec0, const Vector3 & unitVec1);

// Spherical linear interpolation between two 3-D vectors (scalar data contained in vector data type)
// NOTE:
// The result is unpredictable if the vectors point in opposite directions.
// Does not clamp t between 0 and 1.
//
inline const Vector3 slerp(const FloatInVec & t, const Vector3 & unitVec0, const Vector3 & unitVec1);

// Conditionally select between two 3-D vectors
// NOTE:
// This function uses a conditional select instruction to avoid a branch.
// However, the transfer of select1 to a VMX register may use more processing time than a branch.
// Use the BoolInVec version for better performance.
//
inline const Vector3 select(const Vector3 & vec0, const Vector3 & vec1, bool select1);

// Conditionally select between two 3-D vectors (scalar data contained in vector data type)
// NOTE:
// This function uses a conditional select instruction to avoid a branch.
//
inline const Vector3 select(const Vector3 & vec0, const Vector3 & vec1, const BoolInVec & select1);

// Store x, y, and z elements of 3-D vector in first three words of a quadword, preserving fourth word
//
inline void storeXYZ(const Vector3 & vec, __m128 * quad);

// Load four three-float 3-D vectors, stored in three quadwords
//
inline void loadXYZArray(Vector3 & vec0, Vector3 & vec1, Vector3 & vec2, Vector3 & vec3, const __m128 * threeQuads);

// Store four 3-D vectors in three quadwords
//
inline void storeXYZArray(const Vector3 & vec0, const Vector3 & vec1, const Vector3 & vec2, const Vector3 & vec3, __m128 * threeQuads);

#ifdef VECTORMATH_DEBUG

// Print a 3-D vector
// NOTE:
// Function is only defined when VECTORMATH_DEBUG is defined.
//
inline void print(const Vector3 & vec);

// Print a 3-D vector and an associated string identifier
// NOTE:
// Function is only defined when VECTORMATH_DEBUG is defined.
//
inline void print(const Vector3 & vec, const char * name);

#endif // VECTORMATH_DEBUG

// ========================================================
// A 4-D vector in array-of-structures format
// ========================================================

VECTORMATH_ALIGNED_TYPE_PRE class Vector4
{
    __m128 mVec128;

public:

    // Default constructor; does no initialization
    inline Vector4() { }

    // Copy constructor
    inline Vector4 (const Vector4 & vec) = default;

    // Construct a 4-D vector from x, y, z, and w elements
    inline Vector4(float x, float y, float z, float w);

    // Construct a 4-D vector from x, y, z, and w elements (scalar data contained in vector data type)
    inline Vector4(const FloatInVec & x, const FloatInVec & y, const FloatInVec & z, const FloatInVec & w);

    // Construct a 4-D vector from a 3-D vector and a scalar
    inline Vector4(const Vector3 & xyz, float w);

    // Construct a 4-D vector from a 3-D vector and a scalar (scalar data contained in vector data type)
    inline Vector4(const Vector3 & xyz, const FloatInVec & w);

    // Copy x, y, and z from a 3-D vector into a 4-D vector, and set w to 0
    explicit inline Vector4(const Vector3 & vec);

    // Copy x, y, and z from a 3-D point into a 4-D vector, and set w to 1
    explicit inline Vector4(const Point3 & pnt);

    // Copy elements from a quaternion into a 4-D vector
    explicit inline Vector4(const Quat & quat);

    // Set all elements of a 4-D vector to the same scalar value
    explicit inline Vector4(float scalar);

    // Set all elements of a 4-D vector to the same scalar value (scalar data contained in vector data type)
    explicit inline Vector4(const FloatInVec & scalar);

    // Set vector float data in a 4-D vector
    explicit inline Vector4(__m128 vf4);

    // Get vector float data from a 4-D vector
    inline __m128 get128() const;

    // Assign one 4-D vector to another
    inline Vector4 & operator = (const Vector4 & vec);

    // Set the x, y, and z elements of a 4-D vector
    // NOTE:
    // This function does not change the w element.
    inline Vector4 & setXYZ(const Vector3 & vec);

    // Get the x, y, and z elements of a 4-D vector
    inline const Vector3 getXYZ() const;

    // Set the x element of a 4-D vector
    inline Vector4 & setX(float x);

    // Set the y element of a 4-D vector
    inline Vector4 & setY(float y);

    // Set the z element of a 4-D vector
    inline Vector4 & setZ(float z);

    // Set the w element of a 4-D vector
    inline Vector4 & setW(float w);

    // Set the x element of a 4-D vector (scalar data contained in vector data type)
    inline Vector4 & setX(const FloatInVec & x);

    // Set the y element of a 4-D vector (scalar data contained in vector data type)
    inline Vector4 & setY(const FloatInVec & y);

    // Set the z element of a 4-D vector (scalar data contained in vector data type)
    inline Vector4 & setZ(const FloatInVec & z);

    // Set the w element of a 4-D vector (scalar data contained in vector data type)
    inline Vector4 & setW(const FloatInVec & w);

    // Get the x element of a 4-D vector
    inline const FloatInVec getX() const;

    // Get the y element of a 4-D vector
    inline const FloatInVec getY() const;

    // Get the z element of a 4-D vector
    inline const FloatInVec getZ() const;

    // Get the w element of a 4-D vector
    inline const FloatInVec getW() const;

    // Set an x, y, z, or w element of a 4-D vector by index
    inline Vector4 & setElem(int idx, float value);

    // Set an x, y, z, or w element of a 4-D vector by index (scalar data contained in vector data type)
    inline Vector4 & setElem(int idx, const FloatInVec & value);

    // Get an x, y, z, or w element of a 4-D vector by index
    inline const FloatInVec getElem(int idx) const;

    // Subscripting operator to set or get an element
    inline VecIdx operator[](int idx);

    // Subscripting operator to get an element
    inline const FloatInVec operator[](int idx) const;

    // Add two 4-D vectors
    inline const Vector4 operator + (const Vector4 & vec) const;

    // Subtract a 4-D vector from another 4-D vector
    inline const Vector4 operator - (const Vector4 & vec) const;

    // Multiply a 4-D vector by a scalar
    inline const Vector4 operator * (float scalar) const;

    // Divide a 4-D vector by a scalar
    inline const Vector4 operator / (float scalar) const;

    // Multiply a 4-D vector by a scalar (scalar data contained in vector data type)
    inline const Vector4 operator * (const FloatInVec & scalar) const;

    // Divide a 4-D vector by a scalar (scalar data contained in vector data type)
    inline const Vector4 operator / (const FloatInVec & scalar) const;

    // Perform compound assignment and addition with a 4-D vector
    inline Vector4 & operator += (const Vector4 & vec);

    // Perform compound assignment and subtraction by a 4-D vector
    inline Vector4 & operator -= (const Vector4 & vec);

    // Perform compound assignment and multiplication by a scalar
    inline Vector4 & operator *= (float scalar);

    // Perform compound assignment and division by a scalar
    inline Vector4 & operator /= (float scalar);

    // Perform compound assignment and multiplication by a scalar (scalar data contained in vector data type)
    inline Vector4 & operator *= (const FloatInVec & scalar);

    // Perform compound assignment and division by a scalar (scalar data contained in vector data type)
    inline Vector4 & operator /= (const FloatInVec & scalar);

    // Negate all elements of a 4-D vector
    inline const Vector4 operator - () const;

    // Construct x axis
    static inline const Vector4 xAxis();

    // Construct y axis
    static inline const Vector4 yAxis();

    // Construct z axis
    static inline const Vector4 zAxis();

    // Construct w axis
    static inline const Vector4 wAxis();

} VECTORMATH_ALIGNED_TYPE_POST;

// Multiply a 4-D vector by a scalar
//
inline const Vector4 operator * (float scalar, const Vector4 & vec);

// Multiply a 4-D vector by a scalar (scalar data contained in vector data type)
//
inline const Vector4 operator * (const FloatInVec & scalar, const Vector4 & vec);

// Multiply two 4-D vectors per element
//
inline const Vector4 mulPerElem(const Vector4 & vec0, const Vector4 & vec1);

// Divide two 4-D vectors per element
// NOTE:
// Floating-point behavior matches standard library function divf4.
//
inline const Vector4 divPerElem(const Vector4 & vec0, const Vector4 & vec1);

// Compute the reciprocal of a 4-D vector per element
// NOTE:
// Floating-point behavior matches standard library function recipf4.
//
inline const Vector4 recipPerElem(const Vector4 & vec);

// Compute the absolute value of a 4-D vector per element
//
inline const Vector4 absPerElem(const Vector4 & vec);

// Copy sign from one 4-D vector to another, per element
//
inline const Vector4 copySignPerElem(const Vector4 & vec0, const Vector4 & vec1);

// Maximum of two 4-D vectors per element
//
inline const Vector4 maxPerElem(const Vector4 & vec0, const Vector4 & vec1);

// Minimum of two 4-D vectors per element
//
inline const Vector4 minPerElem(const Vector4 & vec0, const Vector4 & vec1);

// Maximum element of a 4-D vector
//
inline const FloatInVec maxElem(const Vector4 & vec);

// Minimum element of a 4-D vector
//
inline const FloatInVec minElem(const Vector4 & vec);

// Compute the sum of all elements of a 4-D vector
//
inline const FloatInVec sum(const Vector4 & vec);

// Compute the dot product of two 4-D vectors
//
inline const FloatInVec dot(const Vector4 & vec0, const Vector4 & vec1);

// Compute the square of the length of a 4-D vector
//
inline const FloatInVec lengthSqr(const Vector4 & vec);

// Compute the length of a 4-D vector
//
inline const FloatInVec length(const Vector4 & vec);

// Normalize a 4-D vector
// NOTE:
// The result is unpredictable when all elements of vec are at or near zero.
//
inline const Vector4 normalize(const Vector4 & vec);

// Outer product of two 4-D vectors
//
inline const Matrix4 outer(const Vector4 & vec0, const Vector4 & vec1);

// Linear interpolation between two 4-D vectors
// NOTE:
// Does not clamp t between 0 and 1.
//
inline const Vector4 lerp(float t, const Vector4 & vec0, const Vector4 & vec1);

// Linear interpolation between two 4-D vectors (scalar data contained in vector data type)
// NOTE:
// Does not clamp t between 0 and 1.
//
inline const Vector4 lerp(const FloatInVec & t, const Vector4 & vec0, const Vector4 & vec1);

// Spherical linear interpolation between two 4-D vectors
// NOTE:
// The result is unpredictable if the vectors point in opposite directions.
// Does not clamp t between 0 and 1.
//
inline const Vector4 slerp(float t, const Vector4 & unitVec0, const Vector4 & unitVec1);

// Spherical linear interpolation between two 4-D vectors (scalar data contained in vector data type)
// NOTE:
// The result is unpredictable if the vectors point in opposite directions.
// Does not clamp t between 0 and 1.
//
inline const Vector4 slerp(const FloatInVec & t, const Vector4 & unitVec0, const Vector4 & unitVec1);

// Conditionally select between two 4-D vectors
// NOTE:
// This function uses a conditional select instruction to avoid a branch.
// However, the transfer of select1 to a VMX register may use more processing time than a branch.
// Use the BoolInVec version for better performance.
//
inline const Vector4 select(const Vector4 & vec0, const Vector4 & vec1, bool select1);

// Conditionally select between two 4-D vectors (scalar data contained in vector data type)
// NOTE:
// This function uses a conditional select instruction to avoid a branch.
//
inline const Vector4 select(const Vector4 & vec0, const Vector4 & vec1, const BoolInVec & select1);

#ifdef VECTORMATH_DEBUG

// Print a 4-D vector
// NOTE:
// Function is only defined when VECTORMATH_DEBUG is defined.
//
inline void print(const Vector4 & vec);

// Print a 4-D vector and an associated string identifier
// NOTE:
// Function is only defined when VECTORMATH_DEBUG is defined.
//
inline void print(const Vector4 & vec, const char * name);

#endif // VECTORMATH_DEBUG

// ========================================================
// A 3-D point in array-of-structures format
// ========================================================

VECTORMATH_ALIGNED_TYPE_PRE class Point3
{
    __m128 mVec128;

public:

    // Default constructor; does no initialization
    inline Point3() { }

    // Construct a 3-D point from x, y, and z elements
    inline Point3(float x, float y, float z);

    // Construct a 3-D point from x, y, and z elements (scalar data contained in vector data type)
    inline Point3(const FloatInVec & x, const FloatInVec & y, const FloatInVec & z);

    // Copy elements from a 3-D vector into a 3-D point
    explicit inline Point3(const Vector3 & vec);

    // Set all elements of a 3-D point to the same scalar value
    explicit inline Point3(float scalar);

    // Set all elements of a 3-D point to the same scalar value (scalar data contained in vector data type)
    explicit inline Point3(const FloatInVec & scalar);

    // Set vector float data in a 3-D point
    explicit inline Point3(__m128 vf4);

    // Get vector float data from a 3-D point
    inline __m128 get128() const;

    // Assign one 3-D point to another
    inline Point3 & operator = (const Point3 & pnt);

    // Set the x element of a 3-D point
    inline Point3 & setX(float x);

    // Set the y element of a 3-D point
    inline Point3 & setY(float y);

    // Set the z element of a 3-D point
    inline Point3 & setZ(float z);

    // Set the w element of a padded 3-D point
    // NOTE:
    // You are free to use the additional w component - if never set, it's value is undefined.
    inline Point3 & setW(float w);

    // Set the x element of a 3-D point (scalar data contained in vector data type)
    inline Point3 & setX(const FloatInVec & x);

    // Set the y element of a 3-D point (scalar data contained in vector data type)
    inline Point3 & setY(const FloatInVec & y);

    // Set the z element of a 3-D point (scalar data contained in vector data type)
    inline Point3 & setZ(const FloatInVec & z);

    // Set the w element of a padded 3-D point
    // NOTE:
    // You are free to use the additional w component - if never set, it's value is undefined.
    inline Point3 & setW(const FloatInVec & w);

    // Get the x element of a 3-D point
    inline const FloatInVec getX() const;

    // Get the y element of a 3-D point
    inline const FloatInVec getY() const;

    // Get the z element of a 3-D point
    inline const FloatInVec getZ() const;

    // Get the w element of a padded 3-D point
    // NOTE:
    // You are free to use the additional w component - if never set, it's value is undefined.
    inline const FloatInVec getW() const;

    // Set an x, y, or z element of a 3-D point by index
    inline Point3 & setElem(int idx, float value);

    // Set an x, y, or z element of a 3-D point by index (scalar data contained in vector data type)
    inline Point3 & setElem(int idx, const FloatInVec & value);

    // Get an x, y, or z element of a 3-D point by index
    inline const FloatInVec getElem(int idx) const;

    // Subscripting operator to set or get an element
    inline VecIdx operator[](int idx);

    // Subscripting operator to get an element
    inline const FloatInVec operator[](int idx) const;

    // Subtract a 3-D point from another 3-D point
    inline const Vector3 operator - (const Point3 & pnt) const;

    // Add a 3-D point to a 3-D vector
    inline const Point3 operator + (const Vector3 & vec) const;

    // Subtract a 3-D vector from a 3-D point
    inline const Point3 operator - (const Vector3 & vec) const;

    // Perform compound assignment and addition with a 3-D vector
    inline Point3 & operator += (const Vector3 & vec);

    // Perform compound assignment and subtraction by a 3-D vector
    inline Point3 & operator -= (const Vector3 & vec);

} VECTORMATH_ALIGNED_TYPE_POST;

// Multiply two 3-D points per element
//
inline const Point3 mulPerElem(const Point3 & pnt0, const Point3 & pnt1);

// Divide two 3-D points per element
// NOTE:
// Floating-point behavior matches standard library function divf4.
//
inline const Point3 divPerElem(const Point3 & pnt0, const Point3 & pnt1);

// Compute the reciprocal of a 3-D point per element
// NOTE:
// Floating-point behavior matches standard library function recipf4.
//
inline const Point3 recipPerElem(const Point3 & pnt);

// Compute the absolute value of a 3-D point per element
//
inline const Point3 absPerElem(const Point3 & pnt);

// Copy sign from one 3-D point to another, per element
//
inline const Point3 copySignPerElem(const Point3 & pnt0, const Point3 & pnt1);

// Maximum of two 3-D points per element
//
inline const Point3 maxPerElem(const Point3 & pnt0, const Point3 & pnt1);

// Minimum of two 3-D points per element
//
inline const Point3 minPerElem(const Point3 & pnt0, const Point3 & pnt1);

// Maximum element of a 3-D point
//
inline const FloatInVec maxElem(const Point3 & pnt);

// Minimum element of a 3-D point
//
inline const FloatInVec minElem(const Point3 & pnt);

// Compute the sum of all elements of a 3-D point
//
inline const FloatInVec sum(const Point3 & pnt);

// Apply uniform scale to a 3-D point
//
inline const Point3 scale(const Point3 & pnt, float scaleVal);

// Apply uniform scale to a 3-D point (scalar data contained in vector data type)
//
inline const Point3 scale(const Point3 & pnt, const FloatInVec & scaleVal);

// Apply non-uniform scale to a 3-D point
//
inline const Point3 scale(const Point3 & pnt, const Vector3 & scaleVec);

// Scalar projection of a 3-D point on a unit-length 3-D vector
//
inline const FloatInVec projection(const Point3 & pnt, const Vector3 & unitVec);

// Compute the square of the distance of a 3-D point from the coordinate-system origin
//
inline const FloatInVec distSqrFromOrigin(const Point3 & pnt);

// Compute the distance of a 3-D point from the coordinate-system origin
//
inline const FloatInVec distFromOrigin(const Point3 & pnt);

// Compute the square of the distance between two 3-D points
//
inline const FloatInVec distSqr(const Point3 & pnt0, const Point3 & pnt1);

// Compute the distance between two 3-D points
//
inline const FloatInVec distp(const Point3 & pnt0, const Point3 & pnt1);

// Linear interpolation between two 3-D points
// NOTE:
// Does not clamp t between 0 and 1.
//
inline const Point3 lerp(float t, const Point3 & pnt0, const Point3 & pnt1);

// Linear interpolation between two 3-D points (scalar data contained in vector data type)
// NOTE:
// Does not clamp t between 0 and 1.
//
inline const Point3 lerp(const FloatInVec & t, const Point3 & pnt0, const Point3 & pnt1);

// Conditionally select between two 3-D points
// NOTE:
// This function uses a conditional select instruction to avoid a branch.
// However, the transfer of select1 to a VMX register may use more processing time than a branch.
// Use the BoolInVec version for better performance.
//
inline const Point3 select(const Point3 & pnt0, const Point3 & pnt1, bool select1);

// Conditionally select between two 3-D points (scalar data contained in vector data type)
// NOTE:
// This function uses a conditional select instruction to avoid a branch.
//
inline const Point3 select(const Point3 & pnt0, const Point3 & pnt1, const BoolInVec & select1);

// Store x, y, and z elements of 3-D point in first three words of a quadword, preserving fourth word
//
inline void storeXYZ(const Point3 & pnt, __m128 * quad);

// Load four three-float 3-D points, stored in three quadwords
//
inline void loadXYZArray(Point3 & pnt0, Point3 & pnt1, Point3 & pnt2, Point3 & pnt3, const __m128 * threeQuads);

// Store four 3-D points in three quadwords
//
inline void storeXYZArray(const Point3 & pnt0, const Point3 & pnt1, const Point3 & pnt2, const Point3 & pnt3, __m128 * threeQuads);

#ifdef VECTORMATH_DEBUG

// Print a 3-D point
// NOTE:
// Function is only defined when VECTORMATH_DEBUG is defined.
//
inline void print(const Point3 & pnt);

// Print a 3-D point and an associated string identifier
// NOTE:
// Function is only defined when VECTORMATH_DEBUG is defined.
//
inline void print(const Point3 & pnt, const char * name);

#endif // VECTORMATH_DEBUG

// ========================================================
// A quaternion in array-of-structures format
// ========================================================

VECTORMATH_ALIGNED_TYPE_PRE class Quat
{
    __m128 mVec128;

public:

    // Default constructor; does no initialization
    inline Quat() { }

    // Construct a quaternion from x, y, z, and w elements
    inline Quat(float x, float y, float z, float w);

    // Construct a quaternion from x, y, z, and w elements (scalar data contained in vector data type)
    inline Quat(const FloatInVec & x, const FloatInVec & y, const FloatInVec & z, const FloatInVec & w);

    // Construct a quaternion from a 3-D vector and a scalar
    inline Quat(const Vector3 & xyz, float w);

    // Construct a quaternion from a 3-D vector and a scalar (scalar data contained in vector data type)
    inline Quat(const Vector3 & xyz, const FloatInVec & w);

    // Copy elements from a 4-D vector into a quaternion
    explicit inline Quat(const Vector4 & vec);

    // Convert a rotation matrix to a unit-length quaternion
    explicit inline Quat(const Matrix3 & rotMat);

    // Set all elements of a quaternion to the same scalar value
    explicit inline Quat(float scalar);

    // Set all elements of a quaternion to the same scalar value (scalar data contained in vector data type)
    explicit inline Quat(const FloatInVec & scalar);

    // Set vector float data in a quaternion
    explicit inline Quat(__m128 vf4);

    // Get vector float data from a quaternion
    inline __m128 get128() const;

    // Assign one quaternion to another
    inline Quat & operator = (const Quat & quat);

    // Set the x, y, and z elements of a quaternion
    // NOTE:
    // This function does not change the w element.
    inline Quat & setXYZ(const Vector3 & vec);

    // Get the x, y, and z elements of a quaternion
    inline const Vector3 getXYZ() const;

    // Set the x element of a quaternion
    inline Quat & setX(float x);

    // Set the y element of a quaternion
    inline Quat & setY(float y);

    // Set the z element of a quaternion
    inline Quat & setZ(float z);

    // Set the w element of a quaternion
    inline Quat & setW(float w);

    // Set the x element of a quaternion (scalar data contained in vector data type)
    inline Quat & setX(const FloatInVec & x);

    // Set the y element of a quaternion (scalar data contained in vector data type)
    inline Quat & setY(const FloatInVec & y);

    // Set the z element of a quaternion (scalar data contained in vector data type)
    inline Quat & setZ(const FloatInVec & z);

    // Set the w element of a quaternion (scalar data contained in vector data type)
    inline Quat & setW(const FloatInVec & w);

    // Get the x element of a quaternion
    inline const FloatInVec getX() const;

    // Get the y element of a quaternion
    inline const FloatInVec getY() const;

    // Get the z element of a quaternion
    inline const FloatInVec getZ() const;

    // Get the w element of a quaternion
    inline const FloatInVec getW() const;

    // Set an x, y, z, or w element of a quaternion by index
    inline Quat & setElem(int idx, float value);

    // Set an x, y, z, or w element of a quaternion by index (scalar data contained in vector data type)
    inline Quat & setElem(int idx, const FloatInVec & value);

    // Get an x, y, z, or w element of a quaternion by index
    inline const FloatInVec getElem(int idx) const;

    // Subscripting operator to set or get an element
    inline VecIdx operator[](int idx);

    // Subscripting operator to get an element
    inline const FloatInVec operator[](int idx) const;

    // Add two quaternions
    inline const Quat operator + (const Quat & quat) const;

    // Subtract a quaternion from another quaternion
    inline const Quat operator - (const Quat & quat) const;

    // Multiply two quaternions
    inline const Quat operator * (const Quat & quat) const;

    // Multiply a quaternion by a scalar
    inline const Quat operator * (float scalar) const;

    // Divide a quaternion by a scalar
    inline const Quat operator / (float scalar) const;

    // Multiply a quaternion by a scalar (scalar data contained in vector data type)
    inline const Quat operator * (const FloatInVec & scalar) const;

    // Divide a quaternion by a scalar (scalar data contained in vector data type)
    inline const Quat operator / (const FloatInVec & scalar) const;

    // Perform compound assignment and addition with a quaternion
    inline Quat & operator += (const Quat & quat);

    // Perform compound assignment and subtraction by a quaternion
    inline Quat & operator -= (const Quat & quat);

    // Perform compound assignment and multiplication by a quaternion
    inline Quat & operator *= (const Quat & quat);

    // Perform compound assignment and multiplication by a scalar
    inline Quat & operator *= (float scalar);

    // Perform compound assignment and division by a scalar
    inline Quat & operator /= (float scalar);

    // Perform compound assignment and multiplication by a scalar (scalar data contained in vector data type)
    inline Quat & operator *= (const FloatInVec & scalar);

    // Perform compound assignment and division by a scalar (scalar data contained in vector data type)
    inline Quat & operator /= (const FloatInVec & scalar);

    // Negate all elements of a quaternion
    inline const Quat operator - () const;

    // Construct an identity quaternion
    static inline const Quat identity();

    // Construct a quaternion to rotate between two unit-length 3-D vectors
    // NOTE:
    // The result is unpredictable if unitVec0 and unitVec1 point in opposite directions.
    static inline const Quat rotation(const Vector3 & unitVec0, const Vector3 & unitVec1);

    // Construct a quaternion to rotate around a unit-length 3-D vector
    static inline const Quat rotation(float radians, const Vector3 & unitVec);

    // Construct a quaternion to rotate around a unit-length 3-D vector (scalar data contained in vector data type)
    static inline const Quat rotation(const FloatInVec & radians, const Vector3 & unitVec);

    // Construct a quaternion to rotate around the x axis
    static inline const Quat rotationX(float radians);

    // Construct a quaternion to rotate around the y axis
    static inline const Quat rotationY(float radians);

    // Construct a quaternion to rotate around the z axis
    static inline const Quat rotationZ(float radians);

    // Construct a quaternion to rotate around the x axis (scalar data contained in vector data type)
    static inline const Quat rotationX(const FloatInVec & radians);

    // Construct a quaternion to rotate around the y axis (scalar data contained in vector data type)
    static inline const Quat rotationY(const FloatInVec & radians);

    // Construct a quaternion to rotate around the z axis (scalar data contained in vector data type)
    static inline const Quat rotationZ(const FloatInVec & radians);

} VECTORMATH_ALIGNED_TYPE_POST;

// Multiply a quaternion by a scalar
//
inline const Quat operator * (float scalar, const Quat & quat);

// Multiply a quaternion by a scalar (scalar data contained in vector data type)
//
inline const Quat operator * (const FloatInVec & scalar, const Quat & quat);

// Compute the conjugate of a quaternion
//
inline const Quat conj(const Quat & quat);

// Use a unit-length quaternion to rotate a 3-D vector
//
inline const Vector3 rotate(const Quat & unitQuat, const Vector3 & vec);

// Compute the dot product of two quaternions
//
inline const FloatInVec dot(const Quat & quat0, const Quat & quat1);

// Compute the norm of a quaternion
//
inline const FloatInVec norm(const Quat & quat);

// Compute the length of a quaternion
//
inline const FloatInVec length(const Quat & quat);

// Normalize a quaternion
// NOTE:
// The result is unpredictable when all elements of quat are at or near zero.
//
inline const Quat normalize(const Quat & quat);

// Linear interpolation between two quaternions
// NOTE:
// Does not clamp t between 0 and 1.
//
inline const Quat lerp(float t, const Quat & quat0, const Quat & quat1);

// Linear interpolation between two quaternions (scalar data contained in vector data type)
// NOTE:
// Does not clamp t between 0 and 1.
//
inline const Quat lerp(const FloatInVec & t, const Quat & quat0, const Quat & quat1);

// Spherical linear interpolation between two quaternions
// NOTE:
// Interpolates along the shortest path between orientations.
// Does not clamp t between 0 and 1.
//
inline const Quat slerp(float t, const Quat & unitQuat0, const Quat & unitQuat1);

// Spherical linear interpolation between two quaternions (scalar data contained in vector data type)
// NOTE:
// Interpolates along the shortest path between orientations.
// Does not clamp t between 0 and 1.
//
inline const Quat slerp(const FloatInVec & t, const Quat & unitQuat0, const Quat & unitQuat1);

// Spherical quadrangle interpolation
//
inline const Quat squad(float t, const Quat & unitQuat0, const Quat & unitQuat1, const Quat & unitQuat2, const Quat & unitQuat3);

// Spherical quadrangle interpolation (scalar data contained in vector data type)
//
inline const Quat squad(const FloatInVec & t, const Quat & unitQuat0, const Quat & unitQuat1, const Quat & unitQuat2, const Quat & unitQuat3);

// Conditionally select between two quaternions
// NOTE:
// This function uses a conditional select instruction to avoid a branch.
// However, the transfer of select1 to a VMX register may use more processing time than a branch.
// Use the BoolInVec version for better performance.
//
inline const Quat select(const Quat & quat0, const Quat & quat1, bool select1);

// Conditionally select between two quaternions (scalar data contained in vector data type)
// NOTE:
// This function uses a conditional select instruction to avoid a branch.
//
inline const Quat select(const Quat & quat0, const Quat & quat1, const BoolInVec & select1);

#ifdef VECTORMATH_DEBUG

// Print a quaternion
// NOTE:
// Function is only defined when VECTORMATH_DEBUG is defined.
//
inline void print(const Quat & quat);

// Print a quaternion and an associated string identifier
// NOTE:
// Function is only defined when VECTORMATH_DEBUG is defined.
//
inline void print(const Quat & quat, const char * name);

#endif // VECTORMATH_DEBUG

// ========================================================
// A 3x3 matrix in array-of-structures format
// ========================================================

VECTORMATH_ALIGNED_TYPE_PRE class Matrix3
{
    Vector3 mCol0;
    Vector3 mCol1;
    Vector3 mCol2;

public:

    // Default constructor; does no initialization
    inline Matrix3() { }

    // Copy a 3x3 matrix
    inline Matrix3(const Matrix3 & mat);

    // Construct a 3x3 matrix containing the specified columns
    inline Matrix3(const Vector3 & col0, const Vector3 & col1, const Vector3 & col2);

    // Construct a 3x3 rotation matrix from a unit-length quaternion
    explicit inline Matrix3(const Quat & unitQuat);

    // Set all elements of a 3x3 matrix to the same scalar value
    explicit inline Matrix3(float scalar);

    // Set all elements of a 3x3 matrix to the same scalar value (scalar data contained in vector data type)
    explicit inline Matrix3(const FloatInVec & scalar);

    // Assign one 3x3 matrix to another
    inline Matrix3 & operator = (const Matrix3 & mat);

    // Set column 0 of a 3x3 matrix
    inline Matrix3 & setCol0(const Vector3 & col0);

    // Set column 1 of a 3x3 matrix
    inline Matrix3 & setCol1(const Vector3 & col1);

    // Set column 2 of a 3x3 matrix
    inline Matrix3 & setCol2(const Vector3 & col2);

    // Get column 0 of a 3x3 matrix
    inline const Vector3 getCol0() const;

    // Get column 1 of a 3x3 matrix
    inline const Vector3 getCol1() const;

    // Get column 2 of a 3x3 matrix
    inline const Vector3 getCol2() const;

    // Set the column of a 3x3 matrix referred to by the specified index
    inline Matrix3 & setCol(int col, const Vector3 & vec);

    // Set the row of a 3x3 matrix referred to by the specified index
    inline Matrix3 & setRow(int row, const Vector3 & vec);

    // Get the column of a 3x3 matrix referred to by the specified index
    inline const Vector3 getCol(int col) const;

    // Get the row of a 3x3 matrix referred to by the specified index
    inline const Vector3 getRow(int row) const;

    // Subscripting operator to set or get a column
    inline Vector3 & operator[](int col);

    // Subscripting operator to get a column
    inline const Vector3 operator[](int col) const;

    // Set the element of a 3x3 matrix referred to by column and row indices
    inline Matrix3 & setElem(int col, int row, float val);

    // Set the element of a 3x3 matrix referred to by column and row indices (scalar data contained in vector data type)
    inline Matrix3 & setElem(int col, int row, const FloatInVec & val);

    // Get the element of a 3x3 matrix referred to by column and row indices
    inline const FloatInVec getElem(int col, int row) const;

    // Add two 3x3 matrices
    inline const Matrix3 operator + (const Matrix3 & mat) const;

    // Subtract a 3x3 matrix from another 3x3 matrix
    inline const Matrix3 operator - (const Matrix3 & mat) const;

    // Negate all elements of a 3x3 matrix
    inline const Matrix3 operator - () const;

    // Multiply a 3x3 matrix by a scalar
    inline const Matrix3 operator * (float scalar) const;

    // Multiply a 3x3 matrix by a scalar (scalar data contained in vector data type)
    inline const Matrix3 operator * (const FloatInVec & scalar) const;

    // Multiply a 3x3 matrix by a 3-D vector
    inline const Vector3 operator * (const Vector3 & vec) const;

    // Multiply two 3x3 matrices
    inline const Matrix3 operator * (const Matrix3 & mat) const;

    // Perform compound assignment and addition with a 3x3 matrix
    inline Matrix3 & operator += (const Matrix3 & mat);

    // Perform compound assignment and subtraction by a 3x3 matrix
    inline Matrix3 & operator -= (const Matrix3 & mat);

    // Perform compound assignment and multiplication by a scalar
    inline Matrix3 & operator *= (float scalar);

    // Perform compound assignment and multiplication by a scalar (scalar data contained in vector data type)
    inline Matrix3 & operator *= (const FloatInVec & scalar);

    // Perform compound assignment and multiplication by a 3x3 matrix
    inline Matrix3 & operator *= (const Matrix3 & mat);

    // Construct an identity 3x3 matrix
    static inline const Matrix3 identity();

    // Construct a 3x3 matrix to rotate around the x axis
    static inline const Matrix3 rotationX(float radians);

    // Construct a 3x3 matrix to rotate around the y axis
    static inline const Matrix3 rotationY(float radians);

    // Construct a 3x3 matrix to rotate around the z axis
    static inline const Matrix3 rotationZ(float radians);

    // Construct a 3x3 matrix to rotate around the x axis (scalar data contained in vector data type)
    static inline const Matrix3 rotationX(const FloatInVec & radians);

    // Construct a 3x3 matrix to rotate around the y axis (scalar data contained in vector data type)
    static inline const Matrix3 rotationY(const FloatInVec & radians);

    // Construct a 3x3 matrix to rotate around the z axis (scalar data contained in vector data type)
    static inline const Matrix3 rotationZ(const FloatInVec & radians);

    // Construct a 3x3 matrix to rotate around the x, y, and z axes
    static inline const Matrix3 rotationZYX(const Vector3 & radiansXYZ);

    // Construct a 3x3 matrix to rotate around a unit-length 3-D vector
    static inline const Matrix3 rotation(float radians, const Vector3 & unitVec);

    // Construct a 3x3 matrix to rotate around a unit-length 3-D vector (scalar data contained in vector data type)
    static inline const Matrix3 rotation(const FloatInVec & radians, const Vector3 & unitVec);

    // Construct a rotation matrix from a unit-length quaternion
    static inline const Matrix3 rotation(const Quat & unitQuat);

    // Construct a 3x3 matrix to perform scaling
    static inline const Matrix3 scale(const Vector3 & scaleVec);

} VECTORMATH_ALIGNED_TYPE_POST;

// Multiply a 3x3 matrix by a scalar
//
inline const Matrix3 operator * (float scalar, const Matrix3 & mat);

// Multiply a 3x3 matrix by a scalar (scalar data contained in vector data type)
//
inline const Matrix3 operator * (const FloatInVec & scalar, const Matrix3 & mat);

// Append (post-multiply) a scale transformation to a 3x3 matrix
// NOTE:
// Faster than creating and multiplying a scale transformation matrix.
//
inline const Matrix3 appendScale(const Matrix3 & mat, const Vector3 & scaleVec);

// Prepend (pre-multiply) a scale transformation to a 3x3 matrix
// NOTE:
// Faster than creating and multiplying a scale transformation matrix.
//
inline const Matrix3 prependScale(const Vector3 & scaleVec, const Matrix3 & mat);

// Multiply two 3x3 matrices per element
//
inline const Matrix3 mulPerElem(const Matrix3 & mat0, const Matrix3 & mat1);

// Compute the absolute value of a 3x3 matrix per element
//
inline const Matrix3 absPerElem(const Matrix3 & mat);

// Transpose of a 3x3 matrix
//
inline const Matrix3 transpose(const Matrix3 & mat);

// Compute the inverse of a 3x3 matrix
// NOTE:
// Result is unpredictable when the determinant of mat is equal to or near 0.
//
inline const Matrix3 inverse(const Matrix3 & mat);

// Determinant of a 3x3 matrix
//
inline const FloatInVec determinant(const Matrix3 & mat);

// Conditionally select between two 3x3 matrices
// NOTE:
// This function uses a conditional select instruction to avoid a branch.
// However, the transfer of select1 to a VMX register may use more processing time than a branch.
// Use the BoolInVec version for better performance.
//
inline const Matrix3 select(const Matrix3 & mat0, const Matrix3 & mat1, bool select1);

// Conditionally select between two 3x3 matrices (scalar data contained in vector data type)
// NOTE:
// This function uses a conditional select instruction to avoid a branch.
//
inline const Matrix3 select(const Matrix3 & mat0, const Matrix3 & mat1, const BoolInVec & select1);

#ifdef VECTORMATH_DEBUG

// Print a 3x3 matrix
// NOTE:
// Function is only defined when VECTORMATH_DEBUG is defined.
//
inline void print(const Matrix3 & mat);

// Print a 3x3 matrix and an associated string identifier
// NOTE:
// Function is only defined when VECTORMATH_DEBUG is defined.
//
inline void print(const Matrix3 & mat, const char * name);

#endif // VECTORMATH_DEBUG

// ========================================================
// A 4x4 matrix in array-of-structures format
// ========================================================

VECTORMATH_ALIGNED_TYPE_PRE class Matrix4
{
    Vector4 mCol0;
    Vector4 mCol1;
    Vector4 mCol2;
    Vector4 mCol3;

public:

    // Default constructor; does no initialization
    inline Matrix4() { }

    // Copy a 4x4 matrix
    inline Matrix4(const Matrix4 & mat);

    // Construct a 4x4 matrix containing the specified columns
    inline Matrix4(const Vector4 & col0, const Vector4 & col1, const Vector4 & col2, const Vector4 & col3);

    // Construct a 4x4 matrix from a 3x4 transformation matrix
    explicit inline Matrix4(const Transform3 & mat);

    // Construct a 4x4 matrix from a 3x3 matrix and a 3-D vector
    inline Matrix4(const Matrix3 & mat, const Vector3 & translateVec);

    // Construct a 4x4 matrix from a unit-length quaternion and a 3-D vector
    inline Matrix4(const Quat & unitQuat, const Vector3 & translateVec);

    // Set all elements of a 4x4 matrix to the same scalar value
    explicit inline Matrix4(float scalar);

    // Set all elements of a 4x4 matrix to the same scalar value (scalar data contained in vector data type)
    explicit inline Matrix4(const FloatInVec & scalar);

    // Assign one 4x4 matrix to another
    inline Matrix4 & operator = (const Matrix4 & mat);

    // Set the upper-left 3x3 submatrix
    // NOTE:
    // This function does not change the bottom row elements.
    inline Matrix4 & setUpper3x3(const Matrix3 & mat3);

    // Get the upper-left 3x3 submatrix of a 4x4 matrix
    inline const Matrix3 getUpper3x3() const;

    // Set translation component
    // NOTE:
    // This function does not change the bottom row elements.
    inline Matrix4 & setTranslation(const Vector3 & translateVec);

    // Get the translation component of a 4x4 matrix
    inline const Vector3 getTranslation() const;

    // Set column 0 of a 4x4 matrix
    inline Matrix4 & setCol0(const Vector4 & col0);

    // Set column 1 of a 4x4 matrix
    inline Matrix4 & setCol1(const Vector4 & col1);

    // Set column 2 of a 4x4 matrix
    inline Matrix4 & setCol2(const Vector4 & col2);

    // Set column 3 of a 4x4 matrix
    inline Matrix4 & setCol3(const Vector4 & col3);

    // Get column 0 of a 4x4 matrix
    inline const Vector4 getCol0() const;

    // Get column 1 of a 4x4 matrix
    inline const Vector4 getCol1() const;

    // Get column 2 of a 4x4 matrix
    inline const Vector4 getCol2() const;

    // Get column 3 of a 4x4 matrix
    inline const Vector4 getCol3() const;

    // Set the column of a 4x4 matrix referred to by the specified index
    inline Matrix4 & setCol(int col, const Vector4 & vec);

    // Set the row of a 4x4 matrix referred to by the specified index
    inline Matrix4 & setRow(int row, const Vector4 & vec);

    // Get the column of a 4x4 matrix referred to by the specified index
    inline const Vector4 getCol(int col) const;

    // Get the row of a 4x4 matrix referred to by the specified index
    inline const Vector4 getRow(int row) const;

    // Subscripting operator to set or get a column
    inline Vector4 & operator[](int col);

    // Subscripting operator to get a column
    inline const Vector4 operator[](int col) const;

    // Set the element of a 4x4 matrix referred to by column and row indices
    inline Matrix4 & setElem(int col, int row, float val);

    // Set the element of a 4x4 matrix referred to by column and row indices (scalar data contained in vector data type)
    inline Matrix4 & setElem(int col, int row, const FloatInVec & val);

    // Get the element of a 4x4 matrix referred to by column and row indices
    inline const FloatInVec getElem(int col, int row) const;

    // Add two 4x4 matrices
    inline const Matrix4 operator + (const Matrix4 & mat) const;

    // Subtract a 4x4 matrix from another 4x4 matrix
    inline const Matrix4 operator - (const Matrix4 & mat) const;

    // Negate all elements of a 4x4 matrix
    inline const Matrix4 operator - () const;

    // Multiply a 4x4 matrix by a scalar
    inline const Matrix4 operator * (float scalar) const;

    // Multiply a 4x4 matrix by a scalar (scalar data contained in vector data type)
    inline const Matrix4 operator * (const FloatInVec & scalar) const;

    // Multiply a 4x4 matrix by a 4-D vector
    inline const Vector4 operator * (const Vector4 & vec) const;

    // Multiply a 4x4 matrix by a 3-D vector
    inline const Vector4 operator * (const Vector3 & vec) const;

    // Multiply a 4x4 matrix by a 3-D point
    inline const Vector4 operator * (const Point3 & pnt) const;

    // Multiply two 4x4 matrices
    inline const Matrix4 operator * (const Matrix4 & mat) const;

    // Multiply a 4x4 matrix by a 3x4 transformation matrix
    inline const Matrix4 operator * (const Transform3 & tfrm) const;

    // Perform compound assignment and addition with a 4x4 matrix
    inline Matrix4 & operator += (const Matrix4 & mat);

    // Perform compound assignment and subtraction by a 4x4 matrix
    inline Matrix4 & operator -= (const Matrix4 & mat);

    // Perform compound assignment and multiplication by a scalar
    inline Matrix4 & operator *= (float scalar);

    // Perform compound assignment and multiplication by a scalar (scalar data contained in vector data type)
    inline Matrix4 & operator *= (const FloatInVec & scalar);

    // Perform compound assignment and multiplication by a 4x4 matrix
    inline Matrix4 & operator *= (const Matrix4 & mat);

    // Perform compound assignment and multiplication by a 3x4 transformation matrix
    inline Matrix4 & operator *= (const Transform3 & tfrm);

    // Construct an identity 4x4 matrix
    static inline const Matrix4 identity();

    // Construct a 4x4 matrix to rotate around the x axis
    static inline const Matrix4 rotationX(float radians);

    // Construct a 4x4 matrix to rotate around the y axis
    static inline const Matrix4 rotationY(float radians);

    // Construct a 4x4 matrix to rotate around the z axis
    static inline const Matrix4 rotationZ(float radians);

    // Construct a 4x4 matrix to rotate around the x axis (scalar data contained in vector data type)
    static inline const Matrix4 rotationX(const FloatInVec & radians);

    // Construct a 4x4 matrix to rotate around the y axis (scalar data contained in vector data type)
    static inline const Matrix4 rotationY(const FloatInVec & radians);

    // Construct a 4x4 matrix to rotate around the z axis (scalar data contained in vector data type)
    static inline const Matrix4 rotationZ(const FloatInVec & radians);

    // Construct a 4x4 matrix to rotate around the x, y, and z axes
    static inline const Matrix4 rotationZYX(const Vector3 & radiansXYZ);

    // Construct a 4x4 matrix to rotate around a unit-length 3-D vector
    static inline const Matrix4 rotation(float radians, const Vector3 & unitVec);

    // Construct a 4x4 matrix to rotate around a unit-length 3-D vector (scalar data contained in vector data type)
    static inline const Matrix4 rotation(const FloatInVec & radians, const Vector3 & unitVec);

    // Construct a rotation matrix from a unit-length quaternion
    static inline const Matrix4 rotation(const Quat & unitQuat);

    // Construct a 4x4 matrix to perform scaling
    static inline const Matrix4 scale(const Vector3 & scaleVec);

    // Construct a 4x4 matrix to perform translation
    static inline const Matrix4 translation(const Vector3 & translateVec);

    // Construct viewing matrix based on eye, position looked at, and up direction
    static inline const Matrix4 lookAt(const Point3 & eyePos, const Point3 & lookAtPos, const Vector3 & upVec);

    // Construct a perspective projection matrix
    static inline const Matrix4 perspective(float fovyRadians, float aspect, float zNear, float zFar);

    // Construct a perspective projection matrix based on frustum
    static inline const Matrix4 frustum(float left, float right, float bottom, float top, float zNear, float zFar);

    // Construct an orthographic projection matrix
    static inline const Matrix4 orthographic(float left, float right, float bottom, float top, float zNear, float zFar);

} VECTORMATH_ALIGNED_TYPE_POST;

// Multiply a 4x4 matrix by a scalar
//
inline const Matrix4 operator * (float scalar, const Matrix4 & mat);

// Multiply a 4x4 matrix by a scalar (scalar data contained in vector data type)
//
inline const Matrix4 operator * (const FloatInVec & scalar, const Matrix4 & mat);

// Append (post-multiply) a scale transformation to a 4x4 matrix
// NOTE:
// Faster than creating and multiplying a scale transformation matrix.
//
inline const Matrix4 appendScale(const Matrix4 & mat, const Vector3 & scaleVec);

// Prepend (pre-multiply) a scale transformation to a 4x4 matrix
// NOTE:
// Faster than creating and multiplying a scale transformation matrix.
//
inline const Matrix4 prependScale(const Vector3 & scaleVec, const Matrix4 & mat);

// Multiply two 4x4 matrices per element
//
inline const Matrix4 mulPerElem(const Matrix4 & mat0, const Matrix4 & mat1);

// Compute the absolute value of a 4x4 matrix per element
//
inline const Matrix4 absPerElem(const Matrix4 & mat);

// Transpose of a 4x4 matrix
//
inline const Matrix4 transpose(const Matrix4 & mat);

// Compute the inverse of a 4x4 matrix
// NOTE:
// Result is unpredictable when the determinant of mat is equal to or near 0.
//
inline const Matrix4 inverse(const Matrix4 & mat);

// Compute the inverse of a 4x4 matrix, which is expected to be an affine matrix
// NOTE:
// This can be used to achieve better performance than a general inverse when the specified 4x4 matrix meets the given restrictions.
// The result is unpredictable when the determinant of mat is equal to or near 0.
//
inline const Matrix4 affineInverse(const Matrix4 & mat);

// Compute the inverse of a 4x4 matrix, which is expected to be an affine matrix with an orthogonal upper-left 3x3 submatrix
// NOTE:
// This can be used to achieve better performance than a general inverse when the specified 4x4 matrix meets the given restrictions.
//
inline const Matrix4 orthoInverse(const Matrix4 & mat);

// Determinant of a 4x4 matrix
//
inline const FloatInVec determinant(const Matrix4 & mat);

// Conditionally select between two 4x4 matrices
// NOTE:
// This function uses a conditional select instruction to avoid a branch.
// However, the transfer of select1 to a VMX register may use more processing time than a branch.
// Use the BoolInVec version for better performance.
//
inline const Matrix4 select(const Matrix4 & mat0, const Matrix4 & mat1, bool select1);

// Conditionally select between two 4x4 matrices (scalar data contained in vector data type)
// NOTE:
// This function uses a conditional select instruction to avoid a branch.
//
inline const Matrix4 select(const Matrix4 & mat0, const Matrix4 & mat1, const BoolInVec & select1);

#ifdef VECTORMATH_DEBUG

// Print a 4x4 matrix
// NOTE:
// Function is only defined when VECTORMATH_DEBUG is defined.
//
inline void print(const Matrix4 & mat);

// Print a 4x4 matrix and an associated string identifier
// NOTE:
// Function is only defined when VECTORMATH_DEBUG is defined.
//
inline void print(const Matrix4 & mat, const char * name);

#endif // VECTORMATH_DEBUG

// ========================================================
// A 3x4 transformation matrix in array-of-structs format
// ========================================================

VECTORMATH_ALIGNED_TYPE_PRE class Transform3
{
    Vector3 mCol0;
    Vector3 mCol1;
    Vector3 mCol2;
    Vector3 mCol3;

public:

    // Default constructor; does no initialization
    inline Transform3() { }

    // Copy a 3x4 transformation matrix
    inline Transform3(const Transform3 & tfrm);

    // Construct a 3x4 transformation matrix containing the specified columns
    inline Transform3(const Vector3 & col0, const Vector3 & col1, const Vector3 & col2, const Vector3 & col3);

    // Construct a 3x4 transformation matrix from a 3x3 matrix and a 3-D vector
    inline Transform3(const Matrix3 & tfrm, const Vector3 & translateVec);

    // Construct a 3x4 transformation matrix from a unit-length quaternion and a 3-D vector
    inline Transform3(const Quat & unitQuat, const Vector3 & translateVec);

    // Set all elements of a 3x4 transformation matrix to the same scalar value
    explicit inline Transform3(float scalar);

    // Set all elements of a 3x4 transformation matrix to the same scalar value (scalar data contained in vector data type)
    explicit inline Transform3(const FloatInVec & scalar);

    // Assign one 3x4 transformation matrix to another
    inline Transform3 & operator = (const Transform3 & tfrm);

    // Set the upper-left 3x3 submatrix
    inline Transform3 & setUpper3x3(const Matrix3 & mat3);

    // Get the upper-left 3x3 submatrix of a 3x4 transformation matrix
    inline const Matrix3 getUpper3x3() const;

    // Set translation component
    inline Transform3 & setTranslation(const Vector3 & translateVec);

    // Get the translation component of a 3x4 transformation matrix
    inline const Vector3 getTranslation() const;

    // Set column 0 of a 3x4 transformation matrix
    inline Transform3 & setCol0(const Vector3 & col0);

    // Set column 1 of a 3x4 transformation matrix
    inline Transform3 & setCol1(const Vector3 & col1);

    // Set column 2 of a 3x4 transformation matrix
    inline Transform3 & setCol2(const Vector3 & col2);

    // Set column 3 of a 3x4 transformation matrix
    inline Transform3 & setCol3(const Vector3 & col3);

    // Get column 0 of a 3x4 transformation matrix
    inline const Vector3 getCol0() const;

    // Get column 1 of a 3x4 transformation matrix
    inline const Vector3 getCol1() const;

    // Get column 2 of a 3x4 transformation matrix
    inline const Vector3 getCol2() const;

    // Get column 3 of a 3x4 transformation matrix
    inline const Vector3 getCol3() const;

    // Set the column of a 3x4 transformation matrix referred to by the specified index
    inline Transform3 & setCol(int col, const Vector3 & vec);

    // Set the row of a 3x4 transformation matrix referred to by the specified index
    inline Transform3 & setRow(int row, const Vector4 & vec);

    // Get the column of a 3x4 transformation matrix referred to by the specified index
    inline const Vector3 getCol(int col) const;

    // Get the row of a 3x4 transformation matrix referred to by the specified index
    inline const Vector4 getRow(int row) const;

    // Subscripting operator to set or get a column
    inline Vector3 & operator[](int col);

    // Subscripting operator to get a column
    inline const Vector3 operator[](int col) const;

    // Set the element of a 3x4 transformation matrix referred to by column and row indices
    inline Transform3 & setElem(int col, int row, float val);

    // Set the element of a 3x4 transformation matrix referred to by column and row indices (scalar data contained in vector data type)
    inline Transform3 & setElem(int col, int row, const FloatInVec & val);

    // Get the element of a 3x4 transformation matrix referred to by column and row indices
    inline const FloatInVec getElem(int col, int row) const;

    // Multiply a 3x4 transformation matrix by a 3-D vector
    inline const Vector3 operator * (const Vector3 & vec) const;

    // Multiply a 3x4 transformation matrix by a 3-D point
    inline const Point3 operator * (const Point3 & pnt) const;

    // Multiply two 3x4 transformation matrices
    inline const Transform3 operator * (const Transform3 & tfrm) const;

    // Perform compound assignment and multiplication by a 3x4 transformation matrix
    inline Transform3 & operator *= (const Transform3 & tfrm);

    // Construct an identity 3x4 transformation matrix
    static inline const Transform3 identity();

    // Construct a 3x4 transformation matrix to rotate around the x axis
    static inline const Transform3 rotationX(float radians);

    // Construct a 3x4 transformation matrix to rotate around the y axis
    static inline const Transform3 rotationY(float radians);

    // Construct a 3x4 transformation matrix to rotate around the z axis
    static inline const Transform3 rotationZ(float radians);

    // Construct a 3x4 transformation matrix to rotate around the x axis (scalar data contained in vector data type)
    static inline const Transform3 rotationX(const FloatInVec & radians);

    // Construct a 3x4 transformation matrix to rotate around the y axis (scalar data contained in vector data type)
    static inline const Transform3 rotationY(const FloatInVec & radians);

    // Construct a 3x4 transformation matrix to rotate around the z axis (scalar data contained in vector data type)
    static inline const Transform3 rotationZ(const FloatInVec & radians);

    // Construct a 3x4 transformation matrix to rotate around the x, y, and z axes
    static inline const Transform3 rotationZYX(const Vector3 & radiansXYZ);

    // Construct a 3x4 transformation matrix to rotate around a unit-length 3-D vector
    static inline const Transform3 rotation(float radians, const Vector3 & unitVec);

    // Construct a 3x4 transformation matrix to rotate around a unit-length 3-D vector (scalar data contained in vector data type)
    static inline const Transform3 rotation(const FloatInVec & radians, const Vector3 & unitVec);

    // Construct a rotation matrix from a unit-length quaternion
    static inline const Transform3 rotation(const Quat & unitQuat);

    // Construct a 3x4 transformation matrix to perform scaling
    static inline const Transform3 scale(const Vector3 & scaleVec);

    // Construct a 3x4 transformation matrix to perform translation
    static inline const Transform3 translation(const Vector3 & translateVec);

} VECTORMATH_ALIGNED_TYPE_POST;

// Append (post-multiply) a scale transformation to a 3x4 transformation matrix
// NOTE:
// Faster than creating and multiplying a scale transformation matrix.
//
inline const Transform3 appendScale(const Transform3 & tfrm, const Vector3 & scaleVec);

// Prepend (pre-multiply) a scale transformation to a 3x4 transformation matrix
// NOTE:
// Faster than creating and multiplying a scale transformation matrix.
//
inline const Transform3 prependScale(const Vector3 & scaleVec, const Transform3 & tfrm);

// Multiply two 3x4 transformation matrices per element
//
inline const Transform3 mulPerElem(const Transform3 & tfrm0, const Transform3 & tfrm1);

// Compute the absolute value of a 3x4 transformation matrix per element
//
inline const Transform3 absPerElem(const Transform3 & tfrm);

// Inverse of a 3x4 transformation matrix
// NOTE:
// Result is unpredictable when the determinant of the left 3x3 submatrix is equal to or near 0.
//
inline const Transform3 inverse(const Transform3 & tfrm);

// Compute the inverse of a 3x4 transformation matrix, expected to have an orthogonal upper-left 3x3 submatrix
// NOTE:
// This can be used to achieve better performance than a general inverse when the specified 3x4 transformation matrix meets the given restrictions.
//
inline const Transform3 orthoInverse(const Transform3 & tfrm);

// Conditionally select between two 3x4 transformation matrices
// NOTE:
// This function uses a conditional select instruction to avoid a branch.
// However, the transfer of select1 to a VMX register may use more processing time than a branch.
// Use the BoolInVec version for better performance.
//
inline const Transform3 select(const Transform3 & tfrm0, const Transform3 & tfrm1, bool select1);

// Conditionally select between two 3x4 transformation matrices (scalar data contained in vector data type)
// NOTE:
// This function uses a conditional select instruction to avoid a branch.
//
inline const Transform3 select(const Transform3 & tfrm0, const Transform3 & tfrm1, const BoolInVec & select1);

#ifdef VECTORMATH_DEBUG

// Print a 3x4 transformation matrix
// NOTE:
// Function is only defined when VECTORMATH_DEBUG is defined.
//
inline void print(const Transform3 & tfrm);

// Print a 3x4 transformation matrix and an associated string identifier
// NOTE:
// Function is only defined when VECTORMATH_DEBUG is defined.
//
inline void print(const Transform3 & tfrm, const char * name);

#endif // VECTORMATH_DEBUG

} // namespace SSE
} // namespace Vectormath

// Inline implementations:

#ifndef VECTORMATH_SSE_VECTOR_HPP
#define VECTORMATH_SSE_VECTOR_HPP

namespace Vectormath
{
namespace SSE
{

// ========================================================
// VecIdx
// ========================================================

#ifdef VECTORMATH_NO_SCALAR_CAST
inline VecIdx::operator FloatInVec() const
{
    return FloatInVec(ref, i);
}
inline float VecIdx::getAsFloat() const
#else
inline VecIdx::operator float() const
#endif
{
    return ((float *)&ref)[i];
}

inline float VecIdx::operator = (float scalar)
{
    sseVecSetElement(ref, scalar, i);
    return scalar;
}

inline FloatInVec VecIdx::operator = (const FloatInVec & scalar)
{
    ref = sseVecInsert(ref, scalar.get128(), i);
    return scalar;
}

inline FloatInVec VecIdx::operator = (const VecIdx & scalar)
{
    return *this = FloatInVec(scalar.ref, scalar.i);
}

inline FloatInVec VecIdx::operator *= (float scalar)
{
    return *this *= FloatInVec(scalar);
}

inline FloatInVec VecIdx::operator *= (const FloatInVec & scalar)
{
    return *this = FloatInVec(ref, i) * scalar;
}

inline FloatInVec VecIdx::operator /= (float scalar)
{
    return *this /= FloatInVec(scalar);
}

inline FloatInVec VecIdx::operator /= (const FloatInVec & scalar)
{
    return *this = FloatInVec(ref, i) / scalar;
}

inline FloatInVec VecIdx::operator += (float scalar)
{
    return *this += FloatInVec(scalar);
}

inline FloatInVec VecIdx::operator += (const FloatInVec & scalar)
{
    return *this = FloatInVec(ref, i) + scalar;
}

inline FloatInVec VecIdx::operator -= (float scalar)
{
    return *this -= FloatInVec(scalar);
}

inline FloatInVec VecIdx::operator -= (const FloatInVec & scalar)
{
    return *this = FloatInVec(ref, i) - scalar;
}

// ========================================================
// Vector3
// ========================================================

inline Vector3::Vector3(float _x, float _y, float _z)
{
    mVec128 = _mm_setr_ps(_x, _y, _z, 0.0f);
}

inline Vector3::Vector3(const FloatInVec & _x, const FloatInVec & _y, const FloatInVec & _z)
{
    const __m128 xz = _mm_unpacklo_ps(_x.get128(), _z.get128());
    mVec128 = _mm_unpacklo_ps(xz, _y.get128());
}

inline Vector3::Vector3(const Point3 & pnt)
{
    mVec128 = pnt.get128();
}

inline Vector3::Vector3(float scalar)
{
    mVec128 = FloatInVec(scalar).get128();
}

inline Vector3::Vector3(const FloatInVec & scalar)
{
    mVec128 = scalar.get128();
}

inline Vector3::Vector3(__m128 vf4)
{
    mVec128 = vf4;
}

inline const Vector3 Vector3::xAxis()
{
    return Vector3(sseUnitVec1000());
}

inline const Vector3 Vector3::yAxis()
{
    return Vector3(sseUnitVec0100());
}

inline const Vector3 Vector3::zAxis()
{
    return Vector3(sseUnitVec0010());
}

inline const Vector3 lerp(float t, const Vector3 & vec0, const Vector3 & vec1)
{
    return lerp(FloatInVec(t), vec0, vec1);
}

inline const Vector3 lerp(const FloatInVec & t, const Vector3 & vec0, const Vector3 & vec1)
{
    return (vec0 + ((vec1 - vec0) * t));
}

inline const Vector3 slerp(float t, const Vector3 & unitVec0, const Vector3 & unitVec1)
{
    return slerp(FloatInVec(t), unitVec0, unitVec1);
}

inline const Vector3 slerp(const FloatInVec & t, const Vector3 & unitVec0, const Vector3 & unitVec1)
{
    __m128 scales, scale0, scale1, cosAngle, angle, tttt, oneMinusT, angles, sines;
    cosAngle = sseVecDot3(unitVec0.get128(), unitVec1.get128());
    __m128 selectMask = _mm_cmpgt_ps(_mm_set1_ps(VECTORMATH_SLERP_TOL), cosAngle);
    angle = sseACosf(cosAngle);
    tttt = t.get128();
    oneMinusT = _mm_sub_ps(_mm_set1_ps(1.0f), tttt);
    angles = _mm_unpacklo_ps(_mm_set1_ps(1.0f), tttt); // angles = 1, t, 1, t
    angles = _mm_unpacklo_ps(angles, oneMinusT);       // angles = 1, 1-t, t, 1-t
    angles = _mm_mul_ps(angles, angle);
    sines = sseSinf(angles);
    scales = _mm_div_ps(sines, sseSplat(sines, 0));
    scale0 = sseSelect(oneMinusT, sseSplat(scales, 1), selectMask);
    scale1 = sseSelect(tttt, sseSplat(scales, 2), selectMask);
    return Vector3(sseMAdd(unitVec0.get128(), scale0, _mm_mul_ps(unitVec1.get128(), scale1)));
}

inline __m128 Vector3::get128() const
{
    return mVec128;
}

inline void storeXYZ(const Vector3 & vec, __m128 * quad)
{
    __m128 dstVec = *quad;
    VECTORMATH_ALIGNED(unsigned int sw[4]) = { 0, 0, 0, 0xFFFFFFFF };
    dstVec = sseSelect(vec.get128(), dstVec, sw);
    *quad = dstVec;
}

inline void loadXYZArray(Vector3 & vec0, Vector3 & vec1, Vector3 & vec2, Vector3 & vec3, const __m128 * threeQuads)
{
    const float * quads = (const float *)threeQuads;
    vec0 = Vector3(_mm_load_ps(quads));
    vec1 = Vector3(_mm_loadu_ps(quads + 3));
    vec2 = Vector3(_mm_loadu_ps(quads + 6));
    vec3 = Vector3(_mm_loadu_ps(quads + 9));
}

inline void storeXYZArray(const Vector3 & vec0, const Vector3 & vec1, const Vector3 & vec2, const Vector3 & vec3, __m128 * threeQuads)
{
    __m128 xxxx = _mm_shuffle_ps(vec1.get128(), vec1.get128(), _MM_SHUFFLE(0, 0, 0, 0));
    __m128 zzzz = _mm_shuffle_ps(vec2.get128(), vec2.get128(), _MM_SHUFFLE(2, 2, 2, 2));
    VECTORMATH_ALIGNED(unsigned int xsw[4]) = { 0, 0, 0, 0xFFFFFFFF };
    VECTORMATH_ALIGNED(unsigned int zsw[4]) = { 0xFFFFFFFF, 0, 0, 0 };
    threeQuads[0] = sseSelect(vec0.get128(), xxxx, xsw);
    threeQuads[1] = _mm_shuffle_ps(vec1.get128(), vec2.get128(), _MM_SHUFFLE(1, 0, 2, 1));
    threeQuads[2] = sseSelect(_mm_shuffle_ps(vec3.get128(), vec3.get128(), _MM_SHUFFLE(2, 1, 0, 3)), zzzz, zsw);
}

inline Vector3 & Vector3::operator = (const Vector3 & vec)
{
    mVec128 = vec.mVec128;
    return *this;
}

inline Vector3 & Vector3::setX(float _x)
{
    sseVecSetElement(mVec128, _x, 0);
    return *this;
}

inline Vector3 & Vector3::setX(const FloatInVec & _x)
{
    mVec128 = sseVecInsert(mVec128, _x.get128(), 0);
    return *this;
}

inline const FloatInVec Vector3::getX() const
{
    return FloatInVec(mVec128, 0);
}

inline Vector3 & Vector3::setY(float _y)
{
    sseVecSetElement(mVec128, _y, 1);
    return *this;
}

inline Vector3 & Vector3::setY(const FloatInVec & _y)
{
    mVec128 = sseVecInsert(mVec128, _y.get128(), 1);
    return *this;
}

inline const FloatInVec Vector3::getY() const
{
    return FloatInVec(mVec128, 1);
}

inline Vector3 & Vector3::setZ(float _z)
{
    sseVecSetElement(mVec128, _z, 2);
    return *this;
}

inline Vector3 & Vector3::setZ(const FloatInVec & _z)
{
    mVec128 = sseVecInsert(mVec128, _z.get128(), 2);
    return *this;
}

inline const FloatInVec Vector3::getZ() const
{
    return FloatInVec(mVec128, 2);
}

inline Vector3 & Vector3::setW(float _w)
{
    sseVecSetElement(mVec128, _w, 3);
    return *this;
}

inline Vector3 & Vector3::setW(const FloatInVec & _w)
{
    mVec128 = sseVecInsert(mVec128, _w.get128(), 3);
    return *this;
}

inline const FloatInVec Vector3::getW() const
{
    return FloatInVec(mVec128, 3);
}

inline Vector3 & Vector3::setElem(int idx, float value)
{
    sseVecSetElement(mVec128, value, idx);
    return *this;
}

inline Vector3 & Vector3::setElem(int idx, const FloatInVec & value)
{
    mVec128 = sseVecInsert(mVec128, value.get128(), idx);
    return *this;
}

inline const FloatInVec Vector3::getElem(int idx) const
{
    return FloatInVec(mVec128, idx);
}

inline VecIdx Vector3::operator[](int idx)
{
    return VecIdx(mVec128, idx);
}

inline const FloatInVec Vector3::operator[](int idx) const
{
    return FloatInVec(mVec128, idx);
}

inline const Vector3 Vector3::operator + (const Vector3 & vec) const
{
    return Vector3(_mm_add_ps(mVec128, vec.mVec128));
}

inline const Vector3 Vector3::operator - (const Vector3 & vec) const
{
    return Vector3(_mm_sub_ps(mVec128, vec.mVec128));
}

inline const Point3 Vector3::operator + (const Point3 & pnt) const
{
    return Point3(_mm_add_ps(mVec128, pnt.get128()));
}

inline const Vector3 Vector3::operator * (float scalar) const
{
    return *this * FloatInVec(scalar);
}

inline const Vector3 Vector3::operator * (const FloatInVec & scalar) const
{
    return Vector3(_mm_mul_ps(mVec128, scalar.get128()));
}

inline Vector3 & Vector3::operator += (const Vector3 & vec)
{
    *this = *this + vec;
    return *this;
}

inline Vector3 & Vector3::operator -= (const Vector3 & vec)
{
    *this = *this - vec;
    return *this;
}

inline Vector3 & Vector3::operator *= (float scalar)
{
    *this = *this * scalar;
    return *this;
}

inline Vector3 & Vector3::operator *= (const FloatInVec & scalar)
{
    *this = *this * scalar;
    return *this;
}

inline const Vector3 Vector3::operator / (float scalar) const
{
    return *this / FloatInVec(scalar);
}

inline const Vector3 Vector3::operator / (const FloatInVec & scalar) const
{
    return Vector3(_mm_div_ps(mVec128, scalar.get128()));
}

inline Vector3 & Vector3::operator /= (float scalar)
{
    *this = *this / scalar;
    return *this;
}

inline Vector3 & Vector3::operator /= (const FloatInVec & scalar)
{
    *this = *this / scalar;
    return *this;
}

inline const Vector3 Vector3::operator - () const
{
    return Vector3(_mm_sub_ps(_mm_setzero_ps(), mVec128));
}

inline const Vector3 operator * (float scalar, const Vector3 & vec)
{
    return FloatInVec(scalar) * vec;
}

inline const Vector3 operator * (const FloatInVec & scalar, const Vector3 & vec)
{
    return vec * scalar;
}

inline const Vector3 mulPerElem(const Vector3 & vec0, const Vector3 & vec1)
{
    return Vector3(_mm_mul_ps(vec0.get128(), vec1.get128()));
}

inline const Vector3 divPerElem(const Vector3 & vec0, const Vector3 & vec1)
{
    return Vector3(_mm_div_ps(vec0.get128(), vec1.get128()));
}

inline const Vector3 recipPerElem(const Vector3 & vec)
{
    return Vector3(_mm_rcp_ps(vec.get128()));
}

inline const Vector3 absPerElem(const Vector3 & vec)
{
    return Vector3(sseFabsf(vec.get128()));
}

inline const Vector3 copySignPerElem(const Vector3 & vec0, const Vector3 & vec1)
{
    const __m128 vmask = sseUintToM128(0x7FFFFFFF);
    return Vector3(_mm_or_ps(
        _mm_and_ps(vmask, vec0.get128()),      // Value
        _mm_andnot_ps(vmask, vec1.get128()))); // Signs
}

inline const Vector3 maxPerElem(const Vector3 & vec0, const Vector3 & vec1)
{
    return Vector3(_mm_max_ps(vec0.get128(), vec1.get128()));
}

inline const FloatInVec maxElem(const Vector3 & vec)
{
    return FloatInVec(_mm_max_ps(_mm_max_ps(sseSplat(vec.get128(), 0), sseSplat(vec.get128(), 1)), sseSplat(vec.get128(), 2)));
}

inline const Vector3 minPerElem(const Vector3 & vec0, const Vector3 & vec1)
{
    return Vector3(_mm_min_ps(vec0.get128(), vec1.get128()));
}

inline const FloatInVec minElem(const Vector3 & vec)
{
    return FloatInVec(_mm_min_ps(_mm_min_ps(sseSplat(vec.get128(), 0), sseSplat(vec.get128(), 1)), sseSplat(vec.get128(), 2)));
}

inline const FloatInVec sum(const Vector3 & vec)
{
    return FloatInVec(_mm_add_ps(_mm_add_ps(sseSplat(vec.get128(), 0), sseSplat(vec.get128(), 1)), sseSplat(vec.get128(), 2)));
}

inline const FloatInVec dot(const Vector3 & vec0, const Vector3 & vec1)
{
    return FloatInVec(sseVecDot3(vec0.get128(), vec1.get128()), 0);
}

inline const FloatInVec lengthSqr(const Vector3 & vec)
{
    return FloatInVec(sseVecDot3(vec.get128(), vec.get128()), 0);
}

inline const FloatInVec length(const Vector3 & vec)
{
    return FloatInVec(_mm_sqrt_ps(sseVecDot3(vec.get128(), vec.get128())), 0);
}

inline const Vector3 normalizeApprox(const Vector3 & vec)
{
    return Vector3(_mm_mul_ps(vec.get128(), _mm_rsqrt_ps(sseVecDot3(vec.get128(), vec.get128()))));
}

inline const Vector3 normalize(const Vector3 & vec)
{
    return Vector3(_mm_mul_ps(vec.get128(), sseNewtonrapsonRSqrtf(sseVecDot3(vec.get128(), vec.get128()))));
}

inline const Vector3 cross(const Vector3 & vec0, const Vector3 & vec1)
{
    return Vector3(sseVecCross(vec0.get128(), vec1.get128()));
}

inline const Vector3 select(const Vector3 & vec0, const Vector3 & vec1, bool select1)
{
    return select(vec0, vec1, BoolInVec(select1));
}

inline const Vector3 select(const Vector3 & vec0, const Vector3 & vec1, const BoolInVec & select1)
{
    return Vector3(sseSelect(vec0.get128(), vec1.get128(), select1.get128()));
}

#ifdef VECTORMATH_DEBUG

inline void print(const Vector3 & vec)
{
    SSEFloat tmp;
    tmp.m128 = vec.get128();
    printf("( %f %f %f )\n", tmp.f[0], tmp.f[1], tmp.f[2]);
}

inline void print(const Vector3 & vec, const char * name)
{
    SSEFloat tmp;
    tmp.m128 = vec.get128();
    printf("%s: ( %f %f %f )\n", name, tmp.f[0], tmp.f[1], tmp.f[2]);
}

#endif // VECTORMATH_DEBUG

// ========================================================
// Vector4
// ========================================================

inline Vector4::Vector4(float _x, float _y, float _z, float _w)
{
    mVec128 = _mm_setr_ps(_x, _y, _z, _w);
}

inline Vector4::Vector4(const FloatInVec & _x, const FloatInVec & _y, const FloatInVec & _z, const FloatInVec & _w)
{
    mVec128 = _mm_unpacklo_ps(
        _mm_unpacklo_ps(_x.get128(), _z.get128()),
        _mm_unpacklo_ps(_y.get128(), _w.get128()));
}

inline Vector4::Vector4(const Vector3 & xyz, float _w)
{
    mVec128 = xyz.get128();
    sseVecSetElement(mVec128, _w, 3);
}

inline Vector4::Vector4(const Vector3 & xyz, const FloatInVec & _w)
{
    mVec128 = xyz.get128();
    mVec128 = sseVecInsert(mVec128, _w.get128(), 3);
}

inline Vector4::Vector4(const Vector3 & vec)
{
    mVec128 = vec.get128();
    mVec128 = sseVecInsert(mVec128, _mm_setzero_ps(), 3);
}

inline Vector4::Vector4(const Point3 & pnt)
{
    mVec128 = pnt.get128();
    mVec128 = sseVecInsert(mVec128, _mm_set1_ps(1.0f), 3);
}

inline Vector4::Vector4(const Quat & quat)
{
    mVec128 = quat.get128();
}

inline Vector4::Vector4(float scalar)
{
    mVec128 = FloatInVec(scalar).get128();
}

inline Vector4::Vector4(const FloatInVec & scalar)
{
    mVec128 = scalar.get128();
}

inline Vector4::Vector4(__m128 vf4)
{
    mVec128 = vf4;
}

inline const Vector4 Vector4::xAxis()
{
    return Vector4(sseUnitVec1000());
}

inline const Vector4 Vector4::yAxis()
{
    return Vector4(sseUnitVec0100());
}

inline const Vector4 Vector4::zAxis()
{
    return Vector4(sseUnitVec0010());
}

inline const Vector4 Vector4::wAxis()
{
    return Vector4(sseUnitVec0001());
}

inline const Vector4 lerp(float t, const Vector4 & vec0, const Vector4 & vec1)
{
    return lerp(FloatInVec(t), vec0, vec1);
}

inline const Vector4 lerp(const FloatInVec & t, const Vector4 & vec0, const Vector4 & vec1)
{
    return (vec0 + ((vec1 - vec0) * t));
}

inline const Vector4 slerp(float t, const Vector4 & unitVec0, const Vector4 & unitVec1)
{
    return slerp(FloatInVec(t), unitVec0, unitVec1);
}

inline const Vector4 slerp(const FloatInVec & t, const Vector4 & unitVec0, const Vector4 & unitVec1)
{
    __m128 scales, scale0, scale1, cosAngle, angle, tttt, oneMinusT, angles, sines;
    cosAngle = sseVecDot4(unitVec0.get128(), unitVec1.get128());
    __m128 selectMask = _mm_cmpgt_ps(_mm_set1_ps(VECTORMATH_SLERP_TOL), cosAngle);
    angle = sseACosf(cosAngle);
    tttt = t.get128();
    oneMinusT = _mm_sub_ps(_mm_set1_ps(1.0f), tttt);
    angles = _mm_unpacklo_ps(_mm_set1_ps(1.0f), tttt); // angles = 1, t, 1, t
    angles = _mm_unpacklo_ps(angles, oneMinusT);       // angles = 1, 1-t, t, 1-t
    angles = _mm_mul_ps(angles, angle);
    sines = sseSinf(angles);
    scales = _mm_div_ps(sines, sseSplat(sines, 0));
    scale0 = sseSelect(oneMinusT, sseSplat(scales, 1), selectMask);
    scale1 = sseSelect(tttt, sseSplat(scales, 2), selectMask);
    return Vector4(sseMAdd(unitVec0.get128(), scale0, _mm_mul_ps(unitVec1.get128(), scale1)));
}

inline __m128 Vector4::get128() const
{
    return mVec128;
}

inline Vector4 & Vector4::operator = (const Vector4 & vec)
{
    mVec128 = vec.mVec128;
    return *this;
}

inline Vector4 & Vector4::setXYZ(const Vector3 & vec)
{
    VECTORMATH_ALIGNED(unsigned int sw[4]) = { 0, 0, 0, 0xFFFFFFFF };
    mVec128 = sseSelect(vec.get128(), mVec128, sw);
    return *this;
}

inline const Vector3 Vector4::getXYZ() const
{
    return Vector3(mVec128);
}

inline Vector4 & Vector4::setX(float _x)
{
    sseVecSetElement(mVec128, _x, 0);
    return *this;
}

inline Vector4 & Vector4::setX(const FloatInVec & _x)
{
    mVec128 = sseVecInsert(mVec128, _x.get128(), 0);
    return *this;
}

inline const FloatInVec Vector4::getX() const
{
    return FloatInVec(mVec128, 0);
}

inline Vector4 & Vector4::setY(float _y)
{
    sseVecSetElement(mVec128, _y, 1);
    return *this;
}

inline Vector4 & Vector4::setY(const FloatInVec & _y)
{
    mVec128 = sseVecInsert(mVec128, _y.get128(), 1);
    return *this;
}

inline const FloatInVec Vector4::getY() const
{
    return FloatInVec(mVec128, 1);
}

inline Vector4 & Vector4::setZ(float _z)
{
    sseVecSetElement(mVec128, _z, 2);
    return *this;
}

inline Vector4 & Vector4::setZ(const FloatInVec & _z)
{
    mVec128 = sseVecInsert(mVec128, _z.get128(), 2);
    return *this;
}

inline const FloatInVec Vector4::getZ() const
{
    return FloatInVec(mVec128, 2);
}

inline Vector4 & Vector4::setW(float _w)
{
    sseVecSetElement(mVec128, _w, 3);
    return *this;
}

inline Vector4 & Vector4::setW(const FloatInVec & _w)
{
    mVec128 = sseVecInsert(mVec128, _w.get128(), 3);
    return *this;
}

inline const FloatInVec Vector4::getW() const
{
    return FloatInVec(mVec128, 3);
}

inline Vector4 & Vector4::setElem(int idx, float value)
{
    sseVecSetElement(mVec128, value, idx);
    return *this;
}

inline Vector4 & Vector4::setElem(int idx, const FloatInVec & value)
{
    mVec128 = sseVecInsert(mVec128, value.get128(), idx);
    return *this;
}

inline const FloatInVec Vector4::getElem(int idx) const
{
    return FloatInVec(mVec128, idx);
}

inline VecIdx Vector4::operator[](int idx)
{
    return VecIdx(mVec128, idx);
}

inline const FloatInVec Vector4::operator[](int idx) const
{
    return FloatInVec(mVec128, idx);
}

inline const Vector4 Vector4::operator + (const Vector4 & vec) const
{
    return Vector4(_mm_add_ps(mVec128, vec.mVec128));
}

inline const Vector4 Vector4::operator - (const Vector4 & vec) const
{
    return Vector4(_mm_sub_ps(mVec128, vec.mVec128));
}

inline const Vector4 Vector4::operator * (float scalar) const
{
    return *this * FloatInVec(scalar);
}

inline const Vector4 Vector4::operator * (const FloatInVec & scalar) const
{
    return Vector4(_mm_mul_ps(mVec128, scalar.get128()));
}

inline Vector4 & Vector4::operator += (const Vector4 & vec)
{
    *this = *this + vec;
    return *this;
}

inline Vector4 & Vector4::operator -= (const Vector4 & vec)
{
    *this = *this - vec;
    return *this;
}

inline Vector4 & Vector4::operator *= (float scalar)
{
    *this = *this * scalar;
    return *this;
}

inline Vector4 & Vector4::operator *= (const FloatInVec & scalar)
{
    *this = *this * scalar;
    return *this;
}

inline const Vector4 Vector4::operator / (float scalar) const
{
    return *this / FloatInVec(scalar);
}

inline const Vector4 Vector4::operator / (const FloatInVec & scalar) const
{
    return Vector4(_mm_div_ps(mVec128, scalar.get128()));
}

inline Vector4 & Vector4::operator /= (float scalar)
{
    *this = *this / scalar;
    return *this;
}

inline Vector4 & Vector4::operator /= (const FloatInVec & scalar)
{
    *this = *this / scalar;
    return *this;
}

inline const Vector4 Vector4::operator - () const
{
    return Vector4(_mm_sub_ps(_mm_setzero_ps(), mVec128));
}

inline const Vector4 operator * (float scalar, const Vector4 & vec)
{
    return FloatInVec(scalar) * vec;
}

inline const Vector4 operator * (const FloatInVec & scalar, const Vector4 & vec)
{
    return vec * scalar;
}

inline const Vector4 mulPerElem(const Vector4 & vec0, const Vector4 & vec1)
{
    return Vector4(_mm_mul_ps(vec0.get128(), vec1.get128()));
}

inline const Vector4 divPerElem(const Vector4 & vec0, const Vector4 & vec1)
{
    return Vector4(_mm_div_ps(vec0.get128(), vec1.get128()));
}

inline const Vector4 recipPerElem(const Vector4 & vec)
{
    return Vector4(_mm_rcp_ps(vec.get128()));
}

inline const Vector4 absPerElem(const Vector4 & vec)
{
    return Vector4(sseFabsf(vec.get128()));
}

inline const Vector4 copySignPerElem(const Vector4 & vec0, const Vector4 & vec1)
{
    const __m128 vmask = sseUintToM128(0x7FFFFFFF);
    return Vector4(_mm_or_ps(
        _mm_and_ps(vmask, vec0.get128()),      // Value
        _mm_andnot_ps(vmask, vec1.get128()))); // Signs
}

inline const Vector4 maxPerElem(const Vector4 & vec0, const Vector4 & vec1)
{
    return Vector4(_mm_max_ps(vec0.get128(), vec1.get128()));
}

inline const FloatInVec maxElem(const Vector4 & vec)
{
    return FloatInVec(_mm_max_ps(
        _mm_max_ps(sseSplat(vec.get128(), 0), sseSplat(vec.get128(), 1)),
        _mm_max_ps(sseSplat(vec.get128(), 2), sseSplat(vec.get128(), 3))));
}

inline const Vector4 minPerElem(const Vector4 & vec0, const Vector4 & vec1)
{
    return Vector4(_mm_min_ps(vec0.get128(), vec1.get128()));
}

inline const FloatInVec minElem(const Vector4 & vec)
{
    return FloatInVec(_mm_min_ps(
        _mm_min_ps(sseSplat(vec.get128(), 0), sseSplat(vec.get128(), 1)),
        _mm_min_ps(sseSplat(vec.get128(), 2), sseSplat(vec.get128(), 3))));
}

inline const FloatInVec sum(const Vector4 & vec)
{
    return FloatInVec(_mm_add_ps(
        _mm_add_ps(sseSplat(vec.get128(), 0), sseSplat(vec.get128(), 1)),
        _mm_add_ps(sseSplat(vec.get128(), 2), sseSplat(vec.get128(), 3))));
}

inline const FloatInVec dot(const Vector4 & vec0, const Vector4 & vec1)
{
    return FloatInVec(sseVecDot4(vec0.get128(), vec1.get128()), 0);
}

inline const FloatInVec lengthSqr(const Vector4 & vec)
{
    return FloatInVec(sseVecDot4(vec.get128(), vec.get128()), 0);
}

inline const FloatInVec length(const Vector4 & vec)
{
    return FloatInVec(_mm_sqrt_ps(sseVecDot4(vec.get128(), vec.get128())), 0);
}

inline const Vector4 normalizeApprox(const Vector4 & vec)
{
    return Vector4(_mm_mul_ps(vec.get128(), _mm_rsqrt_ps(sseVecDot4(vec.get128(), vec.get128()))));
}

inline const Vector4 normalize(const Vector4 & vec)
{
    return Vector4(_mm_mul_ps(vec.get128(), sseNewtonrapsonRSqrtf(sseVecDot4(vec.get128(), vec.get128()))));
}

inline const Vector4 select(const Vector4 & vec0, const Vector4 & vec1, bool select1)
{
    return select(vec0, vec1, BoolInVec(select1));
}

inline const Vector4 select(const Vector4 & vec0, const Vector4 & vec1, const BoolInVec & select1)
{
    return Vector4(sseSelect(vec0.get128(), vec1.get128(), select1.get128()));
}

#ifdef VECTORMATH_DEBUG

inline void print(const Vector4 & vec)
{
    SSEFloat tmp;
    tmp.m128 = vec.get128();
    printf("( %f %f %f %f )\n", tmp.f[0], tmp.f[1], tmp.f[2], tmp.f[3]);
}

inline void print(const Vector4 & vec, const char * name)
{
    SSEFloat tmp;
    tmp.m128 = vec.get128();
    printf("%s: ( %f %f %f %f )\n", name, tmp.f[0], tmp.f[1], tmp.f[2], tmp.f[3]);
}

#endif // VECTORMATH_DEBUG

// ========================================================
// Point3
// ========================================================

inline Point3::Point3(float _x, float _y, float _z)
{
    mVec128 = _mm_setr_ps(_x, _y, _z, 0.0f);
}

inline Point3::Point3(const FloatInVec & _x, const FloatInVec & _y, const FloatInVec & _z)
{
    mVec128 = _mm_unpacklo_ps(_mm_unpacklo_ps(_x.get128(), _z.get128()), _y.get128());
}

inline Point3::Point3(const Vector3 & vec)
{
    mVec128 = vec.get128();
}

inline Point3::Point3(float scalar)
{
    mVec128 = FloatInVec(scalar).get128();
}

inline Point3::Point3(const FloatInVec & scalar)
{
    mVec128 = scalar.get128();
}

inline Point3::Point3(__m128 vf4)
{
    mVec128 = vf4;
}

inline const Point3 lerp(float t, const Point3 & pnt0, const Point3 & pnt1)
{
    return lerp(FloatInVec(t), pnt0, pnt1);
}

inline const Point3 lerp(const FloatInVec & t, const Point3 & pnt0, const Point3 & pnt1)
{
    return (pnt0 + ((pnt1 - pnt0) * t));
}

inline __m128 Point3::get128() const
{
    return mVec128;
}

inline void storeXYZ(const Point3 & pnt, __m128 * quad)
{
    __m128 dstVec = *quad;
    VECTORMATH_ALIGNED(unsigned int sw[4]) = { 0, 0, 0, 0xFFFFFFFF };
    dstVec = sseSelect(pnt.get128(), dstVec, sw);
    *quad = dstVec;
}

inline void loadXYZArray(Point3 & pnt0, Point3 & pnt1, Point3 & pnt2, Point3 & pnt3, const __m128 * threeQuads)
{
    const float * quads = (const float *)threeQuads;
    pnt0 = Point3(_mm_load_ps(quads));
    pnt1 = Point3(_mm_loadu_ps(quads + 3));
    pnt2 = Point3(_mm_loadu_ps(quads + 6));
    pnt3 = Point3(_mm_loadu_ps(quads + 9));
}

inline void storeXYZArray(const Point3 & pnt0, const Point3 & pnt1, const Point3 & pnt2, const Point3 & pnt3, __m128 * threeQuads)
{
    __m128 xxxx = _mm_shuffle_ps(pnt1.get128(), pnt1.get128(), _MM_SHUFFLE(0, 0, 0, 0));
    __m128 zzzz = _mm_shuffle_ps(pnt2.get128(), pnt2.get128(), _MM_SHUFFLE(2, 2, 2, 2));
    VECTORMATH_ALIGNED(unsigned int xsw[4]) = { 0, 0, 0, 0xFFFFFFFF };
    VECTORMATH_ALIGNED(unsigned int zsw[4]) = { 0xFFFFFFFF, 0, 0, 0 };
    threeQuads[0] = sseSelect(pnt0.get128(), xxxx, xsw);
    threeQuads[1] = _mm_shuffle_ps(pnt1.get128(), pnt2.get128(), _MM_SHUFFLE(1, 0, 2, 1));
    threeQuads[2] = sseSelect(_mm_shuffle_ps(pnt3.get128(), pnt3.get128(), _MM_SHUFFLE(2, 1, 0, 3)), zzzz, zsw);
}

inline Point3 & Point3::operator = (const Point3 & pnt)
{
    mVec128 = pnt.mVec128;
    return *this;
}

inline Point3 & Point3::setX(float _x)
{
    sseVecSetElement(mVec128, _x, 0);
    return *this;
}

inline Point3 & Point3::setX(const FloatInVec & _x)
{
    mVec128 = sseVecInsert(mVec128, _x.get128(), 0);
    return *this;
}

inline const FloatInVec Point3::getX() const
{
    return FloatInVec(mVec128, 0);
}

inline Point3 & Point3::setY(float _y)
{
    sseVecSetElement(mVec128, _y, 1);
    return *this;
}

inline Point3 & Point3::setY(const FloatInVec & _y)
{
    mVec128 = sseVecInsert(mVec128, _y.get128(), 1);
    return *this;
}

inline const FloatInVec Point3::getY() const
{
    return FloatInVec(mVec128, 1);
}

inline Point3 & Point3::setZ(float _z)
{
    sseVecSetElement(mVec128, _z, 2);
    return *this;
}

inline Point3 & Point3::setZ(const FloatInVec & _z)
{
    mVec128 = sseVecInsert(mVec128, _z.get128(), 2);
    return *this;
}

inline const FloatInVec Point3::getZ() const
{
    return FloatInVec(mVec128, 2);
}

inline Point3 & Point3::setW(float _w)
{
    sseVecSetElement(mVec128, _w, 3);
    return *this;
}

inline Point3 & Point3::setW(const FloatInVec & _w)
{
    mVec128 = sseVecInsert(mVec128, _w.get128(), 3);
    return *this;
}

inline const FloatInVec Point3::getW() const
{
    return FloatInVec(mVec128, 3);
}

inline Point3 & Point3::setElem(int idx, float value)
{
    sseVecSetElement(mVec128, value, idx);
    return *this;
}

inline Point3 & Point3::setElem(int idx, const FloatInVec & value)
{
    mVec128 = sseVecInsert(mVec128, value.get128(), idx);
    return *this;
}

inline const FloatInVec Point3::getElem(int idx) const
{
    return FloatInVec(mVec128, idx);
}

inline VecIdx Point3::operator[](int idx)
{
    return VecIdx(mVec128, idx);
}

inline const FloatInVec Point3::operator[](int idx) const
{
    return FloatInVec(mVec128, idx);
}

inline const Vector3 Point3::operator - (const Point3 & pnt) const
{
    return Vector3(_mm_sub_ps(mVec128, pnt.mVec128));
}

inline const Point3 Point3::operator + (const Vector3 & vec) const
{
    return Point3(_mm_add_ps(mVec128, vec.get128()));
}

inline const Point3 Point3::operator - (const Vector3 & vec) const
{
    return Point3(_mm_sub_ps(mVec128, vec.get128()));
}

inline Point3 & Point3::operator += (const Vector3 & vec)
{
    *this = *this + vec;
    return *this;
}

inline Point3 & Point3::operator -= (const Vector3 & vec)
{
    *this = *this - vec;
    return *this;
}

inline const Point3 mulPerElem(const Point3 & pnt0, const Point3 & pnt1)
{
    return Point3(_mm_mul_ps(pnt0.get128(), pnt1.get128()));
}

inline const Point3 divPerElem(const Point3 & pnt0, const Point3 & pnt1)
{
    return Point3(_mm_div_ps(pnt0.get128(), pnt1.get128()));
}

inline const Point3 recipPerElem(const Point3 & pnt)
{
    return Point3(_mm_rcp_ps(pnt.get128()));
}

inline const Point3 absPerElem(const Point3 & pnt)
{
    return Point3(sseFabsf(pnt.get128()));
}

inline const Point3 copySignPerElem(const Point3 & pnt0, const Point3 & pnt1)
{
    const __m128 vmask = sseUintToM128(0x7FFFFFFF);
    return Point3(_mm_or_ps(
        _mm_and_ps(vmask, pnt0.get128()),      // Value
        _mm_andnot_ps(vmask, pnt1.get128()))); // Signs
}

inline const Point3 maxPerElem(const Point3 & pnt0, const Point3 & pnt1)
{
    return Point3(_mm_max_ps(pnt0.get128(), pnt1.get128()));
}

inline const FloatInVec maxElem(const Point3 & pnt)
{
    return FloatInVec(_mm_max_ps(_mm_max_ps(sseSplat(pnt.get128(), 0), sseSplat(pnt.get128(), 1)), sseSplat(pnt.get128(), 2)));
}

inline const Point3 minPerElem(const Point3 & pnt0, const Point3 & pnt1)
{
    return Point3(_mm_min_ps(pnt0.get128(), pnt1.get128()));
}

inline const FloatInVec minElem(const Point3 & pnt)
{
    return FloatInVec(_mm_min_ps(_mm_min_ps(sseSplat(pnt.get128(), 0), sseSplat(pnt.get128(), 1)), sseSplat(pnt.get128(), 2)));
}

inline const FloatInVec sum(const Point3 & pnt)
{
    return FloatInVec(_mm_add_ps(_mm_add_ps(sseSplat(pnt.get128(), 0), sseSplat(pnt.get128(), 1)), sseSplat(pnt.get128(), 2)));
}

inline const Point3 scale(const Point3 & pnt, float scaleVal)
{
    return scale(pnt, FloatInVec(scaleVal));
}

inline const Point3 scale(const Point3 & pnt, const FloatInVec & scaleVal)
{
    return mulPerElem(pnt, Point3(scaleVal));
}

inline const Point3 scale(const Point3 & pnt, const Vector3 & scaleVec)
{
    return mulPerElem(pnt, Point3(scaleVec));
}

inline const FloatInVec projection(const Point3 & pnt, const Vector3 & unitVec)
{
    return FloatInVec(sseVecDot3(pnt.get128(), unitVec.get128()), 0);
}

inline const FloatInVec distSqrFromOrigin(const Point3 & pnt)
{
    return lengthSqr(Vector3(pnt));
}

inline const FloatInVec distFromOrigin(const Point3 & pnt)
{
    return length(Vector3(pnt));
}

inline const FloatInVec distSqr(const Point3 & pnt0, const Point3 & pnt1)
{
    return lengthSqr((pnt1 - pnt0));
}

inline const FloatInVec distp(const Point3 & pnt0, const Point3 & pnt1)
{
    return length((pnt1 - pnt0));
}

inline const Point3 select(const Point3 & pnt0, const Point3 & pnt1, bool select1)
{
    return select(pnt0, pnt1, BoolInVec(select1));
}

inline const Point3 select(const Point3 & pnt0, const Point3 & pnt1, const BoolInVec & select1)
{
    return Point3(sseSelect(pnt0.get128(), pnt1.get128(), select1.get128()));
}

#ifdef VECTORMATH_DEBUG

inline void print(const Point3 & pnt)
{
    SSEFloat tmp;
    tmp.m128 = pnt.get128();
    printf("( %f %f %f )\n", tmp.f[0], tmp.f[1], tmp.f[2]);
}

inline void print(const Point3 & pnt, const char * name)
{
    SSEFloat tmp;
    tmp.m128 = pnt.get128();
    printf("%s: ( %f %f %f )\n", name, tmp.f[0], tmp.f[1], tmp.f[2]);
}

#endif // VECTORMATH_DEBUG

} // namespace SSE
} // namespace Vectormath

#endif // VECTORMATH_SSE_VECTOR_HPP

#ifndef VECTORMATH_SSE_QUATERNION_HPP
#define VECTORMATH_SSE_QUATERNION_HPP

namespace Vectormath
{
namespace SSE
{

// ========================================================
// Quat
// ========================================================

inline Quat::Quat(float _x, float _y, float _z, float _w)
{
    mVec128 = _mm_setr_ps(_x, _y, _z, _w);
}

inline Quat::Quat(const FloatInVec & _x, const FloatInVec & _y, const FloatInVec & _z, const FloatInVec & _w)
{
    mVec128 = _mm_unpacklo_ps(
        _mm_unpacklo_ps(_x.get128(), _z.get128()),
        _mm_unpacklo_ps(_y.get128(), _w.get128()));
}

inline Quat::Quat(const Vector3 & xyz, float _w)
{
    mVec128 = xyz.get128();
    sseVecSetElement(mVec128, _w, 3);
}

inline Quat::Quat(const Vector3 & xyz, const FloatInVec & _w)
{
    mVec128 = xyz.get128();
    mVec128 = sseVecInsert(mVec128, _w.get128(), 3);
}

inline Quat::Quat(const Vector4 & vec)
{
    mVec128 = vec.get128();
}

inline Quat::Quat(float scalar)
{
    mVec128 = FloatInVec(scalar).get128();
}

inline Quat::Quat(const FloatInVec & scalar)
{
    mVec128 = scalar.get128();
}

inline Quat::Quat(__m128 vf4)
{
    mVec128 = vf4;
}

inline const Quat Quat::identity()
{
    return Quat(sseUnitVec0001());
}

inline const Quat lerp(float t, const Quat & quat0, const Quat & quat1)
{
    return lerp(FloatInVec(t), quat0, quat1);
}

inline const Quat lerp(const FloatInVec & t, const Quat & quat0, const Quat & quat1)
{
    return (quat0 + ((quat1 - quat0) * t));
}

inline const Quat slerp(float t, const Quat & unitQuat0, const Quat & unitQuat1)
{
    return slerp(FloatInVec(t), unitQuat0, unitQuat1);
}

inline const Quat slerp(const FloatInVec & t, const Quat & unitQuat0, const Quat & unitQuat1)
{
    Quat start;
    SSEFloat4V scales, scale0, scale1, cosAngle, angle, tttt, oneMinusT, angles, sines;
    SSEUint4V selectMask;
    cosAngle = sseVecDot4(unitQuat0.get128(), unitQuat1.get128());
    selectMask = (SSEUint4V)_mm_cmpgt_ps(_mm_setzero_ps(), cosAngle);
    cosAngle = sseSelect(cosAngle, sseNegatef(cosAngle), selectMask);
    start = Quat(sseSelect(unitQuat0.get128(), sseNegatef(unitQuat0.get128()), selectMask));
    selectMask = (SSEUint4V)_mm_cmpgt_ps(_mm_set1_ps(VECTORMATH_SLERP_TOL), cosAngle);
    angle = sseACosf(cosAngle);
    tttt = t.get128();
    oneMinusT = _mm_sub_ps(_mm_set1_ps(1.0f), tttt);
    angles = sseMergeH(_mm_set1_ps(1.0f), tttt);
    angles = sseMergeH(angles, oneMinusT);
    angles = sseMAdd(angles, angle, _mm_setzero_ps());
    sines = sseSinf(angles);
    scales = _mm_div_ps(sines, sseSplat(sines, 0));
    scale0 = sseSelect(oneMinusT, sseSplat(scales, 1), selectMask);
    scale1 = sseSelect(tttt, sseSplat(scales, 2), selectMask);
    return Quat(sseMAdd(start.get128(), scale0, _mm_mul_ps(unitQuat1.get128(), scale1)));
}

inline const Quat squad(float t, const Quat & unitQuat0, const Quat & unitQuat1, const Quat & unitQuat2, const Quat & unitQuat3)
{
    return squad(FloatInVec(t), unitQuat0, unitQuat1, unitQuat2, unitQuat3);
}

inline const Quat squad(const FloatInVec & t, const Quat & unitQuat0, const Quat & unitQuat1, const Quat & unitQuat2, const Quat & unitQuat3)
{
    return slerp(((FloatInVec(2.0f) * t) * (FloatInVec(1.0f) - t)), slerp(t, unitQuat0, unitQuat3), slerp(t, unitQuat1, unitQuat2));
}

inline __m128 Quat::get128() const
{
    return mVec128;
}

inline Quat & Quat::operator = (const Quat & quat)
{
    mVec128 = quat.mVec128;
    return *this;
}

inline Quat & Quat::setXYZ(const Vector3 & vec)
{
    VECTORMATH_ALIGNED(unsigned int sw[4]) = { 0, 0, 0, 0xFFFFFFFF };
    mVec128 = sseSelect(vec.get128(), mVec128, sw);
    return *this;
}

inline const Vector3 Quat::getXYZ() const
{
    return Vector3(mVec128);
}

inline Quat & Quat::setX(float _x)
{
    sseVecSetElement(mVec128, _x, 0);
    return *this;
}

inline Quat & Quat::setX(const FloatInVec & _x)
{
    mVec128 = sseVecInsert(mVec128, _x.get128(), 0);
    return *this;
}

inline const FloatInVec Quat::getX() const
{
    return FloatInVec(mVec128, 0);
}

inline Quat & Quat::setY(float _y)
{
    sseVecSetElement(mVec128, _y, 1);
    return *this;
}

inline Quat & Quat::setY(const FloatInVec & _y)
{
    mVec128 = sseVecInsert(mVec128, _y.get128(), 1);
    return *this;
}

inline const FloatInVec Quat::getY() const
{
    return FloatInVec(mVec128, 1);
}

inline Quat & Quat::setZ(float _z)
{
    sseVecSetElement(mVec128, _z, 2);
    return *this;
}

inline Quat & Quat::setZ(const FloatInVec & _z)
{
    mVec128 = sseVecInsert(mVec128, _z.get128(), 2);
    return *this;
}

inline const FloatInVec Quat::getZ() const
{
    return FloatInVec(mVec128, 2);
}

inline Quat & Quat::setW(float _w)
{
    sseVecSetElement(mVec128, _w, 3);
    return *this;
}

inline Quat & Quat::setW(const FloatInVec & _w)
{
    mVec128 = sseVecInsert(mVec128, _w.get128(), 3);
    return *this;
}

inline const FloatInVec Quat::getW() const
{
    return FloatInVec(mVec128, 3);
}

inline Quat & Quat::setElem(int idx, float value)
{
    sseVecSetElement(mVec128, value, idx);
    return *this;
}

inline Quat & Quat::setElem(int idx, const FloatInVec & value)
{
    mVec128 = sseVecInsert(mVec128, value.get128(), idx);
    return *this;
}

inline const FloatInVec Quat::getElem(int idx) const
{
    return FloatInVec(mVec128, idx);
}

inline VecIdx Quat::operator[](int idx)
{
    return VecIdx(mVec128, idx);
}

inline const FloatInVec Quat::operator[](int idx) const
{
    return FloatInVec(mVec128, idx);
}

inline const Quat Quat::operator + (const Quat & quat) const
{
    return Quat(_mm_add_ps(mVec128, quat.mVec128));
}

inline const Quat Quat::operator - (const Quat & quat) const
{
    return Quat(_mm_sub_ps(mVec128, quat.mVec128));
}

inline const Quat Quat::operator * (float scalar) const
{
    return *this * FloatInVec(scalar);
}

inline const Quat Quat::operator * (const FloatInVec & scalar) const
{
    return Quat(_mm_mul_ps(mVec128, scalar.get128()));
}

inline Quat & Quat::operator += (const Quat & quat)
{
    *this = *this + quat;
    return *this;
}

inline Quat & Quat::operator -= (const Quat & quat)
{
    *this = *this - quat;
    return *this;
}

inline Quat & Quat::operator *= (float scalar)
{
    *this = *this * scalar;
    return *this;
}

inline Quat & Quat::operator *= (const FloatInVec & scalar)
{
    *this = *this * scalar;
    return *this;
}

inline const Quat Quat::operator / (float scalar) const
{
    return *this / FloatInVec(scalar);
}

inline const Quat Quat::operator / (const FloatInVec & scalar) const
{
    return Quat(_mm_div_ps(mVec128, scalar.get128()));
}

inline Quat & Quat::operator /= (float scalar)
{
    *this = *this / scalar;
    return *this;
}

inline Quat & Quat::operator /= (const FloatInVec & scalar)
{
    *this = *this / scalar;
    return *this;
}

inline const Quat Quat::operator - () const
{
    return Quat(_mm_sub_ps(_mm_setzero_ps(), mVec128));
}

inline const Quat operator * (float scalar, const Quat & quat)
{
    return FloatInVec(scalar) * quat;
}

inline const Quat operator * (const FloatInVec & scalar, const Quat & quat)
{
    return quat * scalar;
}

inline const FloatInVec dot(const Quat & quat0, const Quat & quat1)
{
    return FloatInVec(sseVecDot4(quat0.get128(), quat1.get128()), 0);
}

inline const FloatInVec norm(const Quat & quat)
{
    return FloatInVec(sseVecDot4(quat.get128(), quat.get128()), 0);
}

inline const FloatInVec length(const Quat & quat)
{
    return FloatInVec(_mm_sqrt_ps(sseVecDot4(quat.get128(), quat.get128())), 0);
}

inline const Quat normalize(const Quat & quat)
{
    return Quat(_mm_mul_ps(quat.get128(), _mm_rsqrt_ps(sseVecDot4(quat.get128(), quat.get128()))));
}

inline const Quat Quat::rotation(const Vector3 & unitVec0, const Vector3 & unitVec1)
{
    Vector3 crossVec;
    __m128 cosAngle, cosAngleX2Plus2, recipCosHalfAngleX2, cosHalfAngleX2, res;
    cosAngle = sseVecDot3(unitVec0.get128(), unitVec1.get128());
    cosAngleX2Plus2 = sseMAdd(cosAngle, _mm_set1_ps(2.0f), _mm_set1_ps(2.0f));
    recipCosHalfAngleX2 = _mm_rsqrt_ps(cosAngleX2Plus2);
    cosHalfAngleX2 = _mm_mul_ps(recipCosHalfAngleX2, cosAngleX2Plus2);
    crossVec = cross(unitVec0, unitVec1);
    res = _mm_mul_ps(crossVec.get128(), recipCosHalfAngleX2);
    VECTORMATH_ALIGNED(unsigned int sw[4]) = { 0, 0, 0, 0xFFFFFFFF };
    res = sseSelect(res, _mm_mul_ps(cosHalfAngleX2, _mm_set1_ps(0.5f)), sw);
    return Quat(res);
}

inline const Quat Quat::rotation(float radians, const Vector3 & unitVec)
{
    return rotation(FloatInVec(radians), unitVec);
}

inline const Quat Quat::rotation(const FloatInVec & radians, const Vector3 & unitVec)
{
    __m128 s, c, angle, res;
    angle = _mm_mul_ps(radians.get128(), _mm_set1_ps(0.5f));
    sseSinfCosf(angle, &s, &c);
    VECTORMATH_ALIGNED(unsigned int sw[4]) = { 0, 0, 0, 0xFFFFFFFF };
    res = sseSelect(_mm_mul_ps(unitVec.get128(), s), c, sw);
    return Quat(res);
}

inline const Quat Quat::rotationX(float radians)
{
    return rotationX(FloatInVec(radians));
}

inline const Quat Quat::rotationX(const FloatInVec & radians)
{
    __m128 s, c, angle, res;
    angle = _mm_mul_ps(radians.get128(), _mm_set1_ps(0.5f));
    sseSinfCosf(angle, &s, &c);
    VECTORMATH_ALIGNED(unsigned int xsw[4]) = { 0xFFFFFFFF, 0, 0, 0 };
    VECTORMATH_ALIGNED(unsigned int wsw[4]) = { 0, 0, 0, 0xFFFFFFFF };
    res = sseSelect(_mm_setzero_ps(), s, xsw);
    res = sseSelect(res, c, wsw);
    return Quat(res);
}

inline const Quat Quat::rotationY(float radians)
{
    return rotationY(FloatInVec(radians));
}

inline const Quat Quat::rotationY(const FloatInVec & radians)
{
    __m128 s, c, angle, res;
    angle = _mm_mul_ps(radians.get128(), _mm_set1_ps(0.5f));
    sseSinfCosf(angle, &s, &c);
    VECTORMATH_ALIGNED(unsigned int ysw[4]) = { 0, 0xFFFFFFFF, 0, 0 };
    VECTORMATH_ALIGNED(unsigned int wsw[4]) = { 0, 0, 0, 0xFFFFFFFF };
    res = sseSelect(_mm_setzero_ps(), s, ysw);
    res = sseSelect(res, c, wsw);
    return Quat(res);
}

inline const Quat Quat::rotationZ(float radians)
{
    return rotationZ(FloatInVec(radians));
}

inline const Quat Quat::rotationZ(const FloatInVec & radians)
{
    __m128 s, c, angle, res;
    angle = _mm_mul_ps(radians.get128(), _mm_set1_ps(0.5f));
    sseSinfCosf(angle, &s, &c);
    VECTORMATH_ALIGNED(unsigned int zsw[4]) = { 0, 0, 0xFFFFFFFF, 0 };
    VECTORMATH_ALIGNED(unsigned int wsw[4]) = { 0, 0, 0, 0xFFFFFFFF };
    res = sseSelect(_mm_setzero_ps(), s, zsw);
    res = sseSelect(res, c, wsw);
    return Quat(res);
}

inline const Quat Quat::operator * (const Quat & quat) const
{
    __m128 ldata, rdata, qv, tmp0, tmp1, tmp2, tmp3;
    __m128 product, l_wxyz, r_wxyz, xy, qw;
    ldata = mVec128;
    rdata = quat.mVec128;
    tmp0 = _mm_shuffle_ps(ldata, ldata, _MM_SHUFFLE(3, 0, 2, 1));
    tmp1 = _mm_shuffle_ps(rdata, rdata, _MM_SHUFFLE(3, 1, 0, 2));
    tmp2 = _mm_shuffle_ps(ldata, ldata, _MM_SHUFFLE(3, 1, 0, 2));
    tmp3 = _mm_shuffle_ps(rdata, rdata, _MM_SHUFFLE(3, 0, 2, 1));
    qv = _mm_mul_ps(sseSplat(ldata, 3), rdata);
    qv = sseMAdd(sseSplat(rdata, 3), ldata, qv);
    qv = sseMAdd(tmp0, tmp1, qv);
    qv = sseMSub(tmp2, tmp3, qv);
    product = _mm_mul_ps(ldata, rdata);
    l_wxyz = sseSld(ldata, ldata, 12);
    r_wxyz = sseSld(rdata, rdata, 12);
    qw = sseMSub(l_wxyz, r_wxyz, product);
    xy = sseMAdd(l_wxyz, r_wxyz, product);
    qw = _mm_sub_ps(qw, sseSld(xy, xy, 8));
    VECTORMATH_ALIGNED(unsigned int sw[4]) = { 0, 0, 0, 0xFFFFFFFF };
    return Quat(sseSelect(qv, qw, sw));
}

inline Quat & Quat::operator *= (const Quat & quat)
{
    *this = *this * quat;
    return *this;
}

inline const Vector3 rotate(const Quat & quat, const Vector3 & vec)
{
    __m128 qdata, vdata, product, tmp0, tmp1, tmp2, tmp3, wwww, qv, qw, res;
    qdata = quat.get128();
    vdata = vec.get128();
    tmp0 = _mm_shuffle_ps(qdata, qdata, _MM_SHUFFLE(3, 0, 2, 1));
    tmp1 = _mm_shuffle_ps(vdata, vdata, _MM_SHUFFLE(3, 1, 0, 2));
    tmp2 = _mm_shuffle_ps(qdata, qdata, _MM_SHUFFLE(3, 1, 0, 2));
    tmp3 = _mm_shuffle_ps(vdata, vdata, _MM_SHUFFLE(3, 0, 2, 1));
    wwww = sseSplat(qdata, 3);
    qv = _mm_mul_ps(wwww, vdata);
    qv = sseMAdd(tmp0, tmp1, qv);
    qv = sseMSub(tmp2, tmp3, qv);
    product = _mm_mul_ps(qdata, vdata);
    qw = sseMAdd(sseSld(qdata, qdata, 4), sseSld(vdata, vdata, 4), product);
    qw = _mm_add_ps(sseSld(product, product, 8), qw);
    tmp1 = _mm_shuffle_ps(qv, qv, _MM_SHUFFLE(3, 1, 0, 2));
    tmp3 = _mm_shuffle_ps(qv, qv, _MM_SHUFFLE(3, 0, 2, 1));
    res = _mm_mul_ps(sseSplat(qw, 0), qdata);
    res = sseMAdd(wwww, qv, res);
    res = sseMAdd(tmp0, tmp1, res);
    res = sseMSub(tmp2, tmp3, res);
    return Vector3(res);
}

inline const Quat conj(const Quat & quat)
{
    VECTORMATH_ALIGNED(unsigned int sw[4]) = { 0x80000000, 0x80000000, 0x80000000, 0 };
    return Quat(_mm_xor_ps(quat.get128(), _mm_load_ps((float *)sw)));
}

inline const Quat select(const Quat & quat0, const Quat & quat1, bool select1)
{
    return select(quat0, quat1, BoolInVec(select1));
}

inline const Quat select(const Quat & quat0, const Quat & quat1, const BoolInVec & select1)
{
    return Quat(sseSelect(quat0.get128(), quat1.get128(), select1.get128()));
}

#ifdef VECTORMATH_DEBUG

inline void print(const Quat & quat)
{
    SSEFloat tmp;
    tmp.m128 = quat.get128();
    printf("( %f %f %f %f )\n", tmp.f[0], tmp.f[1], tmp.f[2], tmp.f[3]);
}

inline void print(const Quat & quat, const char * name)
{
    SSEFloat tmp;
    tmp.m128 = quat.get128();
    printf("%s: ( %f %f %f %f )\n", name, tmp.f[0], tmp.f[1], tmp.f[2], tmp.f[3]);
}

#endif // VECTORMATH_DEBUG

} // namespace SSE
} // namespace Vectormath

#endif // VECTORMATH_SSE_QUATERNION_HPP

#ifndef VECTORMATH_SSE_MATRIX_HPP
#define VECTORMATH_SSE_MATRIX_HPP

namespace Vectormath
{
namespace SSE
{

// ========================================================
// Matrix3
// ========================================================

inline Matrix3::Matrix3(const Matrix3 & mat)
{
    mCol0 = mat.mCol0;
    mCol1 = mat.mCol1;
    mCol2 = mat.mCol2;
}

inline Matrix3::Matrix3(float scalar)
{
    mCol0 = Vector3(scalar);
    mCol1 = Vector3(scalar);
    mCol2 = Vector3(scalar);
}

inline Matrix3::Matrix3(const FloatInVec & scalar)
{
    mCol0 = Vector3(scalar);
    mCol1 = Vector3(scalar);
    mCol2 = Vector3(scalar);
}

inline Matrix3::Matrix3(const Quat & unitQuat)
{
    __m128 xyzw_2, wwww, yzxw, zxyw, yzxw_2, zxyw_2;
    __m128 tmp0, tmp1, tmp2, tmp3, tmp4, tmp5;

    VECTORMATH_ALIGNED(unsigned int sx[4]) = { 0xFFFFFFFF, 0, 0, 0 };
    VECTORMATH_ALIGNED(unsigned int sz[4]) = { 0, 0, 0xFFFFFFFF, 0 };

    __m128 select_x = _mm_load_ps((float *)sx);
    __m128 select_z = _mm_load_ps((float *)sz);

    xyzw_2 = _mm_add_ps(unitQuat.get128(), unitQuat.get128());
    wwww = _mm_shuffle_ps(unitQuat.get128(), unitQuat.get128(), _MM_SHUFFLE(3, 3, 3, 3));
    yzxw = _mm_shuffle_ps(unitQuat.get128(), unitQuat.get128(), _MM_SHUFFLE(3, 0, 2, 1));
    zxyw = _mm_shuffle_ps(unitQuat.get128(), unitQuat.get128(), _MM_SHUFFLE(3, 1, 0, 2));
    yzxw_2 = _mm_shuffle_ps(xyzw_2, xyzw_2, _MM_SHUFFLE(3, 0, 2, 1));
    zxyw_2 = _mm_shuffle_ps(xyzw_2, xyzw_2, _MM_SHUFFLE(3, 1, 0, 2));

    tmp0 = _mm_mul_ps(yzxw_2, wwww);                                // tmp0 = 2yw, 2zw, 2xw, 2w2
    tmp1 = _mm_sub_ps(_mm_set1_ps(1.0f), _mm_mul_ps(yzxw, yzxw_2)); // tmp1 = 1 - 2y2, 1 - 2z2, 1 - 2x2, 1 - 2w2
    tmp2 = _mm_mul_ps(yzxw, xyzw_2);                                // tmp2 = 2xy, 2yz, 2xz, 2w2
    tmp0 = _mm_add_ps(_mm_mul_ps(zxyw, xyzw_2), tmp0);              // tmp0 = 2yw + 2zx, 2zw + 2xy, 2xw + 2yz, 2w2 + 2w2
    tmp1 = _mm_sub_ps(tmp1, _mm_mul_ps(zxyw, zxyw_2));              // tmp1 = 1 - 2y2 - 2z2, 1 - 2z2 - 2x2, 1 - 2x2 - 2y2, 1 - 2w2 - 2w2
    tmp2 = _mm_sub_ps(tmp2, _mm_mul_ps(zxyw_2, wwww));              // tmp2 = 2xy - 2zw, 2yz - 2xw, 2xz - 2yw, 2w2 -2w2

    tmp3 = sseSelect(tmp0, tmp1, select_x);
    tmp4 = sseSelect(tmp1, tmp2, select_x);
    tmp5 = sseSelect(tmp2, tmp0, select_x);
    mCol0 = Vector3(sseSelect(tmp3, tmp2, select_z));
    mCol1 = Vector3(sseSelect(tmp4, tmp0, select_z));
    mCol2 = Vector3(sseSelect(tmp5, tmp1, select_z));
}

inline Matrix3::Matrix3(const Vector3 & _col0, const Vector3 & _col1, const Vector3 & _col2)
{
    mCol0 = _col0;
    mCol1 = _col1;
    mCol2 = _col2;
}

inline Matrix3 & Matrix3::setCol0(const Vector3 & _col0)
{
    mCol0 = _col0;
    return *this;
}

inline Matrix3 & Matrix3::setCol1(const Vector3 & _col1)
{
    mCol1 = _col1;
    return *this;
}

inline Matrix3 & Matrix3::setCol2(const Vector3 & _col2)
{
    mCol2 = _col2;
    return *this;
}

inline Matrix3 & Matrix3::setCol(int col, const Vector3 & vec)
{
    *(&mCol0 + col) = vec;
    return *this;
}

inline Matrix3 & Matrix3::setRow(int row, const Vector3 & vec)
{
    mCol0.setElem(row, vec.getElem(0));
    mCol1.setElem(row, vec.getElem(1));
    mCol2.setElem(row, vec.getElem(2));
    return *this;
}

inline Matrix3 & Matrix3::setElem(int col, int row, float val)
{
    (*this)[col].setElem(row, val);
    return *this;
}

inline Matrix3 & Matrix3::setElem(int col, int row, const FloatInVec & val)
{
    Vector3 tmpV3_0;
    tmpV3_0 = this->getCol(col);
    tmpV3_0.setElem(row, val);
    this->setCol(col, tmpV3_0);
    return *this;
}

inline const FloatInVec Matrix3::getElem(int col, int row) const
{
    return this->getCol(col).getElem(row);
}

inline const Vector3 Matrix3::getCol0() const
{
    return mCol0;
}

inline const Vector3 Matrix3::getCol1() const
{
    return mCol1;
}

inline const Vector3 Matrix3::getCol2() const
{
    return mCol2;
}

inline const Vector3 Matrix3::getCol(int col) const
{
    return *(&mCol0 + col);
}

inline const Vector3 Matrix3::getRow(int row) const
{
    return Vector3(mCol0.getElem(row), mCol1.getElem(row), mCol2.getElem(row));
}

inline Vector3 & Matrix3::operator[](int col)
{
    return *(&mCol0 + col);
}

inline const Vector3 Matrix3::operator[](int col) const
{
    return *(&mCol0 + col);
}

inline Matrix3 & Matrix3::operator = (const Matrix3 & mat)
{
    mCol0 = mat.mCol0;
    mCol1 = mat.mCol1;
    mCol2 = mat.mCol2;
    return *this;
}

inline const Matrix3 transpose(const Matrix3 & mat)
{
    __m128 tmp0, tmp1, res0, res1, res2;
    tmp0 = sseMergeH(mat.getCol0().get128(), mat.getCol2().get128());
    tmp1 = sseMergeL(mat.getCol0().get128(), mat.getCol2().get128());
    res0 = sseMergeH(tmp0, mat.getCol1().get128());
    VECTORMATH_ALIGNED(unsigned int select_y[4]) = { 0, 0xFFFFFFFF, 0, 0 };
    res1 = _mm_shuffle_ps(tmp0, tmp0, _MM_SHUFFLE(0, 3, 2, 2));
    res1 = sseSelect(res1, mat.getCol1().get128(), select_y);
    res2 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(0, 1, 1, 0));
    res2 = sseSelect(res2, sseSplat(mat.getCol1().get128(), 2), select_y);
    return Matrix3(Vector3(res0), Vector3(res1), Vector3(res2));
}

inline const Matrix3 inverse(const Matrix3 & mat)
{
    __m128 tmp0, tmp1, tmp2, tmp3, tmp4, dot, invdet, inv0, inv1, inv2;
    tmp2 = sseVecCross(mat.getCol0().get128(), mat.getCol1().get128());
    tmp0 = sseVecCross(mat.getCol1().get128(), mat.getCol2().get128());
    tmp1 = sseVecCross(mat.getCol2().get128(), mat.getCol0().get128());
    dot = sseVecDot3(tmp2, mat.getCol2().get128());
    dot = sseSplat(dot, 0);
    invdet = sseRecipf(dot);
    tmp3 = sseMergeH(tmp0, tmp2);
    tmp4 = sseMergeL(tmp0, tmp2);
    inv0 = sseMergeH(tmp3, tmp1);
    VECTORMATH_ALIGNED(unsigned int select_y[4]) = { 0, 0xFFFFFFFF, 0, 0 };
    inv1 = _mm_shuffle_ps(tmp3, tmp3, _MM_SHUFFLE(0, 3, 2, 2));
    inv1 = sseSelect(inv1, tmp1, select_y);
    inv2 = _mm_shuffle_ps(tmp4, tmp4, _MM_SHUFFLE(0, 1, 1, 0));
    inv2 = sseSelect(inv2, sseSplat(tmp1, 2), select_y);
    inv0 = _mm_mul_ps(inv0, invdet);
    inv1 = _mm_mul_ps(inv1, invdet);
    inv2 = _mm_mul_ps(inv2, invdet);
    return Matrix3(Vector3(inv0), Vector3(inv1), Vector3(inv2));
}

inline const FloatInVec determinant(const Matrix3 & mat)
{
    return dot(mat.getCol2(), cross(mat.getCol0(), mat.getCol1()));
}

inline const Matrix3 Matrix3::operator + (const Matrix3 & mat) const
{
    return Matrix3((mCol0 + mat.mCol0),
                   (mCol1 + mat.mCol1),
                   (mCol2 + mat.mCol2));
}

inline const Matrix3 Matrix3::operator - (const Matrix3 & mat) const
{
    return Matrix3((mCol0 - mat.mCol0),
                   (mCol1 - mat.mCol1),
                   (mCol2 - mat.mCol2));
}

inline Matrix3 & Matrix3::operator += (const Matrix3 & mat)
{
    *this = *this + mat;
    return *this;
}

inline Matrix3 & Matrix3::operator -= (const Matrix3 & mat)
{
    *this = *this - mat;
    return *this;
}

inline const Matrix3 Matrix3::operator - () const
{
    return Matrix3((-mCol0), (-mCol1), (-mCol2));
}

inline const Matrix3 absPerElem(const Matrix3 & mat)
{
    return Matrix3(absPerElem(mat.getCol0()),
                   absPerElem(mat.getCol1()),
                   absPerElem(mat.getCol2()));
}

inline const Matrix3 Matrix3::operator * (float scalar) const
{
    return *this * FloatInVec(scalar);
}

inline const Matrix3 Matrix3::operator * (const FloatInVec & scalar) const
{
    return Matrix3((mCol0 * scalar),
                   (mCol1 * scalar),
                   (mCol2 * scalar));
}

inline Matrix3 & Matrix3::operator *= (float scalar)
{
    return *this *= FloatInVec(scalar);
}

inline Matrix3 & Matrix3::operator *= (const FloatInVec & scalar)
{
    *this = *this * scalar;
    return *this;
}

inline const Matrix3 operator * (float scalar, const Matrix3 & mat)
{
    return FloatInVec(scalar) * mat;
}

inline const Matrix3 operator * (const FloatInVec & scalar, const Matrix3 & mat)
{
    return mat * scalar;
}

inline const Vector3 Matrix3::operator * (const Vector3 & vec) const
{
    __m128 res;
    __m128 xxxx, yyyy, zzzz;
    xxxx = sseSplat(vec.get128(), 0);
    yyyy = sseSplat(vec.get128(), 1);
    zzzz = sseSplat(vec.get128(), 2);
    res = _mm_mul_ps(mCol0.get128(), xxxx);
    res = sseMAdd(mCol1.get128(), yyyy, res);
    res = sseMAdd(mCol2.get128(), zzzz, res);
    return Vector3(res);
}

inline const Matrix3 Matrix3::operator * (const Matrix3 & mat) const
{
    return Matrix3((*this * mat.mCol0),
                   (*this * mat.mCol1),
                   (*this * mat.mCol2));
}

inline Matrix3 & Matrix3::operator *= (const Matrix3 & mat)
{
    *this = *this * mat;
    return *this;
}

inline const Matrix3 mulPerElem(const Matrix3 & mat0, const Matrix3 & mat1)
{
    return Matrix3(mulPerElem(mat0.getCol0(), mat1.getCol0()),
                   mulPerElem(mat0.getCol1(), mat1.getCol1()),
                   mulPerElem(mat0.getCol2(), mat1.getCol2()));
}

inline const Matrix3 Matrix3::identity()
{
    return Matrix3(Vector3::xAxis(),
                   Vector3::yAxis(),
                   Vector3::zAxis());
}

inline const Matrix3 Matrix3::rotationX(float radians)
{
    return rotationX(FloatInVec(radians));
}

inline const Matrix3 Matrix3::rotationX(const FloatInVec & radians)
{
    __m128 s, c, res1, res2;
    __m128 zero;
    VECTORMATH_ALIGNED(unsigned int select_y[4]) = { 0, 0xFFFFFFFF, 0, 0 };
    VECTORMATH_ALIGNED(unsigned int select_z[4]) = { 0, 0, 0xFFFFFFFF, 0 };
    zero = _mm_setzero_ps();
    sseSinfCosf(radians.get128(), &s, &c);
    res1 = sseSelect(zero, c, select_y);
    res1 = sseSelect(res1, s, select_z);
    res2 = sseSelect(zero, sseNegatef(s), select_y);
    res2 = sseSelect(res2, c, select_z);
    return Matrix3(Vector3::xAxis(), Vector3(res1), Vector3(res2));
}

inline const Matrix3 Matrix3::rotationY(float radians)
{
    return rotationY(FloatInVec(radians));
}

inline const Matrix3 Matrix3::rotationY(const FloatInVec & radians)
{
    __m128 s, c, res0, res2;
    __m128 zero;
    VECTORMATH_ALIGNED(unsigned int select_x[4]) = { 0xFFFFFFFF, 0, 0, 0 };
    VECTORMATH_ALIGNED(unsigned int select_z[4]) = { 0, 0, 0xFFFFFFFF, 0 };
    zero = _mm_setzero_ps();
    sseSinfCosf(radians.get128(), &s, &c);
    res0 = sseSelect(zero, c, select_x);
    res0 = sseSelect(res0, sseNegatef(s), select_z);
    res2 = sseSelect(zero, s, select_x);
    res2 = sseSelect(res2, c, select_z);
    return Matrix3(Vector3(res0), Vector3::yAxis(), Vector3(res2));
}

inline const Matrix3 Matrix3::rotationZ(float radians)
{
    return rotationZ(FloatInVec(radians));
}

inline const Matrix3 Matrix3::rotationZ(const FloatInVec & radians)
{
    __m128 s, c, res0, res1;
    __m128 zero;
    VECTORMATH_ALIGNED(unsigned int select_x[4]) = { 0xFFFFFFFF, 0, 0, 0 };
    VECTORMATH_ALIGNED(unsigned int select_y[4]) = { 0, 0xFFFFFFFF, 0, 0 };
    zero = _mm_setzero_ps();
    sseSinfCosf(radians.get128(), &s, &c);
    res0 = sseSelect(zero, c, select_x);
    res0 = sseSelect(res0, s, select_y);
    res1 = sseSelect(zero, sseNegatef(s), select_x);
    res1 = sseSelect(res1, c, select_y);
    return Matrix3(Vector3(res0), Vector3(res1), Vector3::zAxis());
}

inline const Matrix3 Matrix3::rotationZYX(const Vector3 & radiansXYZ)
{
    __m128 angles, s, negS, c, X0, X1, Y0, Y1, Z0, Z1, tmp;
    angles = Vector4(radiansXYZ, 0.0f).get128();
    sseSinfCosf(angles, &s, &c);
    negS = sseNegatef(s);
    Z0 = sseMergeL(c, s);
    Z1 = sseMergeL(negS, c);
    VECTORMATH_ALIGNED(unsigned int select_xyz[4]) = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0 };
    Z1 = _mm_and_ps(Z1, _mm_load_ps((float *)select_xyz));
    Y0 = _mm_shuffle_ps(c, negS, _MM_SHUFFLE(0, 1, 1, 1));
    Y1 = _mm_shuffle_ps(s, c, _MM_SHUFFLE(0, 1, 1, 1));
    X0 = sseSplat(s, 0);
    X1 = sseSplat(c, 0);
    tmp = _mm_mul_ps(Z0, Y1);
    return Matrix3(Vector3(_mm_mul_ps(Z0, Y0)),
                   Vector3(sseMAdd(Z1, X1, _mm_mul_ps(tmp, X0))),
                   Vector3(sseMSub(Z1, X0, _mm_mul_ps(tmp, X1))));
}

inline const Matrix3 Matrix3::rotation(float radians, const Vector3 & unitVec)
{
    return rotation(FloatInVec(radians), unitVec);
}

inline const Matrix3 Matrix3::rotation(const FloatInVec & radians, const Vector3 & unitVec)
{
    __m128 axis, s, c, oneMinusC, axisS, negAxisS, xxxx, yyyy, zzzz, tmp0, tmp1, tmp2;
    axis = unitVec.get128();
    sseSinfCosf(radians.get128(), &s, &c);
    xxxx = sseSplat(axis, 0);
    yyyy = sseSplat(axis, 1);
    zzzz = sseSplat(axis, 2);
    oneMinusC = _mm_sub_ps(_mm_set1_ps(1.0f), c);
    axisS = _mm_mul_ps(axis, s);
    negAxisS = sseNegatef(axisS);
    VECTORMATH_ALIGNED(unsigned int select_x[4]) = { 0xFFFFFFFF, 0, 0, 0 };
    VECTORMATH_ALIGNED(unsigned int select_y[4]) = { 0, 0xFFFFFFFF, 0, 0 };
    VECTORMATH_ALIGNED(unsigned int select_z[4]) = { 0, 0, 0xFFFFFFFF, 0 };
    tmp0 = _mm_shuffle_ps(axisS, axisS, _MM_SHUFFLE(0, 0, 2, 0));
    tmp0 = sseSelect(tmp0, sseSplat(negAxisS, 1), select_z);
    tmp1 = sseSelect(sseSplat(axisS, 0), sseSplat(negAxisS, 2), select_x);
    tmp2 = _mm_shuffle_ps(axisS, axisS, _MM_SHUFFLE(0, 0, 0, 1));
    tmp2 = sseSelect(tmp2, sseSplat(negAxisS, 0), select_y);
    tmp0 = sseSelect(tmp0, c, select_x);
    tmp1 = sseSelect(tmp1, c, select_y);
    tmp2 = sseSelect(tmp2, c, select_z);
    return Matrix3(Vector3(sseMAdd(_mm_mul_ps(axis, xxxx), oneMinusC, tmp0)),
                   Vector3(sseMAdd(_mm_mul_ps(axis, yyyy), oneMinusC, tmp1)),
                   Vector3(sseMAdd(_mm_mul_ps(axis, zzzz), oneMinusC, tmp2)));
}

inline const Matrix3 Matrix3::rotation(const Quat & unitQuat)
{
    return Matrix3(unitQuat);
}

inline const Matrix3 Matrix3::scale(const Vector3 & scaleVec)
{
    const __m128 zero = _mm_setzero_ps();
    VECTORMATH_ALIGNED(unsigned int select_x[4]) = { 0xFFFFFFFF, 0, 0, 0 };
    VECTORMATH_ALIGNED(unsigned int select_y[4]) = { 0, 0xFFFFFFFF, 0, 0 };
    VECTORMATH_ALIGNED(unsigned int select_z[4]) = { 0, 0, 0xFFFFFFFF, 0 };
    return Matrix3(Vector3(sseSelect(zero, scaleVec.get128(), select_x)),
                   Vector3(sseSelect(zero, scaleVec.get128(), select_y)),
                   Vector3(sseSelect(zero, scaleVec.get128(), select_z)));
}

inline const Matrix3 appendScale(const Matrix3 & mat, const Vector3 & scaleVec)
{
    return Matrix3((mat.getCol0() * scaleVec.getX()),
                   (mat.getCol1() * scaleVec.getY()),
                   (mat.getCol2() * scaleVec.getZ()));
}

inline const Matrix3 prependScale(const Vector3 & scaleVec, const Matrix3 & mat)
{
    return Matrix3(mulPerElem(mat.getCol0(), scaleVec),
                   mulPerElem(mat.getCol1(), scaleVec),
                   mulPerElem(mat.getCol2(), scaleVec));
}

inline const Matrix3 select(const Matrix3 & mat0, const Matrix3 & mat1, bool select1)
{
    return Matrix3(select(mat0.getCol0(), mat1.getCol0(), select1),
                   select(mat0.getCol1(), mat1.getCol1(), select1),
                   select(mat0.getCol2(), mat1.getCol2(), select1));
}

inline const Matrix3 select(const Matrix3 & mat0, const Matrix3 & mat1, const BoolInVec & select1)
{
    return Matrix3(select(mat0.getCol0(), mat1.getCol0(), select1),
                   select(mat0.getCol1(), mat1.getCol1(), select1),
                   select(mat0.getCol2(), mat1.getCol2(), select1));
}

#ifdef VECTORMATH_DEBUG

inline void print(const Matrix3 & mat)
{
    print(mat.getRow(0));
    print(mat.getRow(1));
    print(mat.getRow(2));
}

inline void print(const Matrix3 & mat, const char * name)
{
    printf("%s:\n", name);
    print(mat);
}

#endif // VECTORMATH_DEBUG

// ========================================================
// Matrix4
// ========================================================

inline Matrix4::Matrix4(const Matrix4 & mat)
{
    mCol0 = mat.mCol0;
    mCol1 = mat.mCol1;
    mCol2 = mat.mCol2;
    mCol3 = mat.mCol3;
}

inline Matrix4::Matrix4(float scalar)
{
    mCol0 = Vector4(scalar);
    mCol1 = Vector4(scalar);
    mCol2 = Vector4(scalar);
    mCol3 = Vector4(scalar);
}

inline Matrix4::Matrix4(const FloatInVec & scalar)
{
    mCol0 = Vector4(scalar);
    mCol1 = Vector4(scalar);
    mCol2 = Vector4(scalar);
    mCol3 = Vector4(scalar);
}

inline Matrix4::Matrix4(const Transform3 & mat)
{
    mCol0 = Vector4(mat.getCol0(), 0.0f);
    mCol1 = Vector4(mat.getCol1(), 0.0f);
    mCol2 = Vector4(mat.getCol2(), 0.0f);
    mCol3 = Vector4(mat.getCol3(), 1.0f);
}

inline Matrix4::Matrix4(const Vector4 & _col0, const Vector4 & _col1, const Vector4 & _col2, const Vector4 & _col3)
{
    mCol0 = _col0;
    mCol1 = _col1;
    mCol2 = _col2;
    mCol3 = _col3;
}

inline Matrix4::Matrix4(const Matrix3 & mat, const Vector3 & translateVec)
{
    mCol0 = Vector4(mat.getCol0(), 0.0f);
    mCol1 = Vector4(mat.getCol1(), 0.0f);
    mCol2 = Vector4(mat.getCol2(), 0.0f);
    mCol3 = Vector4(translateVec,  1.0f);
}

inline Matrix4::Matrix4(const Quat & unitQuat, const Vector3 & translateVec)
{
    Matrix3 mat;
    mat = Matrix3(unitQuat);
    mCol0 = Vector4(mat.getCol0(), 0.0f);
    mCol1 = Vector4(mat.getCol1(), 0.0f);
    mCol2 = Vector4(mat.getCol2(), 0.0f);
    mCol3 = Vector4(translateVec,  1.0f);
}

inline Matrix4 & Matrix4::setCol0(const Vector4 & _col0)
{
    mCol0 = _col0;
    return *this;
}

inline Matrix4 & Matrix4::setCol1(const Vector4 & _col1)
{
    mCol1 = _col1;
    return *this;
}

inline Matrix4 & Matrix4::setCol2(const Vector4 & _col2)
{
    mCol2 = _col2;
    return *this;
}

inline Matrix4 & Matrix4::setCol3(const Vector4 & _col3)
{
    mCol3 = _col3;
    return *this;
}

inline Matrix4 & Matrix4::setCol(int col, const Vector4 & vec)
{
    *(&mCol0 + col) = vec;
    return *this;
}

inline Matrix4 & Matrix4::setRow(int row, const Vector4 & vec)
{
    mCol0.setElem(row, vec.getElem(0));
    mCol1.setElem(row, vec.getElem(1));
    mCol2.setElem(row, vec.getElem(2));
    mCol3.setElem(row, vec.getElem(3));
    return *this;
}

inline Matrix4 & Matrix4::setElem(int col, int row, float val)
{
    (*this)[col].setElem(row, val);
    return *this;
}

inline Matrix4 & Matrix4::setElem(int col, int row, const FloatInVec & val)
{
    Vector4 tmpV3_0;
    tmpV3_0 = this->getCol(col);
    tmpV3_0.setElem(row, val);
    this->setCol(col, tmpV3_0);
    return *this;
}

inline const FloatInVec Matrix4::getElem(int col, int row) const
{
    return this->getCol(col).getElem(row);
}

inline const Vector4 Matrix4::getCol0() const
{
    return mCol0;
}

inline const Vector4 Matrix4::getCol1() const
{
    return mCol1;
}

inline const Vector4 Matrix4::getCol2() const
{
    return mCol2;
}

inline const Vector4 Matrix4::getCol3() const
{
    return mCol3;
}

inline const Vector4 Matrix4::getCol(int col) const
{
    return *(&mCol0 + col);
}

inline const Vector4 Matrix4::getRow(int row) const
{
    return Vector4(mCol0.getElem(row), mCol1.getElem(row), mCol2.getElem(row), mCol3.getElem(row));
}

inline Vector4 & Matrix4::operator[](int col)
{
    return *(&mCol0 + col);
}

inline const Vector4 Matrix4::operator[](int col) const
{
    return *(&mCol0 + col);
}

inline Matrix4 & Matrix4::operator = (const Matrix4 & mat)
{
    mCol0 = mat.mCol0;
    mCol1 = mat.mCol1;
    mCol2 = mat.mCol2;
    mCol3 = mat.mCol3;
    return *this;
}

inline const Matrix4 transpose(const Matrix4 & mat)
{
    __m128 tmp0, tmp1, tmp2, tmp3, res0, res1, res2, res3;
    tmp0 = sseMergeH(mat.getCol0().get128(), mat.getCol2().get128());
    tmp1 = sseMergeH(mat.getCol1().get128(), mat.getCol3().get128());
    tmp2 = sseMergeL(mat.getCol0().get128(), mat.getCol2().get128());
    tmp3 = sseMergeL(mat.getCol1().get128(), mat.getCol3().get128());
    res0 = sseMergeH(tmp0, tmp1);
    res1 = sseMergeL(tmp0, tmp1);
    res2 = sseMergeH(tmp2, tmp3);
    res3 = sseMergeL(tmp2, tmp3);
    return Matrix4(Vector4(res0), Vector4(res1), Vector4(res2), Vector4(res3));
}

inline const Matrix4 inverse(const Matrix4 & mat)
{
    VECTORMATH_ALIGNED(unsigned int PNPN[4]) = { 0x00000000, 0x80000000, 0x00000000, 0x80000000 };
    VECTORMATH_ALIGNED(unsigned int NPNP[4]) = { 0x80000000, 0x00000000, 0x80000000, 0x00000000 };
    VECTORMATH_ALIGNED(float X1_YZ0_W1[4])   = { 1.0f, 0.0f, 0.0f, 1.0f };

    __m128 Va, Vb, Vc;
    __m128 r1, r2, r3, tt, tt2;
    __m128 sum, Det, RDet;
    __m128 trns0, trns1, trns2, trns3;

    __m128 _L1 = mat.getCol0().get128();
    __m128 _L2 = mat.getCol1().get128();
    __m128 _L3 = mat.getCol2().get128();
    __m128 _L4 = mat.getCol3().get128();
    // Calculating the minterms for the first line.

    // sseRor is just a macro using _mm_shuffle_ps().
    tt = _L4;
    tt2 = sseRor(_L3, 1);
    Vc = _mm_mul_ps(tt2, sseRor(tt, 0)); 
    Va = _mm_mul_ps(tt2, sseRor(tt, 2));
    Vb = _mm_mul_ps(tt2, sseRor(tt, 3));

    r1 = _mm_sub_ps(sseRor(Va, 1), sseRor(Vc, 2));
    r2 = _mm_sub_ps(sseRor(Vb, 2), sseRor(Vb, 0));
    r3 = _mm_sub_ps(sseRor(Va, 0), sseRor(Vc, 1));

    tt = _L2;
    Va = sseRor(tt, 1);
    sum = _mm_mul_ps(Va, r1);
    Vb = sseRor(tt, 2);
    sum = _mm_add_ps(sum, _mm_mul_ps(Vb, r2));
    Vc = sseRor(tt, 3);
    sum = _mm_add_ps(sum, _mm_mul_ps(Vc, r3));

    // Calculating the determinant.
    Det = _mm_mul_ps(sum, _L1);
    Det = _mm_add_ps(Det, _mm_movehl_ps(Det, Det));

    const __m128 Sign_PNPN = _mm_load_ps((float *)PNPN);
    const __m128 Sign_NPNP = _mm_load_ps((float *)NPNP);

    __m128 mtL1 = _mm_xor_ps(sum, Sign_PNPN);

    // Calculating the minterms of the second line (using previous results).
    tt = sseRor(_L1, 1);
    sum = _mm_mul_ps(tt, r1);
    tt = sseRor(tt, 1);
    sum = _mm_add_ps(sum, _mm_mul_ps(tt, r2));
    tt = sseRor(tt, 1);
    sum = _mm_add_ps(sum, _mm_mul_ps(tt, r3));
    __m128 mtL2 = _mm_xor_ps(sum, Sign_NPNP);

    // Testing the determinant.
    Det = _mm_sub_ss(Det, _mm_shuffle_ps(Det, Det, 1));

    // Calculating the minterms of the third line.
    tt = sseRor(_L1, 1);
    Va = _mm_mul_ps(tt, Vb);
    Vb = _mm_mul_ps(tt, Vc);
    Vc = _mm_mul_ps(tt, _L2); 

    r1 = _mm_sub_ps(sseRor(Va, 1), sseRor(Vc, 2)); 
    r2 = _mm_sub_ps(sseRor(Vb, 2), sseRor(Vb, 0));
    r3 = _mm_sub_ps(sseRor(Va, 0), sseRor(Vc, 1));

    tt = sseRor(_L4, 1);
    sum = _mm_mul_ps(tt, r1);
    tt = sseRor(tt, 1);
    sum = _mm_add_ps(sum, _mm_mul_ps(tt, r2));
    tt = sseRor(tt, 1);
    sum = _mm_add_ps(sum, _mm_mul_ps(tt, r3));
    __m128 mtL3 = _mm_xor_ps(sum, Sign_PNPN);

    // Dividing is FASTER than rcp_nr! (Because rcp_nr causes many register-memory RWs).
    RDet = _mm_div_ss(_mm_load_ss((float *)&X1_YZ0_W1), Det);
    RDet = _mm_shuffle_ps(RDet, RDet, 0x00);

    // Devide the first 12 minterms with the determinant.
    mtL1 = _mm_mul_ps(mtL1, RDet);
    mtL2 = _mm_mul_ps(mtL2, RDet);
    mtL3 = _mm_mul_ps(mtL3, RDet);

    // Calculate the minterms of the forth line and devide by the determinant.
    tt = sseRor(_L3, 1);
    sum = _mm_mul_ps(tt, r1);
    tt = sseRor(tt, 1);
    sum = _mm_add_ps(sum, _mm_mul_ps(tt, r2));
    tt = sseRor(tt, 1);
    sum = _mm_add_ps(sum, _mm_mul_ps(tt, r3));
    __m128 mtL4 = _mm_xor_ps(sum, Sign_NPNP);
    mtL4 = _mm_mul_ps(mtL4, RDet);

    // Now we just have to transpose the minterms matrix.
    trns0 = _mm_unpacklo_ps(mtL1, mtL2);
    trns1 = _mm_unpacklo_ps(mtL3, mtL4);
    trns2 = _mm_unpackhi_ps(mtL1, mtL2);
    trns3 = _mm_unpackhi_ps(mtL3, mtL4);
    _L1 = _mm_movelh_ps(trns0, trns1);
    _L2 = _mm_movehl_ps(trns1, trns0);
    _L3 = _mm_movelh_ps(trns2, trns3);
    _L4 = _mm_movehl_ps(trns3, trns2);

    return Matrix4(Vector4(_L1), Vector4(_L2), Vector4(_L3), Vector4(_L4));
}

inline const Matrix4 affineInverse(const Matrix4 & mat)
{
    Transform3 affineMat;
    affineMat.setCol0(mat.getCol0().getXYZ());
    affineMat.setCol1(mat.getCol1().getXYZ());
    affineMat.setCol2(mat.getCol2().getXYZ());
    affineMat.setCol3(mat.getCol3().getXYZ());
    return Matrix4(inverse(affineMat));
}

inline const Matrix4 orthoInverse(const Matrix4 & mat)
{
    Transform3 affineMat;
    affineMat.setCol0(mat.getCol0().getXYZ());
    affineMat.setCol1(mat.getCol1().getXYZ());
    affineMat.setCol2(mat.getCol2().getXYZ());
    affineMat.setCol3(mat.getCol3().getXYZ());
    return Matrix4(orthoInverse(affineMat));
}

inline const FloatInVec determinant(const Matrix4 & mat)
{
    __m128 Va, Vb, Vc;
    __m128 r1, r2, r3, tt, tt2;
    __m128 sum, Det;

    __m128 _L1 = mat.getCol0().get128();
    __m128 _L2 = mat.getCol1().get128();
    __m128 _L3 = mat.getCol2().get128();
    __m128 _L4 = mat.getCol3().get128();
    // Calculating the minterms for the first line.

    // sseRor is just a macro using _mm_shuffle_ps().
    tt = _L4;
    tt2 = sseRor(_L3, 1);
    Vc = _mm_mul_ps(tt2, sseRor(tt, 0));
    Va = _mm_mul_ps(tt2, sseRor(tt, 2));
    Vb = _mm_mul_ps(tt2, sseRor(tt, 3));

    r1 = _mm_sub_ps(sseRor(Va, 1), sseRor(Vc, 2));
    r2 = _mm_sub_ps(sseRor(Vb, 2), sseRor(Vb, 0));
    r3 = _mm_sub_ps(sseRor(Va, 0), sseRor(Vc, 1));

    tt = _L2;
    Va = sseRor(tt, 1);
    sum = _mm_mul_ps(Va, r1);
    Vb = sseRor(tt, 2);
    sum = _mm_add_ps(sum, _mm_mul_ps(Vb, r2));
    Vc = sseRor(tt, 3);
    sum = _mm_add_ps(sum, _mm_mul_ps(Vc, r3));

    // Calculating the determinant.
    Det = _mm_mul_ps(sum, _L1);
    Det = _mm_add_ps(Det, _mm_movehl_ps(Det, Det));

    // Calculating the minterms of the second line (using previous results).
    tt = sseRor(_L1, 1);
    sum = _mm_mul_ps(tt, r1);
    tt = sseRor(tt, 1);
    sum = _mm_add_ps(sum, _mm_mul_ps(tt, r2));
    tt = sseRor(tt, 1);
    sum = _mm_add_ps(sum, _mm_mul_ps(tt, r3));

    // Testing the determinant.
    Det = _mm_sub_ss(Det, _mm_shuffle_ps(Det, Det, 1));
    return FloatInVec(Det, 0);
}

inline const Matrix4 Matrix4::operator + (const Matrix4 & mat) const
{
    return Matrix4((mCol0 + mat.mCol0),
                   (mCol1 + mat.mCol1),
                   (mCol2 + mat.mCol2),
                   (mCol3 + mat.mCol3));
}

inline const Matrix4 Matrix4::operator - (const Matrix4 & mat) const
{
    return Matrix4((mCol0 - mat.mCol0),
                   (mCol1 - mat.mCol1),
                   (mCol2 - mat.mCol2),
                   (mCol3 - mat.mCol3));
}

inline Matrix4 & Matrix4::operator += (const Matrix4 & mat)
{
    *this = *this + mat;
    return *this;
}

inline Matrix4 & Matrix4::operator -= (const Matrix4 & mat)
{
    *this = *this - mat;
    return *this;
}

inline const Matrix4 Matrix4::operator - () const
{
    return Matrix4((-mCol0), (-mCol1), (-mCol2), (-mCol3));
}

inline const Matrix4 absPerElem(const Matrix4 & mat)
{
    return Matrix4(absPerElem(mat.getCol0()),
                   absPerElem(mat.getCol1()),
                   absPerElem(mat.getCol2()),
                   absPerElem(mat.getCol3()));
}

inline const Matrix4 Matrix4::operator * (float scalar) const
{
    return *this * FloatInVec(scalar);
}

inline const Matrix4 Matrix4::operator * (const FloatInVec & scalar) const
{
    return Matrix4((mCol0 * scalar),
                   (mCol1 * scalar),
                   (mCol2 * scalar),
                   (mCol3 * scalar));
}

inline Matrix4 & Matrix4::operator *= (float scalar)
{
    return *this *= FloatInVec(scalar);
}

inline Matrix4 & Matrix4::operator *= (const FloatInVec & scalar)
{
    *this = *this * scalar;
    return *this;
}

inline const Matrix4 operator * (float scalar, const Matrix4 & mat)
{
    return FloatInVec(scalar) * mat;
}

inline const Matrix4 operator * (const FloatInVec & scalar, const Matrix4 & mat)
{
    return mat * scalar;
}

inline const Vector4 Matrix4::operator * (const Vector4 & vec) const
{
    return Vector4(
        _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(mCol0.get128(), _mm_shuffle_ps(vec.get128(), vec.get128(), _MM_SHUFFLE(0, 0, 0, 0))), _mm_mul_ps(mCol1.get128(), _mm_shuffle_ps(vec.get128(), vec.get128(), _MM_SHUFFLE(1, 1, 1, 1)))),
            _mm_add_ps(_mm_mul_ps(mCol2.get128(), _mm_shuffle_ps(vec.get128(), vec.get128(), _MM_SHUFFLE(2, 2, 2, 2))), _mm_mul_ps(mCol3.get128(), _mm_shuffle_ps(vec.get128(), vec.get128(), _MM_SHUFFLE(3, 3, 3, 3))))));
}

inline const Vector4 Matrix4::operator * (const Vector3 & vec) const
{
    return Vector4(
        _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(mCol0.get128(), _mm_shuffle_ps(vec.get128(), vec.get128(), _MM_SHUFFLE(0, 0, 0, 0))), _mm_mul_ps(mCol1.get128(), _mm_shuffle_ps(vec.get128(), vec.get128(), _MM_SHUFFLE(1, 1, 1, 1)))),
            _mm_mul_ps(mCol2.get128(), _mm_shuffle_ps(vec.get128(), vec.get128(), _MM_SHUFFLE(2, 2, 2, 2)))));
}

inline const Vector4 Matrix4::operator * (const Point3 & pnt) const
{
    return Vector4(
        _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(mCol0.get128(), _mm_shuffle_ps(pnt.get128(), pnt.get128(), _MM_SHUFFLE(0, 0, 0, 0))), _mm_mul_ps(mCol1.get128(), _mm_shuffle_ps(pnt.get128(), pnt.get128(), _MM_SHUFFLE(1, 1, 1, 1)))),
            _mm_add_ps(_mm_mul_ps(mCol2.get128(), _mm_shuffle_ps(pnt.get128(), pnt.get128(), _MM_SHUFFLE(2, 2, 2, 2))), mCol3.get128())));
}

inline const Matrix4 Matrix4::operator * (const Matrix4 & mat) const
{
    return Matrix4((*this * mat.mCol0),
                   (*this * mat.mCol1),
                   (*this * mat.mCol2),
                   (*this * mat.mCol3));
}

inline Matrix4 & Matrix4::operator *= (const Matrix4 & mat)
{
    *this = *this * mat;
    return *this;
}

inline const Matrix4 Matrix4::operator * (const Transform3 & tfrm) const
{
    return Matrix4((*this * tfrm.getCol0()),
                   (*this * tfrm.getCol1()),
                   (*this * tfrm.getCol2()),
                   (*this * Point3(tfrm.getCol3())));
}

inline Matrix4 & Matrix4::operator *= (const Transform3 & tfrm)
{
    *this = *this * tfrm;
    return *this;
}

inline const Matrix4 mulPerElem(const Matrix4 & mat0, const Matrix4 & mat1)
{
    return Matrix4(mulPerElem(mat0.getCol0(), mat1.getCol0()),
                   mulPerElem(mat0.getCol1(), mat1.getCol1()),
                   mulPerElem(mat0.getCol2(), mat1.getCol2()),
                   mulPerElem(mat0.getCol3(), mat1.getCol3()));
}

inline const Matrix4 Matrix4::identity()
{
    return Matrix4(Vector4::xAxis(),
                   Vector4::yAxis(),
                   Vector4::zAxis(),
                   Vector4::wAxis());
}

inline Matrix4 & Matrix4::setUpper3x3(const Matrix3 & mat3)
{
    mCol0.setXYZ(mat3.getCol0());
    mCol1.setXYZ(mat3.getCol1());
    mCol2.setXYZ(mat3.getCol2());
    return *this;
}

inline const Matrix3 Matrix4::getUpper3x3() const
{
    return Matrix3(mCol0.getXYZ(), mCol1.getXYZ(), mCol2.getXYZ());
}

inline Matrix4 & Matrix4::setTranslation(const Vector3 & translateVec)
{
    mCol3.setXYZ(translateVec);
    return *this;
}

inline const Vector3 Matrix4::getTranslation() const
{
    return mCol3.getXYZ();
}

inline const Matrix4 Matrix4::rotationX(float radians)
{
    return rotationX(FloatInVec(radians));
}

inline const Matrix4 Matrix4::rotationX(const FloatInVec & radians)
{
    __m128 s, c, res1, res2;
    __m128 zero;
    VECTORMATH_ALIGNED(unsigned int select_y[4]) = { 0, 0xFFFFFFFF, 0, 0 };
    VECTORMATH_ALIGNED(unsigned int select_z[4]) = { 0, 0, 0xFFFFFFFF, 0 };
    zero = _mm_setzero_ps();
    sseSinfCosf(radians.get128(), &s, &c);
    res1 = sseSelect(zero, c, select_y);
    res1 = sseSelect(res1, s, select_z);
    res2 = sseSelect(zero, sseNegatef(s), select_y);
    res2 = sseSelect(res2, c, select_z);
    return Matrix4(Vector4::xAxis(),
                   Vector4(res1),
                   Vector4(res2),
                   Vector4::wAxis());
}

inline const Matrix4 Matrix4::rotationY(float radians)
{
    return rotationY(FloatInVec(radians));
}

inline const Matrix4 Matrix4::rotationY(const FloatInVec & radians)
{
    __m128 s, c, res0, res2;
    __m128 zero;
    VECTORMATH_ALIGNED(unsigned int select_x[4]) = { 0xFFFFFFFF, 0, 0, 0 };
    VECTORMATH_ALIGNED(unsigned int select_z[4]) = { 0, 0, 0xFFFFFFFF, 0 };
    zero = _mm_setzero_ps();
    sseSinfCosf(radians.get128(), &s, &c);
    res0 = sseSelect(zero, c, select_x);
    res0 = sseSelect(res0, sseNegatef(s), select_z);
    res2 = sseSelect(zero, s, select_x);
    res2 = sseSelect(res2, c, select_z);
    return Matrix4(Vector4(res0),
                   Vector4::yAxis(),
                   Vector4(res2),
                   Vector4::wAxis());
}

inline const Matrix4 Matrix4::rotationZ(float radians)
{
    return rotationZ(FloatInVec(radians));
}

inline const Matrix4 Matrix4::rotationZ(const FloatInVec & radians)
{
    __m128 s, c, res0, res1;
    __m128 zero;
    VECTORMATH_ALIGNED(unsigned int select_x[4]) = { 0xFFFFFFFF, 0, 0, 0 };
    VECTORMATH_ALIGNED(unsigned int select_y[4]) = { 0, 0xFFFFFFFF, 0, 0 };
    zero = _mm_setzero_ps();
    sseSinfCosf(radians.get128(), &s, &c);
    res0 = sseSelect(zero, c, select_x);
    res0 = sseSelect(res0, s, select_y);
    res1 = sseSelect(zero, sseNegatef(s), select_x);
    res1 = sseSelect(res1, c, select_y);
    return Matrix4(Vector4(res0),
                   Vector4(res1),
                   Vector4::zAxis(),
                   Vector4::wAxis());
}

inline const Matrix4 Matrix4::rotationZYX(const Vector3 & radiansXYZ)
{
    __m128 angles, s, negS, c, X0, X1, Y0, Y1, Z0, Z1, tmp;
    angles = Vector4(radiansXYZ, 0.0f).get128();
    sseSinfCosf(angles, &s, &c);
    negS = sseNegatef(s);
    Z0 = sseMergeL(c, s);
    Z1 = sseMergeL(negS, c);
    VECTORMATH_ALIGNED(unsigned int select_xyz[4]) = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0 };
    Z1 = _mm_and_ps(Z1, _mm_load_ps((float *)select_xyz));
    Y0 = _mm_shuffle_ps(c, negS, _MM_SHUFFLE(0, 1, 1, 1));
    Y1 = _mm_shuffle_ps(s, c, _MM_SHUFFLE(0, 1, 1, 1));
    X0 = sseSplat(s, 0);
    X1 = sseSplat(c, 0);
    tmp = _mm_mul_ps(Z0, Y1);
    return Matrix4(Vector4(_mm_mul_ps(Z0, Y0)),
                   Vector4(sseMAdd(Z1, X1, _mm_mul_ps(tmp, X0))),
                   Vector4(sseMSub(Z1, X0, _mm_mul_ps(tmp, X1))),
                   Vector4::wAxis());
}

inline const Matrix4 Matrix4::rotation(float radians, const Vector3 & unitVec)
{
    return rotation(FloatInVec(radians), unitVec);
}

inline const Matrix4 Matrix4::rotation(const FloatInVec & radians, const Vector3 & unitVec)
{
    __m128 axis, s, c, oneMinusC, axisS, negAxisS, xxxx, yyyy, zzzz, tmp0, tmp1, tmp2;
    axis = unitVec.get128();
    sseSinfCosf(radians.get128(), &s, &c);
    xxxx = sseSplat(axis, 0);
    yyyy = sseSplat(axis, 1);
    zzzz = sseSplat(axis, 2);
    oneMinusC = _mm_sub_ps(_mm_set1_ps(1.0f), c);
    axisS = _mm_mul_ps(axis, s);
    negAxisS = sseNegatef(axisS);
    VECTORMATH_ALIGNED(unsigned int select_x[4]) = { 0xFFFFFFFF, 0, 0, 0 };
    VECTORMATH_ALIGNED(unsigned int select_y[4]) = { 0, 0xFFFFFFFF, 0, 0 };
    VECTORMATH_ALIGNED(unsigned int select_z[4]) = { 0, 0, 0xFFFFFFFF, 0 };
    tmp0 = _mm_shuffle_ps(axisS, axisS, _MM_SHUFFLE(0, 0, 2, 0));
    tmp0 = sseSelect(tmp0, sseSplat(negAxisS, 1), select_z);
    tmp1 = sseSelect(sseSplat(axisS, 0), sseSplat(negAxisS, 2), select_x);
    tmp2 = _mm_shuffle_ps(axisS, axisS, _MM_SHUFFLE(0, 0, 0, 1));
    tmp2 = sseSelect(tmp2, sseSplat(negAxisS, 0), select_y);
    tmp0 = sseSelect(tmp0, c, select_x);
    tmp1 = sseSelect(tmp1, c, select_y);
    tmp2 = sseSelect(tmp2, c, select_z);
    VECTORMATH_ALIGNED(unsigned int select_xyz[4]) = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0 };
    axis = _mm_and_ps(axis, _mm_load_ps((float *)select_xyz));
    tmp0 = _mm_and_ps(tmp0, _mm_load_ps((float *)select_xyz));
    tmp1 = _mm_and_ps(tmp1, _mm_load_ps((float *)select_xyz));
    tmp2 = _mm_and_ps(tmp2, _mm_load_ps((float *)select_xyz));
    return Matrix4(Vector4(sseMAdd(_mm_mul_ps(axis, xxxx), oneMinusC, tmp0)),
                   Vector4(sseMAdd(_mm_mul_ps(axis, yyyy), oneMinusC, tmp1)),
                   Vector4(sseMAdd(_mm_mul_ps(axis, zzzz), oneMinusC, tmp2)),
                   Vector4::wAxis());
}

inline const Matrix4 Matrix4::rotation(const Quat & unitQuat)
{
    return Matrix4(Transform3::rotation(unitQuat));
}

inline const Matrix4 Matrix4::scale(const Vector3 & scaleVec)
{
    __m128 zero = _mm_setzero_ps();
    VECTORMATH_ALIGNED(unsigned int select_x[4]) = { 0xFFFFFFFF, 0, 0, 0 };
    VECTORMATH_ALIGNED(unsigned int select_y[4]) = { 0, 0xFFFFFFFF, 0, 0 };
    VECTORMATH_ALIGNED(unsigned int select_z[4]) = { 0, 0, 0xFFFFFFFF, 0 };
    return Matrix4(Vector4(sseSelect(zero, scaleVec.get128(), select_x)),
                   Vector4(sseSelect(zero, scaleVec.get128(), select_y)),
                   Vector4(sseSelect(zero, scaleVec.get128(), select_z)),
                   Vector4::wAxis());
}

inline const Matrix4 appendScale(const Matrix4 & mat, const Vector3 & scaleVec)
{
    return Matrix4((mat.getCol0() * scaleVec.getX()),
                   (mat.getCol1() * scaleVec.getY()),
                   (mat.getCol2() * scaleVec.getZ()),
                   mat.getCol3());
}

inline const Matrix4 prependScale(const Vector3 & scaleVec, const Matrix4 & mat)
{
    const Vector4 scale4 = Vector4(scaleVec, 1.0f);
    return Matrix4(mulPerElem(mat.getCol0(), scale4),
                   mulPerElem(mat.getCol1(), scale4),
                   mulPerElem(mat.getCol2(), scale4),
                   mulPerElem(mat.getCol3(), scale4));
}

inline const Matrix4 Matrix4::translation(const Vector3 & translateVec)
{
    return Matrix4(Vector4::xAxis(),
                   Vector4::yAxis(),
                   Vector4::zAxis(),
                   Vector4(translateVec, 1.0f));
}

inline const Matrix4 Matrix4::lookAt(const Point3 & eyePos, const Point3 & lookAtPos, const Vector3 & upVec)
{
    Matrix4 m4EyeFrame;
    Vector3 v3X, v3Y, v3Z;
    v3Y = normalize(upVec);
    v3Z = normalize((eyePos - lookAtPos));
    v3X = normalize(cross(v3Y, v3Z));
    v3Y = cross(v3Z, v3X);
    m4EyeFrame = Matrix4(Vector4(v3X), Vector4(v3Y), Vector4(v3Z), Vector4(eyePos));
    return orthoInverse(m4EyeFrame);
}

inline const Matrix4 Matrix4::perspective(float fovyRadians, float aspect, float zNear, float zFar)
{
    static const float VECTORMATH_PI_OVER_2 = 1.570796327f;

    float f, rangeInv;
    SSEFloat tmp;
    __m128 col0, col1, col2, col3;

    f = tanf(VECTORMATH_PI_OVER_2 - fovyRadians * 0.5f);
    rangeInv = 1.0f / (zNear - zFar);
    const __m128 zero = _mm_setzero_ps();
    tmp.m128 = zero;
    tmp.f[0] = f / aspect;
    col0 = tmp.m128;
    tmp.m128 = zero;
    tmp.f[1] = f;
    col1 = tmp.m128;
    tmp.m128 = zero;
    tmp.f[2] = (zNear + zFar) * rangeInv;
    tmp.f[3] = -1.0f;
    col2 = tmp.m128;
    tmp.m128 = zero;
    tmp.f[2] = zNear * zFar * rangeInv * 2.0f;
    col3 = tmp.m128;

    return Matrix4(Vector4(col0), Vector4(col1), Vector4(col2), Vector4(col3));
}

inline const Matrix4 Matrix4::frustum(float left, float right, float bottom, float top, float zNear, float zFar)
{
    /* function implementation based on code from STIDC SDK:           */
    /* --------------------------------------------------------------  */
    /* PLEASE DO NOT MODIFY THIS SECTION                               */
    /* This prolog section is automatically generated.                 */
    /*                                                                 */
    /* (C)Copyright                                                    */
    /* Sony Computer Entertainment, Inc.,                              */
    /* Toshiba Corporation,                                            */
    /* International Business Machines Corporation,                    */
    /* 2001,2002.                                                      */
    /* S/T/I Confidential Information                                  */
    /* --------------------------------------------------------------  */
    __m128 lbf, rtn;
    __m128 diff, sum, inv_diff;
    __m128 diagonal, column, near2;
    __m128 zero = _mm_setzero_ps();
    SSEFloat l, f, r, n, b, t;
    l.f[0] = left;
    f.f[0] = zFar;
    r.f[0] = right;
    n.f[0] = zNear;
    b.f[0] = bottom;
    t.f[0] = top;
    lbf = sseMergeH(l.m128, f.m128);
    rtn = sseMergeH(r.m128, n.m128);
    lbf = sseMergeH(lbf, b.m128);
    rtn = sseMergeH(rtn, t.m128);
    diff = _mm_sub_ps(rtn, lbf);
    sum = _mm_add_ps(rtn, lbf);
    inv_diff = sseRecipf(diff);
    near2 = sseSplat(n.m128, 0);
    near2 = _mm_add_ps(near2, near2);
    diagonal = _mm_mul_ps(near2, inv_diff);
    column = _mm_mul_ps(sum, inv_diff);
    VECTORMATH_ALIGNED(unsigned int select_x[4]) = { 0xFFFFFFFF, 0, 0, 0 };
    VECTORMATH_ALIGNED(unsigned int select_y[4]) = { 0, 0xFFFFFFFF, 0, 0 };
    VECTORMATH_ALIGNED(unsigned int select_z[4]) = { 0, 0, 0xFFFFFFFF, 0 };
    VECTORMATH_ALIGNED(unsigned int select_w[4]) = { 0, 0, 0, 0xFFFFFFFF };
    return Matrix4(Vector4(sseSelect(zero, diagonal, select_x)),
                   Vector4(sseSelect(zero, diagonal, select_y)),
                   Vector4(sseSelect(column, _mm_set1_ps(-1.0f), select_w)),
                   Vector4(sseSelect(zero, _mm_mul_ps(diagonal, sseSplat(f.m128, 0)), select_z)));
}

inline const Matrix4 Matrix4::orthographic(float left, float right, float bottom, float top, float zNear, float zFar)
{
    /* function implementation based on code from STIDC SDK:           */
    /* --------------------------------------------------------------  */
    /* PLEASE DO NOT MODIFY THIS SECTION                               */
    /* This prolog section is automatically generated.                 */
    /*                                                                 */
    /* (C)Copyright                                                    */
    /* Sony Computer Entertainment, Inc.,                              */
    /* Toshiba Corporation,                                            */
    /* International Business Machines Corporation,                    */
    /* 2001,2002.                                                      */
    /* S/T/I Confidential Information                                  */
    /* --------------------------------------------------------------  */
    __m128 lbf, rtn;
    __m128 diff, sum, inv_diff, neg_inv_diff;
    __m128 diagonal, column;
    __m128 zero = _mm_setzero_ps();
    SSEFloat l, f, r, n, b, t;
    l.f[0] = left;
    f.f[0] = zFar;
    r.f[0] = right;
    n.f[0] = zNear;
    b.f[0] = bottom;
    t.f[0] = top;
    lbf = sseMergeH(l.m128, f.m128);
    rtn = sseMergeH(r.m128, n.m128);
    lbf = sseMergeH(lbf, b.m128);
    rtn = sseMergeH(rtn, t.m128);
    diff = _mm_sub_ps(rtn, lbf);
    sum = _mm_add_ps(rtn, lbf);
    inv_diff = sseRecipf(diff);
    neg_inv_diff = sseNegatef(inv_diff);
    diagonal = _mm_add_ps(inv_diff, inv_diff);
    VECTORMATH_ALIGNED(unsigned int select_x[4]) = { 0xFFFFFFFF, 0, 0, 0 };
    VECTORMATH_ALIGNED(unsigned int select_y[4]) = { 0, 0xFFFFFFFF, 0, 0 };
    VECTORMATH_ALIGNED(unsigned int select_z[4]) = { 0, 0, 0xFFFFFFFF, 0 };
    VECTORMATH_ALIGNED(unsigned int select_w[4]) = { 0, 0, 0, 0xFFFFFFFF };
    column = _mm_mul_ps(sum, sseSelect(neg_inv_diff, inv_diff, select_z));
    return Matrix4(Vector4(sseSelect(zero, diagonal, select_x)),
                   Vector4(sseSelect(zero, diagonal, select_y)),
                   Vector4(sseSelect(zero, diagonal, select_z)),
                   Vector4(sseSelect(column, _mm_set1_ps(1.0f), select_w)));
}

inline const Matrix4 select(const Matrix4 & mat0, const Matrix4 & mat1, bool select1)
{
    return Matrix4(select(mat0.getCol0(), mat1.getCol0(), select1),
                   select(mat0.getCol1(), mat1.getCol1(), select1),
                   select(mat0.getCol2(), mat1.getCol2(), select1),
                   select(mat0.getCol3(), mat1.getCol3(), select1));
}

inline const Matrix4 select(const Matrix4 & mat0, const Matrix4 & mat1, const BoolInVec & select1)
{
    return Matrix4(select(mat0.getCol0(), mat1.getCol0(), select1),
                   select(mat0.getCol1(), mat1.getCol1(), select1),
                   select(mat0.getCol2(), mat1.getCol2(), select1),
                   select(mat0.getCol3(), mat1.getCol3(), select1));
}

#ifdef VECTORMATH_DEBUG

inline void print(const Matrix4 & mat)
{
    print(mat.getRow(0));
    print(mat.getRow(1));
    print(mat.getRow(2));
    print(mat.getRow(3));
}

inline void print(const Matrix4 & mat, const char * name)
{
    printf("%s:\n", name);
    print(mat);
}

#endif // VECTORMATH_DEBUG

// ========================================================
// Transform3
// ========================================================

inline Transform3::Transform3(const Transform3 & tfrm)
{
    mCol0 = tfrm.mCol0;
    mCol1 = tfrm.mCol1;
    mCol2 = tfrm.mCol2;
    mCol3 = tfrm.mCol3;
}

inline Transform3::Transform3(float scalar)
{
    mCol0 = Vector3(scalar);
    mCol1 = Vector3(scalar);
    mCol2 = Vector3(scalar);
    mCol3 = Vector3(scalar);
}

inline Transform3::Transform3(const FloatInVec & scalar)
{
    mCol0 = Vector3(scalar);
    mCol1 = Vector3(scalar);
    mCol2 = Vector3(scalar);
    mCol3 = Vector3(scalar);
}

inline Transform3::Transform3(const Vector3 & _col0, const Vector3 & _col1, const Vector3 & _col2, const Vector3 & _col3)
{
    mCol0 = _col0;
    mCol1 = _col1;
    mCol2 = _col2;
    mCol3 = _col3;
}

inline Transform3::Transform3(const Matrix3 & tfrm, const Vector3 & translateVec)
{
    this->setUpper3x3(tfrm);
    this->setTranslation(translateVec);
}

inline Transform3::Transform3(const Quat & unitQuat, const Vector3 & translateVec)
{
    this->setUpper3x3(Matrix3(unitQuat));
    this->setTranslation(translateVec);
}

inline Transform3 & Transform3::setCol0(const Vector3 & _col0)
{
    mCol0 = _col0;
    return *this;
}

inline Transform3 & Transform3::setCol1(const Vector3 & _col1)
{
    mCol1 = _col1;
    return *this;
}

inline Transform3 & Transform3::setCol2(const Vector3 & _col2)
{
    mCol2 = _col2;
    return *this;
}

inline Transform3 & Transform3::setCol3(const Vector3 & _col3)
{
    mCol3 = _col3;
    return *this;
}

inline Transform3 & Transform3::setCol(int col, const Vector3 & vec)
{
    *(&mCol0 + col) = vec;
    return *this;
}

inline Transform3 & Transform3::setRow(int row, const Vector4 & vec)
{
    mCol0.setElem(row, vec.getElem(0));
    mCol1.setElem(row, vec.getElem(1));
    mCol2.setElem(row, vec.getElem(2));
    mCol3.setElem(row, vec.getElem(3));
    return *this;
}

inline Transform3 & Transform3::setElem(int col, int row, float val)
{
    (*this)[col].setElem(row, val);
    return *this;
}

inline Transform3 & Transform3::setElem(int col, int row, const FloatInVec & val)
{
    Vector3 tmpV3_0;
    tmpV3_0 = this->getCol(col);
    tmpV3_0.setElem(row, val);
    this->setCol(col, tmpV3_0);
    return *this;
}

inline const FloatInVec Transform3::getElem(int col, int row) const
{
    return this->getCol(col).getElem(row);
}

inline const Vector3 Transform3::getCol0() const
{
    return mCol0;
}

inline const Vector3 Transform3::getCol1() const
{
    return mCol1;
}

inline const Vector3 Transform3::getCol2() const
{
    return mCol2;
}

inline const Vector3 Transform3::getCol3() const
{
    return mCol3;
}

inline const Vector3 Transform3::getCol(int col) const
{
    return *(&mCol0 + col);
}

inline const Vector4 Transform3::getRow(int row) const
{
    return Vector4(mCol0.getElem(row), mCol1.getElem(row), mCol2.getElem(row), mCol3.getElem(row));
}

inline Vector3 & Transform3::operator[](int col)
{
    return *(&mCol0 + col);
}

inline const Vector3 Transform3::operator[](int col) const
{
    return *(&mCol0 + col);
}

inline Transform3 & Transform3::operator = (const Transform3 & tfrm)
{
    mCol0 = tfrm.mCol0;
    mCol1 = tfrm.mCol1;
    mCol2 = tfrm.mCol2;
    mCol3 = tfrm.mCol3;
    return *this;
}

inline const Transform3 inverse(const Transform3 & tfrm)
{
    __m128 inv0, inv1, inv2, inv3;
    __m128 tmp0, tmp1, tmp2, tmp3, tmp4, dot, invdet;
    __m128 xxxx, yyyy, zzzz;
    tmp2 = sseVecCross(tfrm.getCol0().get128(), tfrm.getCol1().get128());
    tmp0 = sseVecCross(tfrm.getCol1().get128(), tfrm.getCol2().get128());
    tmp1 = sseVecCross(tfrm.getCol2().get128(), tfrm.getCol0().get128());
    inv3 = sseNegatef(tfrm.getCol3().get128());
    dot = sseVecDot3(tmp2, tfrm.getCol2().get128());
    dot = sseSplat(dot, 0);
    invdet = sseRecipf(dot);
    tmp3 = sseMergeH(tmp0, tmp2);
    tmp4 = sseMergeL(tmp0, tmp2);
    inv0 = sseMergeH(tmp3, tmp1);
    xxxx = sseSplat(inv3, 0);
    VECTORMATH_ALIGNED(unsigned int select_y[4]) = { 0, 0xFFFFFFFF, 0, 0 };
    inv1 = _mm_shuffle_ps(tmp3, tmp3, _MM_SHUFFLE(0, 3, 2, 2));
    inv1 = sseSelect(inv1, tmp1, select_y);
    inv2 = _mm_shuffle_ps(tmp4, tmp4, _MM_SHUFFLE(0, 1, 1, 0));
    inv2 = sseSelect(inv2, sseSplat(tmp1, 2), select_y);
    yyyy = sseSplat(inv3, 1);
    zzzz = sseSplat(inv3, 2);
    inv3 = _mm_mul_ps(inv0, xxxx);
    inv3 = sseMAdd(inv1, yyyy, inv3);
    inv3 = sseMAdd(inv2, zzzz, inv3);
    inv0 = _mm_mul_ps(inv0, invdet);
    inv1 = _mm_mul_ps(inv1, invdet);
    inv2 = _mm_mul_ps(inv2, invdet);
    inv3 = _mm_mul_ps(inv3, invdet);
    return Transform3(Vector3(inv0), Vector3(inv1), Vector3(inv2), Vector3(inv3));
}

inline const Transform3 orthoInverse(const Transform3 & tfrm)
{
    __m128 inv0, inv1, inv2, inv3;
    __m128 tmp0, tmp1;
    __m128 xxxx, yyyy, zzzz;
    tmp0 = sseMergeH(tfrm.getCol0().get128(), tfrm.getCol2().get128());
    tmp1 = sseMergeL(tfrm.getCol0().get128(), tfrm.getCol2().get128());
    inv3 = sseNegatef(tfrm.getCol3().get128());
    inv0 = sseMergeH(tmp0, tfrm.getCol1().get128());
    xxxx = sseSplat(inv3, 0);
    VECTORMATH_ALIGNED(unsigned int select_y[4]) = { 0, 0xFFFFFFFF, 0, 0 };
    inv1 = _mm_shuffle_ps(tmp0, tmp0, _MM_SHUFFLE(0, 3, 2, 2));
    inv1 = sseSelect(inv1, tfrm.getCol1().get128(), select_y);
    inv2 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(0, 1, 1, 0));
    inv2 = sseSelect(inv2, sseSplat(tfrm.getCol1().get128(), 2), select_y);
    yyyy = sseSplat(inv3, 1);
    zzzz = sseSplat(inv3, 2);
    inv3 = _mm_mul_ps(inv0, xxxx);
    inv3 = sseMAdd(inv1, yyyy, inv3);
    inv3 = sseMAdd(inv2, zzzz, inv3);
    return Transform3(Vector3(inv0), Vector3(inv1), Vector3(inv2), Vector3(inv3));
}

inline const Transform3 absPerElem(const Transform3 & tfrm)
{
    return Transform3(absPerElem(tfrm.getCol0()),
                      absPerElem(tfrm.getCol1()),
                      absPerElem(tfrm.getCol2()),
                      absPerElem(tfrm.getCol3()));
}

inline const Vector3 Transform3::operator * (const Vector3 & vec) const
{
    __m128 res;
    __m128 xxxx, yyyy, zzzz;
    xxxx = sseSplat(vec.get128(), 0);
    yyyy = sseSplat(vec.get128(), 1);
    zzzz = sseSplat(vec.get128(), 2);
    res = _mm_mul_ps(mCol0.get128(), xxxx);
    res = sseMAdd(mCol1.get128(), yyyy, res);
    res = sseMAdd(mCol2.get128(), zzzz, res);
    return Vector3(res);
}

inline const Point3 Transform3::operator * (const Point3 & pnt) const
{
    __m128 tmp0, tmp1, res;
    __m128 xxxx, yyyy, zzzz;
    xxxx = sseSplat(pnt.get128(), 0);
    yyyy = sseSplat(pnt.get128(), 1);
    zzzz = sseSplat(pnt.get128(), 2);
    tmp0 = _mm_mul_ps(mCol0.get128(), xxxx);
    tmp1 = _mm_mul_ps(mCol1.get128(), yyyy);
    tmp0 = sseMAdd(mCol2.get128(), zzzz, tmp0);
    tmp1 = _mm_add_ps(mCol3.get128(), tmp1);
    res = _mm_add_ps(tmp0, tmp1);
    return Point3(res);
}

inline const Transform3 Transform3::operator * (const Transform3 & tfrm) const
{
    return Transform3((*this * tfrm.mCol0),
                      (*this * tfrm.mCol1),
                      (*this * tfrm.mCol2),
                      Vector3((*this * Point3(tfrm.mCol3))));
}

inline Transform3 & Transform3::operator *= (const Transform3 & tfrm)
{
    *this = *this * tfrm;
    return *this;
}

inline const Transform3 mulPerElem(const Transform3 & tfrm0, const Transform3 & tfrm1)
{
    return Transform3(mulPerElem(tfrm0.getCol0(), tfrm1.getCol0()),
                      mulPerElem(tfrm0.getCol1(), tfrm1.getCol1()),
                      mulPerElem(tfrm0.getCol2(), tfrm1.getCol2()),
                      mulPerElem(tfrm0.getCol3(), tfrm1.getCol3()));
}

inline const Transform3 Transform3::identity()
{
    return Transform3(Vector3::xAxis(),
                      Vector3::yAxis(),
                      Vector3::zAxis(),
                      Vector3(0.0f));
}

inline Transform3 & Transform3::setUpper3x3(const Matrix3 & tfrm)
{
    mCol0 = tfrm.getCol0();
    mCol1 = tfrm.getCol1();
    mCol2 = tfrm.getCol2();
    return *this;
}

inline const Matrix3 Transform3::getUpper3x3() const
{
    return Matrix3(mCol0, mCol1, mCol2);
}

inline Transform3 & Transform3::setTranslation(const Vector3 & translateVec)
{
    mCol3 = translateVec;
    return *this;
}

inline const Vector3 Transform3::getTranslation() const
{
    return mCol3;
}

inline const Transform3 Transform3::rotationX(float radians)
{
    return rotationX(FloatInVec(radians));
}

inline const Transform3 Transform3::rotationX(const FloatInVec & radians)
{
    __m128 s, c, res1, res2;
    __m128 zero;
    VECTORMATH_ALIGNED(unsigned int select_y[4]) = { 0, 0xFFFFFFFF, 0, 0 };
    VECTORMATH_ALIGNED(unsigned int select_z[4]) = { 0, 0, 0xFFFFFFFF, 0 };
    zero = _mm_setzero_ps();
    sseSinfCosf(radians.get128(), &s, &c);
    res1 = sseSelect(zero, c, select_y);
    res1 = sseSelect(res1, s, select_z);
    res2 = sseSelect(zero, sseNegatef(s), select_y);
    res2 = sseSelect(res2, c, select_z);
    return Transform3(Vector3::xAxis(),
                      Vector3(res1),
                      Vector3(res2),
                      Vector3(_mm_setzero_ps()));
}

inline const Transform3 Transform3::rotationY(float radians)
{
    return rotationY(FloatInVec(radians));
}

inline const Transform3 Transform3::rotationY(const FloatInVec & radians)
{
    __m128 s, c, res0, res2;
    __m128 zero;
    VECTORMATH_ALIGNED(unsigned int select_x[4]) = { 0xFFFFFFFF, 0, 0, 0 };
    VECTORMATH_ALIGNED(unsigned int select_z[4]) = { 0, 0, 0xFFFFFFFF, 0 };
    zero = _mm_setzero_ps();
    sseSinfCosf(radians.get128(), &s, &c);
    res0 = sseSelect(zero, c, select_x);
    res0 = sseSelect(res0, sseNegatef(s), select_z);
    res2 = sseSelect(zero, s, select_x);
    res2 = sseSelect(res2, c, select_z);
    return Transform3(Vector3(res0),
                      Vector3::yAxis(),
                      Vector3(res2),
                      Vector3(0.0f));
}

inline const Transform3 Transform3::rotationZ(float radians)
{
    return rotationZ(FloatInVec(radians));
}

inline const Transform3 Transform3::rotationZ(const FloatInVec & radians)
{
    __m128 s, c, res0, res1;
    VECTORMATH_ALIGNED(unsigned int select_x[4]) = { 0xFFFFFFFF, 0, 0, 0 };
    VECTORMATH_ALIGNED(unsigned int select_y[4]) = { 0, 0xFFFFFFFF, 0, 0 };
    __m128 zero = _mm_setzero_ps();
    sseSinfCosf(radians.get128(), &s, &c);
    res0 = sseSelect(zero, c, select_x);
    res0 = sseSelect(res0, s, select_y);
    res1 = sseSelect(zero, sseNegatef(s), select_x);
    res1 = sseSelect(res1, c, select_y);
    return Transform3(Vector3(res0),
                      Vector3(res1),
                      Vector3::zAxis(),
                      Vector3(0.0f));
}

inline const Transform3 Transform3::rotationZYX(const Vector3 & radiansXYZ)
{
    __m128 angles, s, negS, c, X0, X1, Y0, Y1, Z0, Z1, tmp;
    angles = Vector4(radiansXYZ, 0.0f).get128();
    sseSinfCosf(angles, &s, &c);
    negS = sseNegatef(s);
    Z0 = sseMergeL(c, s);
    Z1 = sseMergeL(negS, c);
    VECTORMATH_ALIGNED(unsigned int select_xyz[4]) = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0 };
    Z1 = _mm_and_ps(Z1, _mm_load_ps((float *)select_xyz));
    Y0 = _mm_shuffle_ps(c, negS, _MM_SHUFFLE(0, 1, 1, 1));
    Y1 = _mm_shuffle_ps(s, c, _MM_SHUFFLE(0, 1, 1, 1));
    X0 = sseSplat(s, 0);
    X1 = sseSplat(c, 0);
    tmp = _mm_mul_ps(Z0, Y1);
    return Transform3(Vector3(_mm_mul_ps(Z0, Y0)),
                      Vector3(sseMAdd(Z1, X1, _mm_mul_ps(tmp, X0))),
                      Vector3(sseMSub(Z1, X0, _mm_mul_ps(tmp, X1))),
                      Vector3(0.0f));
}

inline const Transform3 Transform3::rotation(float radians, const Vector3 & unitVec)
{
    return rotation(FloatInVec(radians), unitVec);
}

inline const Transform3 Transform3::rotation(const FloatInVec & radians, const Vector3 & unitVec)
{
    return Transform3(Matrix3::rotation(radians, unitVec), Vector3(0.0f));
}

inline const Transform3 Transform3::rotation(const Quat & unitQuat)
{
    return Transform3(Matrix3(unitQuat), Vector3(0.0f));
}

inline const Transform3 Transform3::scale(const Vector3 & scaleVec)
{
    __m128 zero = _mm_setzero_ps();
    VECTORMATH_ALIGNED(unsigned int select_x[4]) = { 0xFFFFFFFF, 0, 0, 0 };
    VECTORMATH_ALIGNED(unsigned int select_y[4]) = { 0, 0xFFFFFFFF, 0, 0 };
    VECTORMATH_ALIGNED(unsigned int select_z[4]) = { 0, 0, 0xFFFFFFFF, 0 };
    return Transform3(Vector3(sseSelect(zero, scaleVec.get128(), select_x)),
                      Vector3(sseSelect(zero, scaleVec.get128(), select_y)),
                      Vector3(sseSelect(zero, scaleVec.get128(), select_z)),
                      Vector3(0.0f));
}

inline const Transform3 appendScale(const Transform3 & tfrm, const Vector3 & scaleVec)
{
    return Transform3((tfrm.getCol0() * scaleVec.getX()),
                      (tfrm.getCol1() * scaleVec.getY()),
                      (tfrm.getCol2() * scaleVec.getZ()),
                      tfrm.getCol3());
}

inline const Transform3 prependScale(const Vector3 & scaleVec, const Transform3 & tfrm)
{
    return Transform3(mulPerElem(tfrm.getCol0(), scaleVec),
                      mulPerElem(tfrm.getCol1(), scaleVec),
                      mulPerElem(tfrm.getCol2(), scaleVec),
                      mulPerElem(tfrm.getCol3(), scaleVec));
}

inline const Transform3 Transform3::translation(const Vector3 & translateVec)
{
    return Transform3(Vector3::xAxis(),
                      Vector3::yAxis(),
                      Vector3::zAxis(),
                      translateVec);
}

inline const Transform3 select(const Transform3 & tfrm0, const Transform3 & tfrm1, bool select1)
{
    return Transform3(select(tfrm0.getCol0(), tfrm1.getCol0(), select1),
                      select(tfrm0.getCol1(), tfrm1.getCol1(), select1),
                      select(tfrm0.getCol2(), tfrm1.getCol2(), select1),
                      select(tfrm0.getCol3(), tfrm1.getCol3(), select1));
}

inline const Transform3 select(const Transform3 & tfrm0, const Transform3 & tfrm1, const BoolInVec & select1)
{
    return Transform3(select(tfrm0.getCol0(), tfrm1.getCol0(), select1),
                      select(tfrm0.getCol1(), tfrm1.getCol1(), select1),
                      select(tfrm0.getCol2(), tfrm1.getCol2(), select1),
                      select(tfrm0.getCol3(), tfrm1.getCol3(), select1));
}

#ifdef VECTORMATH_DEBUG

inline void print(const Transform3 & tfrm)
{
    print(tfrm.getRow(0));
    print(tfrm.getRow(1));
    print(tfrm.getRow(2));
}

inline void print(const Transform3 & tfrm, const char * name)
{
    printf("%s:\n", name);
    print(tfrm);
}

#endif // VECTORMATH_DEBUG

// ========================================================
// Quat
// ========================================================

inline Quat::Quat(const Matrix3 & tfrm)
{
    __m128 res;
    __m128 col0, col1, col2;
    __m128 xx_yy, xx_yy_zz_xx, yy_zz_xx_yy, zz_xx_yy_zz, diagSum, diagDiff;
    __m128 zy_xz_yx, yz_zx_xy, sum, diff;
    __m128 radicand, invSqrt, scale;
    __m128 res0, res1, res2, res3;
    __m128 xx, yy, zz;

    VECTORMATH_ALIGNED(unsigned int select_x[4]) = { 0xFFFFFFFF, 0, 0, 0 };
    VECTORMATH_ALIGNED(unsigned int select_y[4]) = { 0, 0xFFFFFFFF, 0, 0 };
    VECTORMATH_ALIGNED(unsigned int select_z[4]) = { 0, 0, 0xFFFFFFFF, 0 };
    VECTORMATH_ALIGNED(unsigned int select_w[4]) = { 0, 0, 0, 0xFFFFFFFF };

    col0 = tfrm.getCol0().get128();
    col1 = tfrm.getCol1().get128();
    col2 = tfrm.getCol2().get128();

    /* four cases: */
    /* trace > 0 */
    /* else */
    /*    xx largest diagonal element */
    /*    yy largest diagonal element */
    /*    zz largest diagonal element */
    /* compute quaternion for each case */

    xx_yy = sseSelect(col0, col1, select_y);
    xx_yy_zz_xx = _mm_shuffle_ps(xx_yy, xx_yy, _MM_SHUFFLE(0, 0, 1, 0));
    xx_yy_zz_xx = sseSelect(xx_yy_zz_xx, col2, select_z);
    yy_zz_xx_yy = _mm_shuffle_ps(xx_yy_zz_xx, xx_yy_zz_xx, _MM_SHUFFLE(1, 0, 2, 1));
    zz_xx_yy_zz = _mm_shuffle_ps(xx_yy_zz_xx, xx_yy_zz_xx, _MM_SHUFFLE(2, 1, 0, 2));

    diagSum  = _mm_add_ps(_mm_add_ps(xx_yy_zz_xx, yy_zz_xx_yy), zz_xx_yy_zz);
    diagDiff = _mm_sub_ps(_mm_sub_ps(xx_yy_zz_xx, yy_zz_xx_yy), zz_xx_yy_zz);
    radicand = _mm_add_ps(sseSelect(diagDiff, diagSum, select_w), _mm_set1_ps(1.0f));
    //invSqrt = sseRSqrtf(radicand);
    invSqrt = sseNewtonrapsonRSqrtf(radicand);

    zy_xz_yx = sseSelect(col0, col1, select_z);                             // zy_xz_yx = 00 01 12 03
    zy_xz_yx = _mm_shuffle_ps(zy_xz_yx, zy_xz_yx, _MM_SHUFFLE(0, 1, 2, 2)); // zy_xz_yx = 12 12 01 00
    zy_xz_yx = sseSelect(zy_xz_yx, sseSplat(col2, 0), select_y);            // zy_xz_yx = 12 20 01 00
    yz_zx_xy = sseSelect(col0, col1, select_x);                             // yz_zx_xy = 10 01 02 03
    yz_zx_xy = _mm_shuffle_ps(yz_zx_xy, yz_zx_xy, _MM_SHUFFLE(0, 0, 2, 0)); // yz_zx_xy = 10 02 10 10
    yz_zx_xy = sseSelect(yz_zx_xy, sseSplat(col2, 1), select_x);            // yz_zx_xy = 21 02 10 10

    sum = _mm_add_ps(zy_xz_yx, yz_zx_xy);
    diff = _mm_sub_ps(zy_xz_yx, yz_zx_xy);
    scale = _mm_mul_ps(invSqrt, _mm_set1_ps(0.5f));

    res0 = _mm_shuffle_ps(sum, sum, _MM_SHUFFLE(0, 1, 2, 0));
    res0 = sseSelect(res0, sseSplat(diff, 0), select_w);
    res1 = _mm_shuffle_ps(sum, sum, _MM_SHUFFLE(0, 0, 0, 2));
    res1 = sseSelect(res1, sseSplat(diff, 1), select_w);
    res2 = _mm_shuffle_ps(sum, sum, _MM_SHUFFLE(0, 0, 0, 1));
    res2 = sseSelect(res2, sseSplat(diff, 2), select_w);
    res3 = diff;
    res0 = sseSelect(res0, radicand, select_x);
    res1 = sseSelect(res1, radicand, select_y);
    res2 = sseSelect(res2, radicand, select_z);
    res3 = sseSelect(res3, radicand, select_w);
    res0 = _mm_mul_ps(res0, sseSplat(scale, 0));
    res1 = _mm_mul_ps(res1, sseSplat(scale, 1));
    res2 = _mm_mul_ps(res2, sseSplat(scale, 2));
    res3 = _mm_mul_ps(res3, sseSplat(scale, 3));

    /* determine case and select answer */
    xx = sseSplat(col0, 0);
    yy = sseSplat(col1, 1);
    zz = sseSplat(col2, 2);
    res = sseSelect(res0, res1, _mm_cmpgt_ps(yy, xx));
    res = sseSelect(res, res2, _mm_and_ps(_mm_cmpgt_ps(zz, xx), _mm_cmpgt_ps(zz, yy)));
    res = sseSelect(res, res3, _mm_cmpgt_ps(sseSplat(diagSum, 0), _mm_setzero_ps()));
    mVec128 = res;
}

// ========================================================
// Misc free functions
// ========================================================

inline const Matrix3 outer(const Vector3 & tfrm0, const Vector3 & tfrm1)
{
    return Matrix3((tfrm0 * tfrm1.getX()),
                   (tfrm0 * tfrm1.getY()),
                   (tfrm0 * tfrm1.getZ()));
}

inline const Matrix4 outer(const Vector4 & tfrm0, const Vector4 & tfrm1)
{
    return Matrix4((tfrm0 * tfrm1.getX()),
                   (tfrm0 * tfrm1.getY()),
                   (tfrm0 * tfrm1.getZ()),
                   (tfrm0 * tfrm1.getW()));
}

inline const Vector3 rowMul(const Vector3 & vec, const Matrix3 & mat)
{
    __m128 tmp0, tmp1, mcol0, mcol1, mcol2, res;
    __m128 xxxx, yyyy, zzzz;
    tmp0 = sseMergeH(mat.getCol0().get128(), mat.getCol2().get128());
    tmp1 = sseMergeL(mat.getCol0().get128(), mat.getCol2().get128());
    xxxx = sseSplat(vec.get128(), 0);
    mcol0 = sseMergeH(tmp0, mat.getCol1().get128());
    VECTORMATH_ALIGNED(unsigned int select_y[4]) = { 0, 0xFFFFFFFF, 0, 0 };
    mcol1 = _mm_shuffle_ps(tmp0, tmp0, _MM_SHUFFLE(0, 3, 2, 2));
    mcol1 = sseSelect(mcol1, mat.getCol1().get128(), select_y);
    mcol2 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(0, 1, 1, 0));
    mcol2 = sseSelect(mcol2, sseSplat(mat.getCol1().get128(), 2), select_y);
    yyyy = sseSplat(vec.get128(), 1);
    res = _mm_mul_ps(mcol0, xxxx);
    zzzz = sseSplat(vec.get128(), 2);
    res = sseMAdd(mcol1, yyyy, res);
    res = sseMAdd(mcol2, zzzz, res);
    return Vector3(res);
}

inline const Matrix3 crossMatrix(const Vector3 & vec)
{
    __m128 neg, res0, res1, res2;
    neg = sseNegatef(vec.get128());
    VECTORMATH_ALIGNED(unsigned int select_x[4]) = { 0xFFFFFFFF, 0, 0, 0 };
    VECTORMATH_ALIGNED(unsigned int select_y[4]) = { 0, 0xFFFFFFFF, 0, 0 };
    VECTORMATH_ALIGNED(unsigned int select_z[4]) = { 0, 0, 0xFFFFFFFF, 0 };
    res0 = _mm_shuffle_ps(vec.get128(), vec.get128(), _MM_SHUFFLE(0, 2, 2, 0));
    res0 = sseSelect(res0, sseSplat(neg, 1), select_z);
    res1 = sseSelect(sseSplat(vec.get128(), 0), sseSplat(neg, 2), select_x);
    res2 = _mm_shuffle_ps(vec.get128(), vec.get128(), _MM_SHUFFLE(0, 0, 1, 1));
    res2 = sseSelect(res2, sseSplat(neg, 0), select_y);
    VECTORMATH_ALIGNED(unsigned int filter_x[4]) = { 0, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };
    VECTORMATH_ALIGNED(unsigned int filter_y[4]) = { 0xFFFFFFFF, 0, 0xFFFFFFFF, 0xFFFFFFFF };
    VECTORMATH_ALIGNED(unsigned int filter_z[4]) = { 0xFFFFFFFF, 0xFFFFFFFF, 0, 0xFFFFFFFF };
    res0 = _mm_and_ps(res0, _mm_load_ps((float *)filter_x));
    res1 = _mm_and_ps(res1, _mm_load_ps((float *)filter_y));
    res2 = _mm_and_ps(res2, _mm_load_ps((float *)filter_z));
    return Matrix3(Vector3(res0), Vector3(res1), Vector3(res2));
}

inline const Matrix3 crossMatrixMul(const Vector3 & vec, const Matrix3 & mat)
{
    return Matrix3(cross(vec, mat.getCol0()), cross(vec, mat.getCol1()), cross(vec, mat.getCol2()));
}

} // namespace SSE
} // namespace Vectormath

#endif // VECTORMATH_SSE_MATRIX_HPP

#endif // VECTORMATH_SSE_VECTORMATH_HPP

    using namespace Vectormath::SSE;
    #define VECTORMATH_MODE_SCALAR 0
    #define VECTORMATH_MODE_SSE    1
#else // !SSE

#ifndef VECTORMATH_SCALAR_VECTORMATH_HPP
#define VECTORMATH_SCALAR_VECTORMATH_HPP

#include <cmath>

#ifdef VECTORMATH_DEBUG
    #include <cstdio>
#endif // VECTORMATH_DEBUG

#if defined(_MSC_VER)
    // Visual Studio (MS compiler)
    #define VECTORMATH_ALIGNED(type)      __declspec(align(16)) type
    #define VECTORMATH_ALIGNED_TYPE_PRE   __declspec(align(16))
    #define VECTORMATH_ALIGNED_TYPE_POST  /* nothing */
#elif defined(__GNUC__)
    // GCC or Clang
    #define VECTORMATH_ALIGNED(type)      type __attribute__((aligned(16)))
    #define VECTORMATH_ALIGNED_TYPE_PRE   /* nothing */
    #define VECTORMATH_ALIGNED_TYPE_POST  __attribute__((aligned(16)))
#else
    // Unknown compiler
    #error "Define VECTORMATH_ALIGNED for your compiler or platform!"
#endif

namespace Vectormath
{
namespace Scalar
{

// ========================================================
// Forward Declarations
// ========================================================

class Vector3;
class Vector4;
class Point3;
class Quat;
class Matrix3;
class Matrix4;
class Transform3;

// ========================================================
// A 3-D vector in array-of-structures format
// ========================================================

VECTORMATH_ALIGNED_TYPE_PRE class Vector3
{
    float mX;
    float mY;
    float mZ;
    float mW;

public:

    // Default constructor; does no initialization
    inline Vector3() { }

    // Copy a 3-D vector
    inline Vector3(const Vector3 & vec);

    // Construct a 3-D vector from x, y, and z elements
    inline Vector3(float x, float y, float z);

    // Copy elements from a 3-D point into a 3-D vector
    explicit inline Vector3(const Point3 & pnt);

    // Set all elements of a 3-D vector to the same scalar value
    explicit inline Vector3(float scalar);

    // Assign one 3-D vector to another
    inline Vector3 & operator = (const Vector3 & vec);

    // Set the x element of a 3-D vector
    inline Vector3 & setX(float x);

    // Set the y element of a 3-D vector
    inline Vector3 & setY(float y);

    // Set the z element of a 3-D vector
    inline Vector3 & setZ(float z);

    // Set the w element of a padded 3-D vector
    // NOTE:
    // You are free to use the additional w component - if never set, it's value is undefined.
    inline Vector3 & setW(float w);

    // Get the x element of a 3-D vector
    inline float getX() const;

    // Get the y element of a 3-D vector
    inline float getY() const;

    // Get the z element of a 3-D vector
    inline float getZ() const;

    // Get the w element of a padded 3-D vector
    // NOTE:
    // You are free to use the additional w component - if never set, it's value is undefined.
    inline float getW() const;

    // Set an x, y, or z element of a 3-D vector by index
    inline Vector3 & setElem(int idx, float value);

    // Get an x, y, or z element of a 3-D vector by index
    inline float getElem(int idx) const;

    // Subscripting operator to set or get an element
    inline float & operator[](int idx);

    // Subscripting operator to get an element
    inline float operator[](int idx) const;

    // Add two 3-D vectors
    inline const Vector3 operator + (const Vector3 & vec) const;

    // Subtract a 3-D vector from another 3-D vector
    inline const Vector3 operator - (const Vector3 & vec) const;

    // Add a 3-D vector to a 3-D point
    inline const Point3 operator + (const Point3 & pnt) const;

    // Multiply a 3-D vector by a scalar
    inline const Vector3 operator * (float scalar) const;

    // Divide a 3-D vector by a scalar
    inline const Vector3 operator / (float scalar) const;

    // Perform compound assignment and addition with a 3-D vector
    inline Vector3 & operator += (const Vector3 & vec);

    // Perform compound assignment and subtraction by a 3-D vector
    inline Vector3 & operator -= (const Vector3 & vec);

    // Perform compound assignment and multiplication by a scalar
    inline Vector3 & operator *= (float scalar);

    // Perform compound assignment and division by a scalar
    inline Vector3 & operator /= (float scalar);

    // Negate all elements of a 3-D vector
    inline const Vector3 operator - () const;

    // Construct x axis
    static inline const Vector3 xAxis();

    // Construct y axis
    static inline const Vector3 yAxis();

    // Construct z axis
    static inline const Vector3 zAxis();

} VECTORMATH_ALIGNED_TYPE_POST;

// Multiply a 3-D vector by a scalar
//
inline const Vector3 operator * (float scalar, const Vector3 & vec);

// Multiply two 3-D vectors per element
//
inline const Vector3 mulPerElem(const Vector3 & vec0, const Vector3 & vec1);

// Divide two 3-D vectors per element
// NOTE:
// Floating-point behavior matches standard library function divf4.
//
inline const Vector3 divPerElem(const Vector3 & vec0, const Vector3 & vec1);

// Compute the reciprocal of a 3-D vector per element
// NOTE:
// Floating-point behavior matches standard library function recipf4.
//
inline const Vector3 recipPerElem(const Vector3 & vec);

// Compute the square root of a 3-D vector per element
// NOTE:
// Floating-point behavior matches standard library function sqrtf4.
//
inline const Vector3 sqrtPerElem(const Vector3 & vec);

// Compute the reciprocal square root of a 3-D vector per element
// NOTE:
// Floating-point behavior matches standard library function rsqrtf4.
//
inline const Vector3 rsqrtPerElem(const Vector3 & vec);

// Compute the absolute value of a 3-D vector per element
//
inline const Vector3 absPerElem(const Vector3 & vec);

// Copy sign from one 3-D vector to another, per element
//
inline const Vector3 copySignPerElem(const Vector3 & vec0, const Vector3 & vec1);

// Maximum of two 3-D vectors per element
//
inline const Vector3 maxPerElem(const Vector3 & vec0, const Vector3 & vec1);

// Minimum of two 3-D vectors per element
//
inline const Vector3 minPerElem(const Vector3 & vec0, const Vector3 & vec1);

// Maximum element of a 3-D vector
//
inline float maxElem(const Vector3 & vec);

// Minimum element of a 3-D vector
//
inline float minElem(const Vector3 & vec);

// Compute the sum of all elements of a 3-D vector
//
inline float sum(const Vector3 & vec);

// Compute the dot product of two 3-D vectors
//
inline float dot(const Vector3 & vec0, const Vector3 & vec1);

// Compute the square of the length of a 3-D vector
//
inline float lengthSqr(const Vector3 & vec);

// Compute the length of a 3-D vector
//
inline float length(const Vector3 & vec);

// Normalize a 3-D vector
// NOTE:
// The result is unpredictable when all elements of vec are at or near zero.
//
inline const Vector3 normalize(const Vector3 & vec);

// Compute cross product of two 3-D vectors
//
inline const Vector3 cross(const Vector3 & vec0, const Vector3 & vec1);

// Outer product of two 3-D vectors
//
inline const Matrix3 outer(const Vector3 & vec0, const Vector3 & vec1);

// Pre-multiply a row vector by a 3x3 matrix
//
inline const Vector3 rowMul(const Vector3 & vec, const Matrix3 & mat);

// Cross-product matrix of a 3-D vector
//
inline const Matrix3 crossMatrix(const Vector3 & vec);

// Create cross-product matrix and multiply
// NOTE:
// Faster than separately creating a cross-product matrix and multiplying.
//
inline const Matrix3 crossMatrixMul(const Vector3 & vec, const Matrix3 & mat);

// Linear interpolation between two 3-D vectors
// NOTE:
// Does not clamp t between 0 and 1.
//
inline const Vector3 lerp(float t, const Vector3 & vec0, const Vector3 & vec1);

// Spherical linear interpolation between two 3-D vectors
// NOTE:
// The result is unpredictable if the vectors point in opposite directions.
// Does not clamp t between 0 and 1.
//
inline const Vector3 slerp(float t, const Vector3 & unitVec0, const Vector3 & unitVec1);

// Conditionally select between two 3-D vectors
//
inline const Vector3 select(const Vector3 & vec0, const Vector3 & vec1, bool select1);

#ifdef VECTORMATH_DEBUG

// Print a 3-D vector
// NOTE:
// Function is only defined when VECTORMATH_DEBUG is defined.
//
inline void print(const Vector3 & vec);

// Print a 3-D vector and an associated string identifier
// NOTE:
// Function is only defined when VECTORMATH_DEBUG is defined.
//
inline void print(const Vector3 & vec, const char * name);

#endif // VECTORMATH_DEBUG

// ========================================================
// A 4-D vector in array-of-structures format
// ========================================================

VECTORMATH_ALIGNED_TYPE_PRE class Vector4
{
    float mX;
    float mY;
    float mZ;
    float mW;

public:

    // Default constructor; does no initialization
    inline Vector4() { }

    // Copy a 4-D vector
    inline Vector4(const Vector4 & vec);

    // Construct a 4-D vector from x, y, z, and w elements
    inline Vector4(float x, float y, float z, float w);

    // Construct a 4-D vector from a 3-D vector and a scalar
    inline Vector4(const Vector3 & xyz, float w);

    // Copy x, y, and z from a 3-D vector into a 4-D vector, and set w to 0
    explicit inline Vector4(const Vector3 & vec);

    // Copy x, y, and z from a 3-D point into a 4-D vector, and set w to 1
    explicit inline Vector4(const Point3 & pnt);

    // Copy elements from a quaternion into a 4-D vector
    explicit inline Vector4(const Quat & quat);

    // Set all elements of a 4-D vector to the same scalar value
    explicit inline Vector4(float scalar);

    // Assign one 4-D vector to another
    inline Vector4 & operator = (const Vector4 & vec);

    // Set the x, y, and z elements of a 4-D vector
    // NOTE:
    // This function does not change the w element.
    inline Vector4 & setXYZ(const Vector3 & vec);

    // Get the x, y, and z elements of a 4-D vector
    inline const Vector3 getXYZ() const;

    // Set the x element of a 4-D vector
    inline Vector4 & setX(float x);

    // Set the y element of a 4-D vector
    inline Vector4 & setY(float y);

    // Set the z element of a 4-D vector
    inline Vector4 & setZ(float z);

    // Set the w element of a 4-D vector
    inline Vector4 & setW(float w);

    // Get the x element of a 4-D vector
    inline float getX() const;

    // Get the y element of a 4-D vector
    inline float getY() const;

    // Get the z element of a 4-D vector
    inline float getZ() const;

    // Get the w element of a 4-D vector
    inline float getW() const;

    // Set an x, y, z, or w element of a 4-D vector by index
    inline Vector4 & setElem(int idx, float value);

    // Get an x, y, z, or w element of a 4-D vector by index
    inline float getElem(int idx) const;

    // Subscripting operator to set or get an element
    inline float & operator[](int idx);

    // Subscripting operator to get an element
    inline float operator[](int idx) const;

    // Add two 4-D vectors
    inline const Vector4 operator + (const Vector4 & vec) const;

    // Subtract a 4-D vector from another 4-D vector
    inline const Vector4 operator - (const Vector4 & vec) const;

    // Multiply a 4-D vector by a scalar
    inline const Vector4 operator * (float scalar) const;

    // Divide a 4-D vector by a scalar
    inline const Vector4 operator / (float scalar) const;

    // Perform compound assignment and addition with a 4-D vector
    inline Vector4 & operator += (const Vector4 & vec);

    // Perform compound assignment and subtraction by a 4-D vector
    inline Vector4 & operator -= (const Vector4 & vec);

    // Perform compound assignment and multiplication by a scalar
    inline Vector4 & operator *= (float scalar);

    // Perform compound assignment and division by a scalar
    inline Vector4 & operator /= (float scalar);

    // Negate all elements of a 4-D vector
    inline const Vector4 operator - () const;

    // Construct x axis
    static inline const Vector4 xAxis();

    // Construct y axis
    static inline const Vector4 yAxis();

    // Construct z axis
    static inline const Vector4 zAxis();

    // Construct w axis
    static inline const Vector4 wAxis();

} VECTORMATH_ALIGNED_TYPE_POST;

// Multiply a 4-D vector by a scalar
//
inline const Vector4 operator * (float scalar, const Vector4 & vec);

// Multiply two 4-D vectors per element
//
inline const Vector4 mulPerElem(const Vector4 & vec0, const Vector4 & vec1);

// Divide two 4-D vectors per element
// NOTE:
// Floating-point behavior matches standard library function divf4.
//
inline const Vector4 divPerElem(const Vector4 & vec0, const Vector4 & vec1);

// Compute the reciprocal of a 4-D vector per element
// NOTE:
// Floating-point behavior matches standard library function recipf4.
//
inline const Vector4 recipPerElem(const Vector4 & vec);

// Compute the square root of a 4-D vector per element
// NOTE:
// Floating-point behavior matches standard library function sqrtf4.
//
inline const Vector4 sqrtPerElem(const Vector4 & vec);

// Compute the reciprocal square root of a 4-D vector per element
// NOTE:
// Floating-point behavior matches standard library function rsqrtf4.
//
inline const Vector4 rsqrtPerElem(const Vector4 & vec);

// Compute the absolute value of a 4-D vector per element
//
inline const Vector4 absPerElem(const Vector4 & vec);

// Copy sign from one 4-D vector to another, per element
//
inline const Vector4 copySignPerElem(const Vector4 & vec0, const Vector4 & vec1);

// Maximum of two 4-D vectors per element
//
inline const Vector4 maxPerElem(const Vector4 & vec0, const Vector4 & vec1);

// Minimum of two 4-D vectors per element
//
inline const Vector4 minPerElem(const Vector4 & vec0, const Vector4 & vec1);

// Maximum element of a 4-D vector
//
inline float maxElem(const Vector4 & vec);

// Minimum element of a 4-D vector
//
inline float minElem(const Vector4 & vec);

// Compute the sum of all elements of a 4-D vector
//
inline float sum(const Vector4 & vec);

// Compute the dot product of two 4-D vectors
//
inline float dot(const Vector4 & vec0, const Vector4 & vec1);

// Compute the square of the length of a 4-D vector
//
inline float lengthSqr(const Vector4 & vec);

// Compute the length of a 4-D vector
//
inline float length(const Vector4 & vec);

// Normalize a 4-D vector
// NOTE:
// The result is unpredictable when all elements of vec are at or near zero.
//
inline const Vector4 normalize(const Vector4 & vec);

// Outer product of two 4-D vectors
//
inline const Matrix4 outer(const Vector4 & vec0, const Vector4 & vec1);

// Linear interpolation between two 4-D vectors
// NOTE:
// Does not clamp t between 0 and 1.
//
inline const Vector4 lerp(float t, const Vector4 & vec0, const Vector4 & vec1);

// Spherical linear interpolation between two 4-D vectors
// NOTE:
// The result is unpredictable if the vectors point in opposite directions.
// Does not clamp t between 0 and 1.
//
inline const Vector4 slerp(float t, const Vector4 & unitVec0, const Vector4 & unitVec1);

// Conditionally select between two 4-D vectors
//
inline const Vector4 select(const Vector4 & vec0, const Vector4 & vec1, bool select1);

#ifdef VECTORMATH_DEBUG

// Print a 4-D vector
// NOTE:
// Function is only defined when VECTORMATH_DEBUG is defined.
//
inline void print(const Vector4 & vec);

// Print a 4-D vector and an associated string identifier
// NOTE:
// Function is only defined when VECTORMATH_DEBUG is defined.
//
inline void print(const Vector4 & vec, const char * name);

#endif // VECTORMATH_DEBUG

// ========================================================
// A 3-D point in array-of-structures format
// ========================================================

VECTORMATH_ALIGNED_TYPE_PRE class Point3
{
    float mX;
    float mY;
    float mZ;
    float mW;

public:

    // Default constructor; does no initialization
    inline Point3() { }

    // Copy a 3-D point
    inline Point3(const Point3 & pnt);

    // Construct a 3-D point from x, y, and z elements
    inline Point3(float x, float y, float z);

    // Copy elements from a 3-D vector into a 3-D point
    explicit inline Point3(const Vector3 & vec);

    // Set all elements of a 3-D point to the same scalar value
    explicit inline Point3(float scalar);

    // Assign one 3-D point to another
    inline Point3 & operator = (const Point3 & pnt);

    // Set the x element of a 3-D point
    inline Point3 & setX(float x);

    // Set the y element of a 3-D point
    inline Point3 & setY(float y);

    // Set the z element of a 3-D point
    inline Point3 & setZ(float z);

    // Set the w element of a padded 3-D point
    // NOTE:
    // You are free to use the additional w component - if never set, it's value is undefined.
    inline Point3 & setW(float w);

    // Get the x element of a 3-D point
    inline float getX() const;

    // Get the y element of a 3-D point
    inline float getY() const;

    // Get the z element of a 3-D point
    inline float getZ() const;

    // Get the w element of a padded 3-D point
    // NOTE:
    // You are free to use the additional w component - if never set, it's value is undefined.
    inline float getW() const;

    // Set an x, y, or z element of a 3-D point by index
    inline Point3 & setElem(int idx, float value);

    // Get an x, y, or z element of a 3-D point by index
    inline float getElem(int idx) const;

    // Subscripting operator to set or get an element
    inline float & operator[](int idx);

    // Subscripting operator to get an element
    inline float operator[](int idx) const;

    // Subtract a 3-D point from another 3-D point
    inline const Vector3 operator - (const Point3 & pnt) const;

    // Add a 3-D point to a 3-D vector
    inline const Point3 operator + (const Vector3 & vec) const;

    // Subtract a 3-D vector from a 3-D point
    inline const Point3 operator - (const Vector3 & vec) const;

    // Perform compound assignment and addition with a 3-D vector
    inline Point3 & operator += (const Vector3 & vec);

    // Perform compound assignment and subtraction by a 3-D vector
    inline Point3 & operator -= (const Vector3 & vec);

} VECTORMATH_ALIGNED_TYPE_POST;

// Multiply two 3-D points per element
//
inline const Point3 mulPerElem(const Point3 & pnt0, const Point3 & pnt1);

// Divide two 3-D points per element
// NOTE:
// Floating-point behavior matches standard library function divf4.
//
inline const Point3 divPerElem(const Point3 & pnt0, const Point3 & pnt1);

// Compute the reciprocal of a 3-D point per element
// NOTE:
// Floating-point behavior matches standard library function recipf4.
//
inline const Point3 recipPerElem(const Point3 & pnt);

// Compute the square root of a 3-D point per element
// NOTE:
// Floating-point behavior matches standard library function sqrtf4.
//
inline const Point3 sqrtPerElem(const Point3 & pnt);

// Compute the reciprocal square root of a 3-D point per element
// NOTE:
// Floating-point behavior matches standard library function rsqrtf4.
//
inline const Point3 rsqrtPerElem(const Point3 & pnt);

// Compute the absolute value of a 3-D point per element
//
inline const Point3 absPerElem(const Point3 & pnt);

// Copy sign from one 3-D point to another, per element
//
inline const Point3 copySignPerElem(const Point3 & pnt0, const Point3 & pnt1);

// Maximum of two 3-D points per element
//
inline const Point3 maxPerElem(const Point3 & pnt0, const Point3 & pnt1);

// Minimum of two 3-D points per element
//
inline const Point3 minPerElem(const Point3 & pnt0, const Point3 & pnt1);

// Maximum element of a 3-D point
//
inline float maxElem(const Point3 & pnt);

// Minimum element of a 3-D point
//
inline float minElem(const Point3 & pnt);

// Compute the sum of all elements of a 3-D point
//
inline float sum(const Point3 & pnt);

// Apply uniform scale to a 3-D point
//
inline const Point3 scale(const Point3 & pnt, float scaleVal);

// Apply non-uniform scale to a 3-D point
//
inline const Point3 scale(const Point3 & pnt, const Vector3 & scaleVec);

// Scalar projection of a 3-D point on a unit-length 3-D vector
//
inline float projection(const Point3 & pnt, const Vector3 & unitVec);

// Compute the square of the distance of a 3-D point from the coordinate-system origin
//
inline float distSqrFromOrigin(const Point3 & pnt);

// Compute the distance of a 3-D point from the coordinate-system origin
//
inline float distFromOrigin(const Point3 & pnt);

// Compute the square of the distance between two 3-D points
//
inline float distSqr(const Point3 & pnt0, const Point3 & pnt1);

// Compute the distance between two 3-D points
//
inline float distp(const Point3 & pnt0, const Point3 & pnt1);

// Linear interpolation between two 3-D points
// NOTE:
// Does not clamp t between 0 and 1.
//
inline const Point3 lerp(float t, const Point3 & pnt0, const Point3 & pnt1);

// Conditionally select between two 3-D points
//
inline const Point3 select(const Point3 & pnt0, const Point3 & pnt1, bool select1);

#ifdef VECTORMATH_DEBUG

// Print a 3-D point
// NOTE:
// Function is only defined when VECTORMATH_DEBUG is defined.
//
inline void print(const Point3 & pnt);

// Print a 3-D point and an associated string identifier
// NOTE:
// Function is only defined when VECTORMATH_DEBUG is defined.
//
inline void print(const Point3 & pnt, const char * name);

#endif // VECTORMATH_DEBUG

// ========================================================
// A quaternion in array-of-structures format
// ========================================================

VECTORMATH_ALIGNED_TYPE_PRE class Quat
{
    float mX;
    float mY;
    float mZ;
    float mW;

public:

    // Default constructor; does no initialization
    inline Quat() { }

    // Copy a quaternion
    inline Quat(const Quat & quat);

    // Construct a quaternion from x, y, z, and w elements
    inline Quat(float x, float y, float z, float w);

    // Construct a quaternion from a 3-D vector and a scalar
    inline Quat(const Vector3 & xyz, float w);

    // Copy elements from a 4-D vector into a quaternion
    explicit inline Quat(const Vector4 & vec);

    // Convert a rotation matrix to a unit-length quaternion
    explicit inline Quat(const Matrix3 & rotMat);

    // Set all elements of a quaternion to the same scalar value
    explicit inline Quat(float scalar);

    // Assign one quaternion to another
    inline Quat & operator = (const Quat & quat);

    // Set the x, y, and z elements of a quaternion
    // NOTE:
    // This function does not change the w element.
    inline Quat & setXYZ(const Vector3 & vec);

    // Get the x, y, and z elements of a quaternion
    inline const Vector3 getXYZ() const;

    // Set the x element of a quaternion
    inline Quat & setX(float x);

    // Set the y element of a quaternion
    inline Quat & setY(float y);

    // Set the z element of a quaternion
    inline Quat & setZ(float z);

    // Set the w element of a quaternion
    inline Quat & setW(float w);

    // Get the x element of a quaternion
    inline float getX() const;

    // Get the y element of a quaternion
    inline float getY() const;

    // Get the z element of a quaternion
    inline float getZ() const;

    // Get the w element of a quaternion
    inline float getW() const;

    // Set an x, y, z, or w element of a quaternion by index
    inline Quat & setElem(int idx, float value);

    // Get an x, y, z, or w element of a quaternion by index
    inline float getElem(int idx) const;

    // Subscripting operator to set or get an element
    inline float & operator[](int idx);

    // Subscripting operator to get an element
    inline float operator[](int idx) const;

    // Add two quaternions
    inline const Quat operator + (const Quat & quat) const;

    // Subtract a quaternion from another quaternion
    inline const Quat operator - (const Quat & quat) const;

    // Multiply two quaternions
    inline const Quat operator * (const Quat & quat) const;

    // Multiply a quaternion by a scalar
    inline const Quat operator * (float scalar) const;

    // Divide a quaternion by a scalar
    inline const Quat operator / (float scalar) const;

    // Perform compound assignment and addition with a quaternion
    inline Quat & operator += (const Quat & quat);

    // Perform compound assignment and subtraction by a quaternion
    inline Quat & operator -= (const Quat & quat);

    // Perform compound assignment and multiplication by a quaternion
    inline Quat & operator *= (const Quat & quat);

    // Perform compound assignment and multiplication by a scalar
    inline Quat & operator *= (float scalar);

    // Perform compound assignment and division by a scalar
    inline Quat & operator /= (float scalar);

    // Negate all elements of a quaternion
    inline const Quat operator - () const;

    // Construct an identity quaternion
    static inline const Quat identity();

    // Construct a quaternion to rotate between two unit-length 3-D vectors
    // NOTE:
    // The result is unpredictable if unitVec0 and unitVec1 point in opposite directions.
    static inline const Quat rotation(const Vector3 & unitVec0, const Vector3 & unitVec1);

    // Construct a quaternion to rotate around a unit-length 3-D vector
    static inline const Quat rotation(float radians, const Vector3 & unitVec);

    // Construct a quaternion to rotate around the x axis
    static inline const Quat rotationX(float radians);

    // Construct a quaternion to rotate around the y axis
    static inline const Quat rotationY(float radians);

    // Construct a quaternion to rotate around the z axis
    static inline const Quat rotationZ(float radians);

} VECTORMATH_ALIGNED_TYPE_POST;

// Multiply a quaternion by a scalar
//
inline const Quat operator * (float scalar, const Quat & quat);

// Compute the conjugate of a quaternion
//
inline const Quat conj(const Quat & quat);

// Use a unit-length quaternion to rotate a 3-D vector
//
inline const Vector3 rotate(const Quat & unitQuat, const Vector3 & vec);

// Compute the dot product of two quaternions
//
inline float dot(const Quat & quat0, const Quat & quat1);

// Compute the norm of a quaternion
//
inline float norm(const Quat & quat);

// Compute the length of a quaternion
//
inline float length(const Quat & quat);

// Normalize a quaternion
// NOTE:
// The result is unpredictable when all elements of quat are at or near zero.
//
inline const Quat normalize(const Quat & quat);

// Linear interpolation between two quaternions
// NOTE:
// Does not clamp t between 0 and 1.
//
inline const Quat lerp(float t, const Quat & quat0, const Quat & quat1);

// Spherical linear interpolation between two quaternions
// NOTE:
// Interpolates along the shortest path between orientations.
// Does not clamp t between 0 and 1.
//
inline const Quat slerp(float t, const Quat & unitQuat0, const Quat & unitQuat1);

// Spherical quadrangle interpolation
//
inline const Quat squad(float t, const Quat & unitQuat0, const Quat & unitQuat1, const Quat & unitQuat2, const Quat & unitQuat3);

// Conditionally select between two quaternions
//
inline const Quat select(const Quat & quat0, const Quat & quat1, bool select1);

#ifdef VECTORMATH_DEBUG

// Print a quaternion
// NOTE:
// Function is only defined when VECTORMATH_DEBUG is defined.
//
inline void print(const Quat & quat);

// Print a quaternion and an associated string identifier
// NOTE:
// Function is only defined when VECTORMATH_DEBUG is defined.
//
inline void print(const Quat & quat, const char * name);

#endif // VECTORMATH_DEBUG

// ========================================================
// A 3x3 matrix in array-of-structures format
// ========================================================

VECTORMATH_ALIGNED_TYPE_PRE class Matrix3
{
    Vector3 mCol0;
    Vector3 mCol1;
    Vector3 mCol2;

public:

    // Default constructor; does no initialization
    inline Matrix3() { }

    // Copy a 3x3 matrix
    inline Matrix3(const Matrix3 & mat);

    // Construct a 3x3 matrix containing the specified columns
    inline Matrix3(const Vector3 & col0, const Vector3 & col1, const Vector3 & col2);

    // Construct a 3x3 rotation matrix from a unit-length quaternion
    explicit inline Matrix3(const Quat & unitQuat);

    // Set all elements of a 3x3 matrix to the same scalar value
    explicit inline Matrix3(float scalar);

    // Assign one 3x3 matrix to another
    inline Matrix3 & operator = (const Matrix3 & mat);

    // Set column 0 of a 3x3 matrix
    inline Matrix3 & setCol0(const Vector3 & col0);

    // Set column 1 of a 3x3 matrix
    inline Matrix3 & setCol1(const Vector3 & col1);

    // Set column 2 of a 3x3 matrix
    inline Matrix3 & setCol2(const Vector3 & col2);

    // Get column 0 of a 3x3 matrix
    inline const Vector3 getCol0() const;

    // Get column 1 of a 3x3 matrix
    inline const Vector3 getCol1() const;

    // Get column 2 of a 3x3 matrix
    inline const Vector3 getCol2() const;

    // Set the column of a 3x3 matrix referred to by the specified index
    inline Matrix3 & setCol(int col, const Vector3 & vec);

    // Set the row of a 3x3 matrix referred to by the specified index
    inline Matrix3 & setRow(int row, const Vector3 & vec);

    // Get the column of a 3x3 matrix referred to by the specified index
    inline const Vector3 getCol(int col) const;

    // Get the row of a 3x3 matrix referred to by the specified index
    inline const Vector3 getRow(int row) const;

    // Subscripting operator to set or get a column
    inline Vector3 & operator[](int col);

    // Subscripting operator to get a column
    inline const Vector3 operator[](int col) const;

    // Set the element of a 3x3 matrix referred to by column and row indices
    inline Matrix3 & setElem(int col, int row, float val);

    // Get the element of a 3x3 matrix referred to by column and row indices
    inline float getElem(int col, int row) const;

    // Add two 3x3 matrices
    inline const Matrix3 operator + (const Matrix3 & mat) const;

    // Subtract a 3x3 matrix from another 3x3 matrix
    inline const Matrix3 operator - (const Matrix3 & mat) const;

    // Negate all elements of a 3x3 matrix
    inline const Matrix3 operator - () const;

    // Multiply a 3x3 matrix by a scalar
    inline const Matrix3 operator * (float scalar) const;

    // Multiply a 3x3 matrix by a 3-D vector
    inline const Vector3 operator * (const Vector3 & vec) const;

    // Multiply two 3x3 matrices
    inline const Matrix3 operator * (const Matrix3 & mat) const;

    // Perform compound assignment and addition with a 3x3 matrix
    inline Matrix3 & operator += (const Matrix3 & mat);

    // Perform compound assignment and subtraction by a 3x3 matrix
    inline Matrix3 & operator -= (const Matrix3 & mat);

    // Perform compound assignment and multiplication by a scalar
    inline Matrix3 & operator *= (float scalar);

    // Perform compound assignment and multiplication by a 3x3 matrix
    inline Matrix3 & operator *= (const Matrix3 & mat);

    // Construct an identity 3x3 matrix
    static inline const Matrix3 identity();

    // Construct a 3x3 matrix to rotate around the x axis
    static inline const Matrix3 rotationX(float radians);

    // Construct a 3x3 matrix to rotate around the y axis
    static inline const Matrix3 rotationY(float radians);

    // Construct a 3x3 matrix to rotate around the z axis
    static inline const Matrix3 rotationZ(float radians);

    // Construct a 3x3 matrix to rotate around the x, y, and z axes
    static inline const Matrix3 rotationZYX(const Vector3 & radiansXYZ);

    // Construct a 3x3 matrix to rotate around a unit-length 3-D vector
    static inline const Matrix3 rotation(float radians, const Vector3 & unitVec);

    // Construct a rotation matrix from a unit-length quaternion
    static inline const Matrix3 rotation(const Quat & unitQuat);

    // Construct a 3x3 matrix to perform scaling
    static inline const Matrix3 scale(const Vector3 & scaleVec);

} VECTORMATH_ALIGNED_TYPE_POST;

// Multiply a 3x3 matrix by a scalar
//
inline const Matrix3 operator * (float scalar, const Matrix3 & mat);

// Append (post-multiply) a scale transformation to a 3x3 matrix
// NOTE:
// Faster than creating and multiplying a scale transformation matrix.
//
inline const Matrix3 appendScale(const Matrix3 & mat, const Vector3 & scaleVec);

// Prepend (pre-multiply) a scale transformation to a 3x3 matrix
// NOTE:
// Faster than creating and multiplying a scale transformation matrix.
//
inline const Matrix3 prependScale(const Vector3 & scaleVec, const Matrix3 & mat);

// Multiply two 3x3 matrices per element
//
inline const Matrix3 mulPerElem(const Matrix3 & mat0, const Matrix3 & mat1);

// Compute the absolute value of a 3x3 matrix per element
//
inline const Matrix3 absPerElem(const Matrix3 & mat);

// Transpose of a 3x3 matrix
//
inline const Matrix3 transpose(const Matrix3 & mat);

// Compute the inverse of a 3x3 matrix
// NOTE:
// Result is unpredictable when the determinant of mat is equal to or near 0.
//
inline const Matrix3 inverse(const Matrix3 & mat);

// Determinant of a 3x3 matrix
//
inline float determinant(const Matrix3 & mat);

// Conditionally select between two 3x3 matrices
//
inline const Matrix3 select(const Matrix3 & mat0, const Matrix3 & mat1, bool select1);

#ifdef VECTORMATH_DEBUG

// Print a 3x3 matrix
// NOTE:
// Function is only defined when VECTORMATH_DEBUG is defined.
//
inline void print(const Matrix3 & mat);

// Print a 3x3 matrix and an associated string identifier
// NOTE:
// Function is only defined when VECTORMATH_DEBUG is defined.
//
inline void print(const Matrix3 & mat, const char * name);

#endif // VECTORMATH_DEBUG

// ========================================================
// A 4x4 matrix in array-of-structures format
// ========================================================

VECTORMATH_ALIGNED_TYPE_PRE class Matrix4
{
    Vector4 mCol0;
    Vector4 mCol1;
    Vector4 mCol2;
    Vector4 mCol3;

public:

    // Default constructor; does no initialization
    inline Matrix4() { }

    // Copy a 4x4 matrix
    inline Matrix4(const Matrix4 & mat);

    // Construct a 4x4 matrix containing the specified columns
    inline Matrix4(const Vector4 & col0, const Vector4 & col1, const Vector4 & col2, const Vector4 & col3);

    // Construct a 4x4 matrix from a 3x4 transformation matrix
    explicit inline Matrix4(const Transform3 & mat);

    // Construct a 4x4 matrix from a 3x3 matrix and a 3-D vector
    inline Matrix4(const Matrix3 & mat, const Vector3 & translateVec);

    // Construct a 4x4 matrix from a unit-length quaternion and a 3-D vector
    inline Matrix4(const Quat & unitQuat, const Vector3 & translateVec);

    // Set all elements of a 4x4 matrix to the same scalar value
    explicit inline Matrix4(float scalar);

    // Assign one 4x4 matrix to another
    inline Matrix4 & operator = (const Matrix4 & mat);

    // Set the upper-left 3x3 submatrix
    // NOTE:
    // This function does not change the bottom row elements.
    inline Matrix4 & setUpper3x3(const Matrix3 & mat3);

    // Get the upper-left 3x3 submatrix of a 4x4 matrix
    inline const Matrix3 getUpper3x3() const;

    // Set translation component
    // NOTE:
    // This function does not change the bottom row elements.
    inline Matrix4 & setTranslation(const Vector3 & translateVec);

    // Get the translation component of a 4x4 matrix
    inline const Vector3 getTranslation() const;

    // Set column 0 of a 4x4 matrix
    inline Matrix4 & setCol0(const Vector4 & col0);

    // Set column 1 of a 4x4 matrix
    inline Matrix4 & setCol1(const Vector4 & col1);

    // Set column 2 of a 4x4 matrix
    inline Matrix4 & setCol2(const Vector4 & col2);

    // Set column 3 of a 4x4 matrix
    inline Matrix4 & setCol3(const Vector4 & col3);

    // Get column 0 of a 4x4 matrix
    inline const Vector4 getCol0() const;

    // Get column 1 of a 4x4 matrix
    inline const Vector4 getCol1() const;

    // Get column 2 of a 4x4 matrix
    inline const Vector4 getCol2() const;

    // Get column 3 of a 4x4 matrix
    inline const Vector4 getCol3() const;

    // Set the column of a 4x4 matrix referred to by the specified index
    inline Matrix4 & setCol(int col, const Vector4 & vec);

    // Set the row of a 4x4 matrix referred to by the specified index
    inline Matrix4 & setRow(int row, const Vector4 & vec);

    // Get the column of a 4x4 matrix referred to by the specified index
    inline const Vector4 getCol(int col) const;

    // Get the row of a 4x4 matrix referred to by the specified index
    inline const Vector4 getRow(int row) const;

    // Subscripting operator to set or get a column
    inline Vector4 & operator[](int col);

    // Subscripting operator to get a column
    inline const Vector4 operator[](int col) const;

    // Set the element of a 4x4 matrix referred to by column and row indices
    inline Matrix4 & setElem(int col, int row, float val);

    // Get the element of a 4x4 matrix referred to by column and row indices
    inline float getElem(int col, int row) const;

    // Add two 4x4 matrices
    inline const Matrix4 operator + (const Matrix4 & mat) const;

    // Subtract a 4x4 matrix from another 4x4 matrix
    inline const Matrix4 operator - (const Matrix4 & mat) const;

    // Negate all elements of a 4x4 matrix
    inline const Matrix4 operator - () const;

    // Multiply a 4x4 matrix by a scalar
    inline const Matrix4 operator * (float scalar) const;

    // Multiply a 4x4 matrix by a 4-D vector
    inline const Vector4 operator * (const Vector4 & vec) const;

    // Multiply a 4x4 matrix by a 3-D vector
    inline const Vector4 operator * (const Vector3 & vec) const;

    // Multiply a 4x4 matrix by a 3-D point
    inline const Vector4 operator * (const Point3 & pnt) const;

    // Multiply two 4x4 matrices
    inline const Matrix4 operator * (const Matrix4 & mat) const;

    // Multiply a 4x4 matrix by a 3x4 transformation matrix
    inline const Matrix4 operator * (const Transform3 & tfrm) const;

    // Perform compound assignment and addition with a 4x4 matrix
    inline Matrix4 & operator += (const Matrix4 & mat);

    // Perform compound assignment and subtraction by a 4x4 matrix
    inline Matrix4 & operator -= (const Matrix4 & mat);

    // Perform compound assignment and multiplication by a scalar
    inline Matrix4 & operator *= (float scalar);

    // Perform compound assignment and multiplication by a 4x4 matrix
    inline Matrix4 & operator *= (const Matrix4 & mat);

    // Perform compound assignment and multiplication by a 3x4 transformation matrix
    inline Matrix4 & operator *= (const Transform3 & tfrm);

    // Construct an identity 4x4 matrix
    static inline const Matrix4 identity();

    // Construct a 4x4 matrix to rotate around the x axis
    static inline const Matrix4 rotationX(float radians);

    // Construct a 4x4 matrix to rotate around the y axis
    static inline const Matrix4 rotationY(float radians);

    // Construct a 4x4 matrix to rotate around the z axis
    static inline const Matrix4 rotationZ(float radians);

    // Construct a 4x4 matrix to rotate around the x, y, and z axes
    static inline const Matrix4 rotationZYX(const Vector3 & radiansXYZ);

    // Construct a 4x4 matrix to rotate around a unit-length 3-D vector
    static inline const Matrix4 rotation(float radians, const Vector3 & unitVec);

    // Construct a rotation matrix from a unit-length quaternion
    static inline const Matrix4 rotation(const Quat & unitQuat);

    // Construct a 4x4 matrix to perform scaling
    static inline const Matrix4 scale(const Vector3 & scaleVec);

    // Construct a 4x4 matrix to perform translation
    static inline const Matrix4 translation(const Vector3 & translateVec);

    // Construct viewing matrix based on eye position, position looked at, and up direction
    static inline const Matrix4 lookAt(const Point3 & eyePos, const Point3 & lookAtPos, const Vector3 & upVec);

    // Construct a perspective projection matrix
    static inline const Matrix4 perspective(float fovyRadians, float aspect, float zNear, float zFar);

    // Construct a perspective projection matrix based on frustum
    static inline const Matrix4 frustum(float left, float right, float bottom, float top, float zNear, float zFar);

    // Construct an orthographic projection matrix
    static inline const Matrix4 orthographic(float left, float right, float bottom, float top, float zNear, float zFar);

} VECTORMATH_ALIGNED_TYPE_POST;

// Multiply a 4x4 matrix by a scalar
//
inline const Matrix4 operator * (float scalar, const Matrix4 & mat);

// Append (post-multiply) a scale transformation to a 4x4 matrix
// NOTE:
// Faster than creating and multiplying a scale transformation matrix.
//
inline const Matrix4 appendScale(const Matrix4 & mat, const Vector3 & scaleVec);

// Prepend (pre-multiply) a scale transformation to a 4x4 matrix
// NOTE:
// Faster than creating and multiplying a scale transformation matrix.
//
inline const Matrix4 prependScale(const Vector3 & scaleVec, const Matrix4 & mat);

// Multiply two 4x4 matrices per element
//
inline const Matrix4 mulPerElem(const Matrix4 & mat0, const Matrix4 & mat1);

// Compute the absolute value of a 4x4 matrix per element
//
inline const Matrix4 absPerElem(const Matrix4 & mat);

// Transpose of a 4x4 matrix
//
inline const Matrix4 transpose(const Matrix4 & mat);

// Compute the inverse of a 4x4 matrix
// NOTE:
// Result is unpredictable when the determinant of mat is equal to or near 0.
//
inline const Matrix4 inverse(const Matrix4 & mat);

// Compute the inverse of a 4x4 matrix, which is expected to be an affine matrix
// NOTE:
// This can be used to achieve better performance than a general inverse when the specified 4x4 matrix meets the given restrictions.  The result is unpredictable when the determinant of mat is equal to or near 0.
//
inline const Matrix4 affineInverse(const Matrix4 & mat);

// Compute the inverse of a 4x4 matrix, which is expected to be an affine matrix with an orthogonal upper-left 3x3 submatrix
// NOTE:
// This can be used to achieve better performance than a general inverse when the specified 4x4 matrix meets the given restrictions.
//
inline const Matrix4 orthoInverse(const Matrix4 & mat);

// Determinant of a 4x4 matrix
//
inline float determinant(const Matrix4 & mat);

// Conditionally select between two 4x4 matrices
//
inline const Matrix4 select(const Matrix4 & mat0, const Matrix4 & mat1, bool select1);

#ifdef VECTORMATH_DEBUG

// Print a 4x4 matrix
// NOTE:
// Function is only defined when VECTORMATH_DEBUG is defined.
//
inline void print(const Matrix4 & mat);

// Print a 4x4 matrix and an associated string identifier
// NOTE:
// Function is only defined when VECTORMATH_DEBUG is defined.
//
inline void print(const Matrix4 & mat, const char * name);

#endif // VECTORMATH_DEBUG

// ========================================================
// A 3x4 transformation matrix in array-of-structs format
// ========================================================

VECTORMATH_ALIGNED_TYPE_PRE class Transform3
{
    Vector3 mCol0;
    Vector3 mCol1;
    Vector3 mCol2;
    Vector3 mCol3;

public:

    // Default constructor; does no initialization
    inline Transform3() { }

    // Copy a 3x4 transformation matrix
    inline Transform3(const Transform3 & tfrm);

    // Construct a 3x4 transformation matrix containing the specified columns
    inline Transform3(const Vector3 & col0, const Vector3 & col1, const Vector3 & col2, const Vector3 & col3);

    // Construct a 3x4 transformation matrix from a 3x3 matrix and a 3-D vector
    inline Transform3(const Matrix3 & tfrm, const Vector3 & translateVec);

    // Construct a 3x4 transformation matrix from a unit-length quaternion and a 3-D vector
    inline Transform3(const Quat & unitQuat, const Vector3 & translateVec);

    // Set all elements of a 3x4 transformation matrix to the same scalar value
    explicit inline Transform3(float scalar);

    // Assign one 3x4 transformation matrix to another
    inline Transform3 & operator = (const Transform3 & tfrm);

    // Set the upper-left 3x3 submatrix
    inline Transform3 & setUpper3x3(const Matrix3 & mat3);

    // Get the upper-left 3x3 submatrix of a 3x4 transformation matrix
    inline const Matrix3 getUpper3x3() const;

    // Set translation component
    inline Transform3 & setTranslation(const Vector3 & translateVec);

    // Get the translation component of a 3x4 transformation matrix
    inline const Vector3 getTranslation() const;

    // Set column 0 of a 3x4 transformation matrix
    inline Transform3 & setCol0(const Vector3 & col0);

    // Set column 1 of a 3x4 transformation matrix
    inline Transform3 & setCol1(const Vector3 & col1);

    // Set column 2 of a 3x4 transformation matrix
    inline Transform3 & setCol2(const Vector3 & col2);

    // Set column 3 of a 3x4 transformation matrix
    inline Transform3 & setCol3(const Vector3 & col3);

    // Get column 0 of a 3x4 transformation matrix
    inline const Vector3 getCol0() const;

    // Get column 1 of a 3x4 transformation matrix
    inline const Vector3 getCol1() const;

    // Get column 2 of a 3x4 transformation matrix
    inline const Vector3 getCol2() const;

    // Get column 3 of a 3x4 transformation matrix
    inline const Vector3 getCol3() const;

    // Set the column of a 3x4 transformation matrix referred to by the specified index
    inline Transform3 & setCol(int col, const Vector3 & vec);

    // Set the row of a 3x4 transformation matrix referred to by the specified index
    inline Transform3 & setRow(int row, const Vector4 & vec);

    // Get the column of a 3x4 transformation matrix referred to by the specified index
    inline const Vector3 getCol(int col) const;

    // Get the row of a 3x4 transformation matrix referred to by the specified index
    inline const Vector4 getRow(int row) const;

    // Subscripting operator to set or get a column
    inline Vector3 & operator[](int col);

    // Subscripting operator to get a column
    inline const Vector3 operator[](int col) const;

    // Set the element of a 3x4 transformation matrix referred to by column and row indices
    inline Transform3 & setElem(int col, int row, float val);

    // Get the element of a 3x4 transformation matrix referred to by column and row indices
    inline float getElem(int col, int row) const;

    // Multiply a 3x4 transformation matrix by a 3-D vector
    inline const Vector3 operator * (const Vector3 & vec) const;

    // Multiply a 3x4 transformation matrix by a 3-D point
    inline const Point3 operator * (const Point3 & pnt) const;

    // Multiply two 3x4 transformation matrices
    inline const Transform3 operator * (const Transform3 & tfrm) const;

    // Perform compound assignment and multiplication by a 3x4 transformation matrix
    inline Transform3 & operator *= (const Transform3 & tfrm);

    // Construct an identity 3x4 transformation matrix
    static inline const Transform3 identity();

    // Construct a 3x4 transformation matrix to rotate around the x axis
    static inline const Transform3 rotationX(float radians);

    // Construct a 3x4 transformation matrix to rotate around the y axis
    static inline const Transform3 rotationY(float radians);

    // Construct a 3x4 transformation matrix to rotate around the z axis
    static inline const Transform3 rotationZ(float radians);

    // Construct a 3x4 transformation matrix to rotate around the x, y, and z axes
    static inline const Transform3 rotationZYX(const Vector3 & radiansXYZ);

    // Construct a 3x4 transformation matrix to rotate around a unit-length 3-D vector
    static inline const Transform3 rotation(float radians, const Vector3 & unitVec);

    // Construct a rotation matrix from a unit-length quaternion
    static inline const Transform3 rotation(const Quat & unitQuat);

    // Construct a 3x4 transformation matrix to perform scaling
    static inline const Transform3 scale(const Vector3 & scaleVec);

    // Construct a 3x4 transformation matrix to perform translation
    static inline const Transform3 translation(const Vector3 & translateVec);

} VECTORMATH_ALIGNED_TYPE_POST;

// Append (post-multiply) a scale transformation to a 3x4 transformation matrix
// NOTE:
// Faster than creating and multiplying a scale transformation matrix.
//
inline const Transform3 appendScale(const Transform3 & tfrm, const Vector3 & scaleVec);

// Prepend (pre-multiply) a scale transformation to a 3x4 transformation matrix
// NOTE:
// Faster than creating and multiplying a scale transformation matrix.
//
inline const Transform3 prependScale(const Vector3 & scaleVec, const Transform3 & tfrm);

// Multiply two 3x4 transformation matrices per element
//
inline const Transform3 mulPerElem(const Transform3 & tfrm0, const Transform3 & tfrm1);

// Compute the absolute value of a 3x4 transformation matrix per element
//
inline const Transform3 absPerElem(const Transform3 & tfrm);

// Inverse of a 3x4 transformation matrix
// NOTE:
// Result is unpredictable when the determinant of the left 3x3 submatrix is equal to or near 0.
//
inline const Transform3 inverse(const Transform3 & tfrm);

// Compute the inverse of a 3x4 transformation matrix, expected to have an orthogonal upper-left 3x3 submatrix
// NOTE:
// This can be used to achieve better performance than a general inverse when the specified 3x4 transformation matrix meets the given restrictions.
//
inline const Transform3 orthoInverse(const Transform3 & tfrm);

// Conditionally select between two 3x4 transformation matrices
//
inline const Transform3 select(const Transform3 & tfrm0, const Transform3 & tfrm1, bool select1);

#ifdef VECTORMATH_DEBUG

// Print a 3x4 transformation matrix
// NOTE:
// Function is only defined when VECTORMATH_DEBUG is defined.
//
inline void print(const Transform3 & tfrm);

// Print a 3x4 transformation matrix and an associated string identifier
// NOTE:
// Function is only defined when VECTORMATH_DEBUG is defined.
//
inline void print(const Transform3 & tfrm, const char * name);

#endif // VECTORMATH_DEBUG

} // namespace Scalar
} // namespace Vectormath

// Inline implementations:

#ifndef VECTORMATH_SCALAR_VECTOR_HPP
#define VECTORMATH_SCALAR_VECTOR_HPP

namespace Vectormath
{
namespace Scalar
{

// Small epsilon value
static const float VECTORMATH_SLERP_TOL = 0.999f;

// ========================================================
// Vector3
// ========================================================

inline Vector3::Vector3(const Vector3 & vec)
{
    mX = vec.mX;
    mY = vec.mY;
    mZ = vec.mZ;
}

inline Vector3::Vector3(float _x, float _y, float _z)
{
    mX = _x;
    mY = _y;
    mZ = _z;
}

inline Vector3::Vector3(const Point3 & pnt)
{
    mX = pnt.getX();
    mY = pnt.getY();
    mZ = pnt.getZ();
}

inline Vector3::Vector3(float scalar)
{
    mX = scalar;
    mY = scalar;
    mZ = scalar;
}

inline const Vector3 Vector3::xAxis()
{
    return Vector3(1.0f, 0.0f, 0.0f);
}

inline const Vector3 Vector3::yAxis()
{
    return Vector3(0.0f, 1.0f, 0.0f);
}

inline const Vector3 Vector3::zAxis()
{
    return Vector3(0.0f, 0.0f, 1.0f);
}

inline const Vector3 lerp(float t, const Vector3 & vec0, const Vector3 & vec1)
{
    return (vec0 + ((vec1 - vec0) * t));
}

inline const Vector3 slerp(float t, const Vector3 & unitVec0, const Vector3 & unitVec1)
{
    float recipSinAngle, scale0, scale1, cosAngle, angle;
    cosAngle = dot(unitVec0, unitVec1);
    if (cosAngle < VECTORMATH_SLERP_TOL)
    {
        angle = acosf(cosAngle);
        recipSinAngle = (1.0f / sinf(angle));
        scale0 = (sinf(((1.0f - t) * angle)) * recipSinAngle);
        scale1 = (sinf((t * angle)) * recipSinAngle);
    }
    else
    {
        scale0 = (1.0f - t);
        scale1 = t;
    }
    return ((unitVec0 * scale0) + (unitVec1 * scale1));
}

inline Vector3 & Vector3::operator = (const Vector3 & vec)
{
    mX = vec.mX;
    mY = vec.mY;
    mZ = vec.mZ;
    return *this;
}

inline Vector3 & Vector3::setX(float _x)
{
    mX = _x;
    return *this;
}

inline float Vector3::getX() const
{
    return mX;
}

inline Vector3 & Vector3::setY(float _y)
{
    mY = _y;
    return *this;
}

inline float Vector3::getY() const
{
    return mY;
}

inline Vector3 & Vector3::setZ(float _z)
{
    mZ = _z;
    return *this;
}

inline float Vector3::getZ() const
{
    return mZ;
}

inline Vector3 & Vector3::setW(float _w)
{
    mW = _w;
    return *this;
}

inline float Vector3::getW() const
{
    return mW;
}

inline Vector3 & Vector3::setElem(int idx, float value)
{
    *(&mX + idx) = value;
    return *this;
}

inline float Vector3::getElem(int idx) const
{
    return *(&mX + idx);
}

inline float & Vector3::operator[](int idx)
{
    return *(&mX + idx);
}

inline float Vector3::operator[](int idx) const
{
    return *(&mX + idx);
}

inline const Vector3 Vector3::operator + (const Vector3 & vec) const
{
    return Vector3((mX + vec.mX),
                   (mY + vec.mY),
                   (mZ + vec.mZ));
}

inline const Vector3 Vector3::operator - (const Vector3 & vec) const
{
    return Vector3((mX - vec.mX),
                   (mY - vec.mY),
                   (mZ - vec.mZ));
}

inline const Point3 Vector3::operator + (const Point3 & pnt) const
{
    return Point3((mX + pnt.getX()),
                  (mY + pnt.getY()),
                  (mZ + pnt.getZ()));
}

inline const Vector3 Vector3::operator * (float scalar) const
{
    return Vector3((mX * scalar),
                   (mY * scalar),
                   (mZ * scalar));
}

inline Vector3 & Vector3::operator += (const Vector3 & vec)
{
    *this = *this + vec;
    return *this;
}

inline Vector3 & Vector3::operator -= (const Vector3 & vec)
{
    *this = *this - vec;
    return *this;
}

inline Vector3 & Vector3::operator *= (float scalar)
{
    *this = *this * scalar;
    return *this;
}

inline const Vector3 Vector3::operator / (float scalar) const
{
    return Vector3((mX / scalar),
                   (mY / scalar),
                   (mZ / scalar));
}

inline Vector3 & Vector3::operator /= (float scalar)
{
    *this = *this / scalar;
    return *this;
}

inline const Vector3 Vector3::operator - () const
{
    return Vector3(-mX, -mY, -mZ);
}

inline const Vector3 operator * (float scalar, const Vector3 & vec)
{
    return vec * scalar;
}

inline const Vector3 mulPerElem(const Vector3 & vec0, const Vector3 & vec1)
{
    return Vector3((vec0.getX() * vec1.getX()),
                   (vec0.getY() * vec1.getY()),
                   (vec0.getZ() * vec1.getZ()));
}

inline const Vector3 divPerElem(const Vector3 & vec0, const Vector3 & vec1)
{
    return Vector3((vec0.getX() / vec1.getX()),
                   (vec0.getY() / vec1.getY()),
                   (vec0.getZ() / vec1.getZ()));
}

inline const Vector3 recipPerElem(const Vector3 & vec)
{
    return Vector3((1.0f / vec.getX()),
                   (1.0f / vec.getY()),
                   (1.0f / vec.getZ()));
}

inline const Vector3 sqrtPerElem(const Vector3 & vec)
{
    return Vector3(sqrtf(vec.getX()),
                   sqrtf(vec.getY()),
                   sqrtf(vec.getZ()));
}

inline const Vector3 rsqrtPerElem(const Vector3 & vec)
{
    return Vector3((1.0f / sqrtf(vec.getX())),
                   (1.0f / sqrtf(vec.getY())),
                   (1.0f / sqrtf(vec.getZ())));
}

inline const Vector3 absPerElem(const Vector3 & vec)
{
    return Vector3(fabsf(vec.getX()),
                   fabsf(vec.getY()),
                   fabsf(vec.getZ()));
}

inline const Vector3 copySignPerElem(const Vector3 & vec0, const Vector3 & vec1)
{
    return Vector3((vec1.getX() < 0.0f) ? -fabsf(vec0.getX()) : fabsf(vec0.getX()),
                   (vec1.getY() < 0.0f) ? -fabsf(vec0.getY()) : fabsf(vec0.getY()),
                   (vec1.getZ() < 0.0f) ? -fabsf(vec0.getZ()) : fabsf(vec0.getZ()));
}

inline const Vector3 maxPerElem(const Vector3 & vec0, const Vector3 & vec1)
{
    return Vector3((vec0.getX() > vec1.getX()) ? vec0.getX() : vec1.getX(),
                   (vec0.getY() > vec1.getY()) ? vec0.getY() : vec1.getY(),
                   (vec0.getZ() > vec1.getZ()) ? vec0.getZ() : vec1.getZ());
}

inline float maxElem(const Vector3 & vec)
{
    float result;
    result = (vec.getX() > vec.getY()) ? vec.getX() : vec.getY();
    result = (vec.getZ() > result)     ? vec.getZ() : result;
    return result;
}

inline const Vector3 minPerElem(const Vector3 & vec0, const Vector3 & vec1)
{
    return Vector3((vec0.getX() < vec1.getX()) ? vec0.getX() : vec1.getX(),
                   (vec0.getY() < vec1.getY()) ? vec0.getY() : vec1.getY(),
                   (vec0.getZ() < vec1.getZ()) ? vec0.getZ() : vec1.getZ());
}

inline float minElem(const Vector3 & vec)
{
    float result;
    result = (vec.getX() < vec.getY()) ? vec.getX() : vec.getY();
    result = (vec.getZ() < result)     ? vec.getZ() : result;
    return result;
}

inline float sum(const Vector3 & vec)
{
    float result;
    result = (vec.getX() + vec.getY());
    result = (result + vec.getZ());
    return result;
}

inline float dot(const Vector3 & vec0, const Vector3 & vec1)
{
    float result;
    result = (vec0.getX() * vec1.getX());
    result = (result + (vec0.getY() * vec1.getY()));
    result = (result + (vec0.getZ() * vec1.getZ()));
    return result;
}

inline float lengthSqr(const Vector3 & vec)
{
    float result;
    result = (vec.getX() * vec.getX());
    result = (result + (vec.getY() * vec.getY()));
    result = (result + (vec.getZ() * vec.getZ()));
    return result;
}

inline float length(const Vector3 & vec)
{
    return sqrtf(lengthSqr(vec));
}

inline const Vector3 normalize(const Vector3 & vec)
{
    float lenSqr, lenInv;
    lenSqr = lengthSqr(vec);
    lenInv = (1.0f / sqrtf(lenSqr));
    return Vector3((vec.getX() * lenInv),
                   (vec.getY() * lenInv),
                   (vec.getZ() * lenInv));
}

inline const Vector3 cross(const Vector3 & vec0, const Vector3 & vec1)
{
    return Vector3(((vec0.getY() * vec1.getZ()) - (vec0.getZ() * vec1.getY())),
                   ((vec0.getZ() * vec1.getX()) - (vec0.getX() * vec1.getZ())),
                   ((vec0.getX() * vec1.getY()) - (vec0.getY() * vec1.getX())));
}

inline const Vector3 select(const Vector3 & vec0, const Vector3 & vec1, bool select1)
{
    return Vector3((select1) ? vec1.getX() : vec0.getX(),
                   (select1) ? vec1.getY() : vec0.getY(),
                   (select1) ? vec1.getZ() : vec0.getZ());
}

#ifdef VECTORMATH_DEBUG

inline void print(const Vector3 & vec)
{
    printf("( %f %f %f )\n", vec.getX(), vec.getY(), vec.getZ());
}

inline void print(const Vector3 & vec, const char * name)
{
    printf("%s: ( %f %f %f )\n", name, vec.getX(), vec.getY(), vec.getZ());
}

#endif // VECTORMATH_DEBUG

// ========================================================
// Vector4
// ========================================================

inline Vector4::Vector4(const Vector4 & vec)
{
    mX = vec.mX;
    mY = vec.mY;
    mZ = vec.mZ;
    mW = vec.mW;
}

inline Vector4::Vector4(float _x, float _y, float _z, float _w)
{
    mX = _x;
    mY = _y;
    mZ = _z;
    mW = _w;
}

inline Vector4::Vector4(const Vector3 & xyz, float _w)
{
    this->setXYZ(xyz);
    this->setW(_w);
}

inline Vector4::Vector4(const Vector3 & vec)
{
    mX = vec.getX();
    mY = vec.getY();
    mZ = vec.getZ();
    mW = 0.0f;
}

inline Vector4::Vector4(const Point3 & pnt)
{
    mX = pnt.getX();
    mY = pnt.getY();
    mZ = pnt.getZ();
    mW = 1.0f;
}

inline Vector4::Vector4(const Quat & quat)
{
    mX = quat.getX();
    mY = quat.getY();
    mZ = quat.getZ();
    mW = quat.getW();
}

inline Vector4::Vector4(float scalar)
{
    mX = scalar;
    mY = scalar;
    mZ = scalar;
    mW = scalar;
}

inline const Vector4 Vector4::xAxis()
{
    return Vector4(1.0f, 0.0f, 0.0f, 0.0f);
}

inline const Vector4 Vector4::yAxis()
{
    return Vector4(0.0f, 1.0f, 0.0f, 0.0f);
}

inline const Vector4 Vector4::zAxis()
{
    return Vector4(0.0f, 0.0f, 1.0f, 0.0f);
}

inline const Vector4 Vector4::wAxis()
{
    return Vector4(0.0f, 0.0f, 0.0f, 1.0f);
}

inline const Vector4 lerp(float t, const Vector4 & vec0, const Vector4 & vec1)
{
    return (vec0 + ((vec1 - vec0) * t));
}

inline const Vector4 slerp(float t, const Vector4 & unitVec0, const Vector4 & unitVec1)
{
    float recipSinAngle, scale0, scale1, cosAngle, angle;
    cosAngle = dot(unitVec0, unitVec1);
    if (cosAngle < VECTORMATH_SLERP_TOL)
    {
        angle = acosf(cosAngle);
        recipSinAngle = (1.0f / sinf(angle));
        scale0 = (sinf(((1.0f - t) * angle)) * recipSinAngle);
        scale1 = (sinf((t * angle)) * recipSinAngle);
    }
    else
    {
        scale0 = (1.0f - t);
        scale1 = t;
    }
    return ((unitVec0 * scale0) + (unitVec1 * scale1));
}

inline Vector4 & Vector4::operator = (const Vector4 & vec)
{
    mX = vec.mX;
    mY = vec.mY;
    mZ = vec.mZ;
    mW = vec.mW;
    return *this;
}

inline Vector4 & Vector4::setXYZ(const Vector3 & vec)
{
    mX = vec.getX();
    mY = vec.getY();
    mZ = vec.getZ();
    return *this;
}

inline const Vector3 Vector4::getXYZ() const
{
    return Vector3(mX, mY, mZ);
}

inline Vector4 & Vector4::setX(float _x)
{
    mX = _x;
    return *this;
}

inline float Vector4::getX() const
{
    return mX;
}

inline Vector4 & Vector4::setY(float _y)
{
    mY = _y;
    return *this;
}

inline float Vector4::getY() const
{
    return mY;
}

inline Vector4 & Vector4::setZ(float _z)
{
    mZ = _z;
    return *this;
}

inline float Vector4::getZ() const
{
    return mZ;
}

inline Vector4 & Vector4::setW(float _w)
{
    mW = _w;
    return *this;
}

inline float Vector4::getW() const
{
    return mW;
}

inline Vector4 & Vector4::setElem(int idx, float value)
{
    *(&mX + idx) = value;
    return *this;
}

inline float Vector4::getElem(int idx) const
{
    return *(&mX + idx);
}

inline float & Vector4::operator[](int idx)
{
    return *(&mX + idx);
}

inline float Vector4::operator[](int idx) const
{
    return *(&mX + idx);
}

inline const Vector4 Vector4::operator + (const Vector4 & vec) const
{
    return Vector4((mX + vec.mX),
                   (mY + vec.mY),
                   (mZ + vec.mZ),
                   (mW + vec.mW));
}

inline const Vector4 Vector4::operator - (const Vector4 & vec) const
{
    return Vector4((mX - vec.mX),
                   (mY - vec.mY),
                   (mZ - vec.mZ),
                   (mW - vec.mW));
}

inline const Vector4 Vector4::operator * (float scalar) const
{
    return Vector4((mX * scalar),
                   (mY * scalar),
                   (mZ * scalar),
                   (mW * scalar));
}

inline Vector4 & Vector4::operator += (const Vector4 & vec)
{
    *this = *this + vec;
    return *this;
}

inline Vector4 & Vector4::operator -= (const Vector4 & vec)
{
    *this = *this - vec;
    return *this;
}

inline Vector4 & Vector4::operator *= (float scalar)
{
    *this = *this * scalar;
    return *this;
}

inline const Vector4 Vector4::operator / (float scalar) const
{
    return Vector4((mX / scalar),
                   (mY / scalar),
                   (mZ / scalar),
                   (mW / scalar));
}

inline Vector4 & Vector4::operator /= (float scalar)
{
    *this = *this / scalar;
    return *this;
}

inline const Vector4 Vector4::operator - () const
{
    return Vector4(-mX, -mY, -mZ, -mW);
}

inline const Vector4 operator * (float scalar, const Vector4 & vec)
{
    return vec * scalar;
}

inline const Vector4 mulPerElem(const Vector4 & vec0, const Vector4 & vec1)
{
    return Vector4((vec0.getX() * vec1.getX()),
                   (vec0.getY() * vec1.getY()),
                   (vec0.getZ() * vec1.getZ()),
                   (vec0.getW() * vec1.getW()));
}

inline const Vector4 divPerElem(const Vector4 & vec0, const Vector4 & vec1)
{
    return Vector4((vec0.getX() / vec1.getX()),
                   (vec0.getY() / vec1.getY()),
                   (vec0.getZ() / vec1.getZ()),
                   (vec0.getW() / vec1.getW()));
}

inline const Vector4 recipPerElem(const Vector4 & vec)
{
    return Vector4((1.0f / vec.getX()),
                   (1.0f / vec.getY()),
                   (1.0f / vec.getZ()),
                   (1.0f / vec.getW()));
}

inline const Vector4 sqrtPerElem(const Vector4 & vec)
{
    return Vector4(sqrtf(vec.getX()),
                   sqrtf(vec.getY()),
                   sqrtf(vec.getZ()),
                   sqrtf(vec.getW()));
}

inline const Vector4 rsqrtPerElem(const Vector4 & vec)
{
    return Vector4((1.0f / sqrtf(vec.getX())),
                   (1.0f / sqrtf(vec.getY())),
                   (1.0f / sqrtf(vec.getZ())),
                   (1.0f / sqrtf(vec.getW())));
}

inline const Vector4 absPerElem(const Vector4 & vec)
{
    return Vector4(fabsf(vec.getX()),
                   fabsf(vec.getY()),
                   fabsf(vec.getZ()),
                   fabsf(vec.getW()));
}

inline const Vector4 copySignPerElem(const Vector4 & vec0, const Vector4 & vec1)
{
    return Vector4((vec1.getX() < 0.0f) ? -fabsf(vec0.getX()) : fabsf(vec0.getX()),
                   (vec1.getY() < 0.0f) ? -fabsf(vec0.getY()) : fabsf(vec0.getY()),
                   (vec1.getZ() < 0.0f) ? -fabsf(vec0.getZ()) : fabsf(vec0.getZ()),
                   (vec1.getW() < 0.0f) ? -fabsf(vec0.getW()) : fabsf(vec0.getW()));
}

inline const Vector4 maxPerElem(const Vector4 & vec0, const Vector4 & vec1)
{
    return Vector4((vec0.getX() > vec1.getX()) ? vec0.getX() : vec1.getX(),
                   (vec0.getY() > vec1.getY()) ? vec0.getY() : vec1.getY(),
                   (vec0.getZ() > vec1.getZ()) ? vec0.getZ() : vec1.getZ(),
                   (vec0.getW() > vec1.getW()) ? vec0.getW() : vec1.getW());
}

inline float maxElem(const Vector4 & vec)
{
    float result;
    result = (vec.getX() > vec.getY()) ? vec.getX() : vec.getY();
    result = (vec.getZ() > result)     ? vec.getZ() : result;
    result = (vec.getW() > result)     ? vec.getW() : result;
    return result;
}

inline const Vector4 minPerElem(const Vector4 & vec0, const Vector4 & vec1)
{
    return Vector4((vec0.getX() < vec1.getX()) ? vec0.getX() : vec1.getX(),
                   (vec0.getY() < vec1.getY()) ? vec0.getY() : vec1.getY(),
                   (vec0.getZ() < vec1.getZ()) ? vec0.getZ() : vec1.getZ(),
                   (vec0.getW() < vec1.getW()) ? vec0.getW() : vec1.getW());
}

inline float minElem(const Vector4 & vec)
{
    float result;
    result = (vec.getX() < vec.getY()) ? vec.getX() : vec.getY();
    result = (vec.getZ() < result)     ? vec.getZ() : result;
    result = (vec.getW() < result)     ? vec.getW() : result;
    return result;
}

inline float sum(const Vector4 & vec)
{
    float result;
    result = (vec.getX() + vec.getY());
    result = (result + vec.getZ());
    result = (result + vec.getW());
    return result;
}

inline float dot(const Vector4 & vec0, const Vector4 & vec1)
{
    float result;
    result = (vec0.getX() * vec1.getX());
    result = (result + (vec0.getY() * vec1.getY()));
    result = (result + (vec0.getZ() * vec1.getZ()));
    result = (result + (vec0.getW() * vec1.getW()));
    return result;
}

inline float lengthSqr(const Vector4 & vec)
{
    float result;
    result = (vec.getX() * vec.getX());
    result = (result + (vec.getY() * vec.getY()));
    result = (result + (vec.getZ() * vec.getZ()));
    result = (result + (vec.getW() * vec.getW()));
    return result;
}

inline float length(const Vector4 & vec)
{
    return sqrtf(lengthSqr(vec));
}

inline const Vector4 normalize(const Vector4 & vec)
{
    float lenSqr, lenInv;
    lenSqr = lengthSqr(vec);
    lenInv = (1.0f / sqrtf(lenSqr));
    return Vector4((vec.getX() * lenInv),
                   (vec.getY() * lenInv),
                   (vec.getZ() * lenInv),
                   (vec.getW() * lenInv));
}

inline const Vector4 select(const Vector4 & vec0, const Vector4 & vec1, bool select1)
{
    return Vector4((select1) ? vec1.getX() : vec0.getX(),
                   (select1) ? vec1.getY() : vec0.getY(),
                   (select1) ? vec1.getZ() : vec0.getZ(),
                   (select1) ? vec1.getW() : vec0.getW());
}

#ifdef VECTORMATH_DEBUG

inline void print(const Vector4 & vec)
{
    printf("( %f %f %f %f )\n", vec.getX(), vec.getY(), vec.getZ(), vec.getW());
}

inline void print(const Vector4 & vec, const char * name)
{
    printf("%s: ( %f %f %f %f )\n", name, vec.getX(), vec.getY(), vec.getZ(), vec.getW());
}

#endif // VECTORMATH_DEBUG

// ========================================================
// Point3
// ========================================================

inline Point3::Point3(const Point3 & pnt)
{
    mX = pnt.mX;
    mY = pnt.mY;
    mZ = pnt.mZ;
}

inline Point3::Point3(float _x, float _y, float _z)
{
    mX = _x;
    mY = _y;
    mZ = _z;
}

inline Point3::Point3(const Vector3 & vec)
{
    mX = vec.getX();
    mY = vec.getY();
    mZ = vec.getZ();
}

inline Point3::Point3(float scalar)
{
    mX = scalar;
    mY = scalar;
    mZ = scalar;
}

inline const Point3 lerp(float t, const Point3 & pnt0, const Point3 & pnt1)
{
    return (pnt0 + ((pnt1 - pnt0) * t));
}

inline Point3 & Point3::operator = (const Point3 & pnt)
{
    mX = pnt.mX;
    mY = pnt.mY;
    mZ = pnt.mZ;
    return *this;
}

inline Point3 & Point3::setX(float _x)
{
    mX = _x;
    return *this;
}

inline float Point3::getX() const
{
    return mX;
}

inline Point3 & Point3::setY(float _y)
{
    mY = _y;
    return *this;
}

inline float Point3::getY() const
{
    return mY;
}

inline Point3 & Point3::setZ(float _z)
{
    mZ = _z;
    return *this;
}

inline float Point3::getZ() const
{
    return mZ;
}

inline Point3 & Point3::setW(float _w)
{
    mW = _w;
    return *this;
}

inline float Point3::getW() const
{
    return mW;
}

inline Point3 & Point3::setElem(int idx, float value)
{
    *(&mX + idx) = value;
    return *this;
}

inline float Point3::getElem(int idx) const
{
    return *(&mX + idx);
}

inline float & Point3::operator[](int idx)
{
    return *(&mX + idx);
}

inline float Point3::operator[](int idx) const
{
    return *(&mX + idx);
}

inline const Vector3 Point3::operator - (const Point3 & pnt) const
{
    return Vector3((mX - pnt.mX), (mY - pnt.mY), (mZ - pnt.mZ));
}

inline const Point3 Point3::operator + (const Vector3 & vec) const
{
    return Point3((mX + vec.getX()), (mY + vec.getY()), (mZ + vec.getZ()));
}

inline const Point3 Point3::operator - (const Vector3 & vec) const
{
    return Point3((mX - vec.getX()), (mY - vec.getY()), (mZ - vec.getZ()));
}

inline Point3 & Point3::operator += (const Vector3 & vec)
{
    *this = *this + vec;
    return *this;
}

inline Point3 & Point3::operator -= (const Vector3 & vec)
{
    *this = *this - vec;
    return *this;
}

inline const Point3 mulPerElem(const Point3 & pnt0, const Point3 & pnt1)
{
    return Point3((pnt0.getX() * pnt1.getX()),
                  (pnt0.getY() * pnt1.getY()),
                  (pnt0.getZ() * pnt1.getZ()));
}

inline const Point3 divPerElem(const Point3 & pnt0, const Point3 & pnt1)
{
    return Point3((pnt0.getX() / pnt1.getX()),
                  (pnt0.getY() / pnt1.getY()),
                  (pnt0.getZ() / pnt1.getZ()));
}

inline const Point3 recipPerElem(const Point3 & pnt)
{
    return Point3((1.0f / pnt.getX()),
                  (1.0f / pnt.getY()),
                  (1.0f / pnt.getZ()));
}

inline const Point3 sqrtPerElem(const Point3 & pnt)
{
    return Point3(sqrtf(pnt.getX()),
                  sqrtf(pnt.getY()),
                  sqrtf(pnt.getZ()));
}

inline const Point3 rsqrtPerElem(const Point3 & pnt)
{
    return Point3((1.0f / sqrtf(pnt.getX())),
                  (1.0f / sqrtf(pnt.getY())),
                  (1.0f / sqrtf(pnt.getZ())));
}

inline const Point3 absPerElem(const Point3 & pnt)
{
    return Point3(fabsf(pnt.getX()),
                  fabsf(pnt.getY()),
                  fabsf(pnt.getZ()));
}

inline const Point3 copySignPerElem(const Point3 & pnt0, const Point3 & pnt1)
{
    return Point3((pnt1.getX() < 0.0f) ? -fabsf(pnt0.getX()) : fabsf(pnt0.getX()),
                  (pnt1.getY() < 0.0f) ? -fabsf(pnt0.getY()) : fabsf(pnt0.getY()),
                  (pnt1.getZ() < 0.0f) ? -fabsf(pnt0.getZ()) : fabsf(pnt0.getZ()));
}

inline const Point3 maxPerElem(const Point3 & pnt0, const Point3 & pnt1)
{
    return Point3((pnt0.getX() > pnt1.getX()) ? pnt0.getX() : pnt1.getX(),
                  (pnt0.getY() > pnt1.getY()) ? pnt0.getY() : pnt1.getY(),
                  (pnt0.getZ() > pnt1.getZ()) ? pnt0.getZ() : pnt1.getZ());
}

inline float maxElem(const Point3 & pnt)
{
    float result;
    result = (pnt.getX() > pnt.getY()) ? pnt.getX() : pnt.getY();
    result = (pnt.getZ() > result)     ? pnt.getZ() : result;
    return result;
}

inline const Point3 minPerElem(const Point3 & pnt0, const Point3 & pnt1)
{
    return Point3((pnt0.getX() < pnt1.getX()) ? pnt0.getX() : pnt1.getX(),
                  (pnt0.getY() < pnt1.getY()) ? pnt0.getY() : pnt1.getY(),
                  (pnt0.getZ() < pnt1.getZ()) ? pnt0.getZ() : pnt1.getZ());
}

inline float minElem(const Point3 & pnt)
{
    float result;
    result = (pnt.getX() < pnt.getY()) ? pnt.getX() : pnt.getY();
    result = (pnt.getZ() < result)     ? pnt.getZ() : result;
    return result;
}

inline float sum(const Point3 & pnt)
{
    float result;
    result = (pnt.getX() + pnt.getY());
    result = (result + pnt.getZ());
    return result;
}

inline const Point3 scale(const Point3 & pnt, float scaleVal)
{
    return mulPerElem(pnt, Point3(scaleVal));
}

inline const Point3 scale(const Point3 & pnt, const Vector3 & scaleVec)
{
    return mulPerElem(pnt, Point3(scaleVec));
}

inline float projection(const Point3 & pnt, const Vector3 & unitVec)
{
    float result;
    result = (pnt.getX() * unitVec.getX());
    result = (result + (pnt.getY() * unitVec.getY()));
    result = (result + (pnt.getZ() * unitVec.getZ()));
    return result;
}

inline float distSqrFromOrigin(const Point3 & pnt)
{
    return lengthSqr(Vector3(pnt));
}

inline float distFromOrigin(const Point3 & pnt)
{
    return length(Vector3(pnt));
}

inline float distSqr(const Point3 & pnt0, const Point3 & pnt1)
{
    return lengthSqr(pnt1 - pnt0);
}

inline float distp(const Point3 & pnt0, const Point3 & pnt1)
{
    return length(pnt1 - pnt0);
}

inline const Point3 select(const Point3 & pnt0, const Point3 & pnt1, bool select1)
{
    return Point3((select1) ? pnt1.getX() : pnt0.getX(),
                  (select1) ? pnt1.getY() : pnt0.getY(),
                  (select1) ? pnt1.getZ() : pnt0.getZ());
}

#ifdef VECTORMATH_DEBUG

inline void print(const Point3 & pnt)
{
    printf("( %f %f %f )\n", pnt.getX(), pnt.getY(), pnt.getZ());
}

inline void print(const Point3 & pnt, const char * name)
{
    printf("%s: ( %f %f %f )\n", name, pnt.getX(), pnt.getY(), pnt.getZ());
}

#endif // VECTORMATH_DEBUG

} // namespace Scalar
} // namespace Vectormath

#endif // VECTORMATH_SCALAR_VECTOR_HPP

#ifndef VECTORMATH_SCALAR_QUATERNION_HPP
#define VECTORMATH_SCALAR_QUATERNION_HPP

namespace Vectormath
{
namespace Scalar
{

// ========================================================
// Quat
// ========================================================

inline Quat::Quat(const Quat & quat)
{
    mX = quat.mX;
    mY = quat.mY;
    mZ = quat.mZ;
    mW = quat.mW;
}

inline Quat::Quat(float _x, float _y, float _z, float _w)
{
    mX = _x;
    mY = _y;
    mZ = _z;
    mW = _w;
}

inline Quat::Quat(const Vector3 & xyz, float _w)
{
    this->setXYZ(xyz);
    this->setW(_w);
}

inline Quat::Quat(const Vector4 & vec)
{
    mX = vec.getX();
    mY = vec.getY();
    mZ = vec.getZ();
    mW = vec.getW();
}

inline Quat::Quat(float scalar)
{
    mX = scalar;
    mY = scalar;
    mZ = scalar;
    mW = scalar;
}

inline const Quat Quat::identity()
{
    return Quat(0.0f, 0.0f, 0.0f, 1.0f);
}

inline const Quat lerp(float t, const Quat & quat0, const Quat & quat1)
{
    return (quat0 + ((quat1 - quat0) * t));
}

inline const Quat slerp(float t, const Quat & unitQuat0, const Quat & unitQuat1)
{
    Quat start;
    float recipSinAngle, scale0, scale1, cosAngle, angle;
    cosAngle = dot(unitQuat0, unitQuat1);
    if (cosAngle < 0.0f)
    {
        cosAngle = -cosAngle;
        start = (-unitQuat0);
    }
    else
    {
        start = unitQuat0;
    }
    if (cosAngle < VECTORMATH_SLERP_TOL)
    {
        angle = acosf(cosAngle);
        recipSinAngle = (1.0f / sinf(angle));
        scale0 = (sinf(((1.0f - t) * angle)) * recipSinAngle);
        scale1 = (sinf((t * angle)) * recipSinAngle);
    }
    else
    {
        scale0 = (1.0f - t);
        scale1 = t;
    }
    return ((start * scale0) + (unitQuat1 * scale1));
}

inline const Quat squad(float t, const Quat & unitQuat0, const Quat & unitQuat1, const Quat & unitQuat2, const Quat & unitQuat3)
{
    Quat tmp0, tmp1;
    tmp0 = slerp(t, unitQuat0, unitQuat3);
    tmp1 = slerp(t, unitQuat1, unitQuat2);
    return slerp(((2.0f * t) * (1.0f - t)), tmp0, tmp1);
}

inline Quat & Quat::operator = (const Quat & quat)
{
    mX = quat.mX;
    mY = quat.mY;
    mZ = quat.mZ;
    mW = quat.mW;
    return *this;
}

inline Quat & Quat::setXYZ(const Vector3 & vec)
{
    mX = vec.getX();
    mY = vec.getY();
    mZ = vec.getZ();
    return *this;
}

inline const Vector3 Quat::getXYZ() const
{
    return Vector3(mX, mY, mZ);
}

inline Quat & Quat::setX(float _x)
{
    mX = _x;
    return *this;
}

inline float Quat::getX() const
{
    return mX;
}

inline Quat & Quat::setY(float _y)
{
    mY = _y;
    return *this;
}

inline float Quat::getY() const
{
    return mY;
}

inline Quat & Quat::setZ(float _z)
{
    mZ = _z;
    return *this;
}

inline float Quat::getZ() const
{
    return mZ;
}

inline Quat & Quat::setW(float _w)
{
    mW = _w;
    return *this;
}

inline float Quat::getW() const
{
    return mW;
}

inline Quat & Quat::setElem(int idx, float value)
{
    *(&mX + idx) = value;
    return *this;
}

inline float Quat::getElem(int idx) const
{
    return *(&mX + idx);
}

inline float & Quat::operator[](int idx)
{
    return *(&mX + idx);
}

inline float Quat::operator[](int idx) const
{
    return *(&mX + idx);
}

inline const Quat Quat::operator + (const Quat & quat) const
{
    return Quat((mX + quat.mX),
                (mY + quat.mY),
                (mZ + quat.mZ),
                (mW + quat.mW));
}

inline const Quat Quat::operator - (const Quat & quat) const
{
    return Quat((mX - quat.mX),
                (mY - quat.mY),
                (mZ - quat.mZ),
                (mW - quat.mW));
}

inline const Quat Quat::operator * (float scalar) const
{
    return Quat((mX * scalar),
                (mY * scalar),
                (mZ * scalar),
                (mW * scalar));
}

inline Quat & Quat::operator += (const Quat & quat)
{
    *this = *this + quat;
    return *this;
}

inline Quat & Quat::operator -= (const Quat & quat)
{
    *this = *this - quat;
    return *this;
}

inline Quat & Quat::operator *= (float scalar)
{
    *this = *this * scalar;
    return *this;
}

inline const Quat Quat::operator / (float scalar) const
{
    return Quat((mX / scalar),
                (mY / scalar),
                (mZ / scalar),
                (mW / scalar));
}

inline Quat & Quat::operator /= (float scalar)
{
    *this = *this / scalar;
    return *this;
}

inline const Quat Quat::operator - () const
{
    return Quat(-mX, -mY, -mZ, -mW);
}

inline const Quat operator * (float scalar, const Quat & quat)
{
    return quat * scalar;
}

inline float dot(const Quat & quat0, const Quat & quat1)
{
    float result;
    result = (quat0.getX() * quat1.getX());
    result = (result + (quat0.getY() * quat1.getY()));
    result = (result + (quat0.getZ() * quat1.getZ()));
    result = (result + (quat0.getW() * quat1.getW()));
    return result;
}

inline float norm(const Quat & quat)
{
    float result;
    result = (quat.getX() * quat.getX());
    result = (result + (quat.getY() * quat.getY()));
    result = (result + (quat.getZ() * quat.getZ()));
    result = (result + (quat.getW() * quat.getW()));
    return result;
}

inline float length(const Quat & quat)
{
    return sqrtf(norm(quat));
}

inline const Quat normalize(const Quat & quat)
{
    float lenSqr, lenInv;
    lenSqr = norm(quat);
    lenInv = (1.0f / sqrtf(lenSqr));
    return Quat((quat.getX() * lenInv),
                (quat.getY() * lenInv),
                (quat.getZ() * lenInv),
                (quat.getW() * lenInv));
}

inline const Quat Quat::rotation(const Vector3 & unitVec0, const Vector3 & unitVec1)
{
    float cosHalfAngleX2, recipCosHalfAngleX2;
    cosHalfAngleX2 = sqrtf((2.0f * (1.0f + dot(unitVec0, unitVec1))));
    recipCosHalfAngleX2 = (1.0f / cosHalfAngleX2);
    return Quat((cross(unitVec0, unitVec1) * recipCosHalfAngleX2), (cosHalfAngleX2 * 0.5f));
}

inline const Quat Quat::rotation(float radians, const Vector3 & unitVec)
{
    float s, c, angle;
    angle = (radians * 0.5f);
    s = sinf(angle);
    c = cosf(angle);
    return Quat((unitVec * s), c);
}

inline const Quat Quat::rotationX(float radians)
{
    float s, c, angle;
    angle = (radians * 0.5f);
    s = sinf(angle);
    c = cosf(angle);
    return Quat(s, 0.0f, 0.0f, c);
}

inline const Quat Quat::rotationY(float radians)
{
    float s, c, angle;
    angle = (radians * 0.5f);
    s = sinf(angle);
    c = cosf(angle);
    return Quat(0.0f, s, 0.0f, c);
}

inline const Quat Quat::rotationZ(float radians)
{
    float s, c, angle;
    angle = (radians * 0.5f);
    s = sinf(angle);
    c = cosf(angle);
    return Quat(0.0f, 0.0f, s, c);
}

inline const Quat Quat::operator * (const Quat & quat) const
{
    return Quat(((((mW * quat.mX) + (mX * quat.mW)) + (mY * quat.mZ)) - (mZ * quat.mY)),
                ((((mW * quat.mY) + (mY * quat.mW)) + (mZ * quat.mX)) - (mX * quat.mZ)),
                ((((mW * quat.mZ) + (mZ * quat.mW)) + (mX * quat.mY)) - (mY * quat.mX)),
                ((((mW * quat.mW) - (mX * quat.mX)) - (mY * quat.mY)) - (mZ * quat.mZ)));
}

inline Quat & Quat::operator *= (const Quat & quat)
{
    *this = *this * quat;
    return *this;
}

inline const Vector3 rotate(const Quat & quat, const Vector3 & vec)
{
    float tmpX, tmpY, tmpZ, tmpW;
    tmpX = (((quat.getW() * vec.getX()) + (quat.getY() * vec.getZ())) - (quat.getZ() * vec.getY()));
    tmpY = (((quat.getW() * vec.getY()) + (quat.getZ() * vec.getX())) - (quat.getX() * vec.getZ()));
    tmpZ = (((quat.getW() * vec.getZ()) + (quat.getX() * vec.getY())) - (quat.getY() * vec.getX()));
    tmpW = (((quat.getX() * vec.getX()) + (quat.getY() * vec.getY())) + (quat.getZ() * vec.getZ()));
    return Vector3(((((tmpW * quat.getX()) + (tmpX * quat.getW())) - (tmpY * quat.getZ())) + (tmpZ * quat.getY())),
                   ((((tmpW * quat.getY()) + (tmpY * quat.getW())) - (tmpZ * quat.getX())) + (tmpX * quat.getZ())),
                   ((((tmpW * quat.getZ()) + (tmpZ * quat.getW())) - (tmpX * quat.getY())) + (tmpY * quat.getX())));
}

inline const Quat conj(const Quat & quat)
{
    return Quat(-quat.getX(), -quat.getY(), -quat.getZ(), quat.getW());
}

inline const Quat select(const Quat & quat0, const Quat & quat1, bool select1)
{
    return Quat((select1) ? quat1.getX() : quat0.getX(),
                (select1) ? quat1.getY() : quat0.getY(),
                (select1) ? quat1.getZ() : quat0.getZ(),
                (select1) ? quat1.getW() : quat0.getW());
}

#ifdef VECTORMATH_DEBUG

inline void print(const Quat & quat)
{
    printf("( %f %f %f %f )\n", quat.getX(), quat.getY(), quat.getZ(), quat.getW());
}

inline void print(const Quat & quat, const char * name)
{
    printf("%s: ( %f %f %f %f )\n", name, quat.getX(), quat.getY(), quat.getZ(), quat.getW());
}

#endif // VECTORMATH_DEBUG

} // namespace Scalar
} // namespace Vectormath

#endif // VECTORMATH_SCALAR_QUATERNION_HPP

#ifndef VECTORMATH_SCALAR_MATRIX_HPP
#define VECTORMATH_SCALAR_MATRIX_HPP

namespace Vectormath
{
namespace Scalar
{

// ========================================================
// Matrix3
// ========================================================

inline Matrix3::Matrix3(const Matrix3 & mat)
{
    mCol0 = mat.mCol0;
    mCol1 = mat.mCol1;
    mCol2 = mat.mCol2;
}

inline Matrix3::Matrix3(float scalar)
{
    mCol0 = Vector3(scalar);
    mCol1 = Vector3(scalar);
    mCol2 = Vector3(scalar);
}

inline Matrix3::Matrix3(const Quat & unitQuat)
{
    float qx, qy, qz, qw, qx2, qy2, qz2, qxqx2, qyqy2, qzqz2, qxqy2, qyqz2, qzqw2, qxqz2, qyqw2, qxqw2;
    qx = unitQuat.getX();
    qy = unitQuat.getY();
    qz = unitQuat.getZ();
    qw = unitQuat.getW();
    qx2 = (qx + qx);
    qy2 = (qy + qy);
    qz2 = (qz + qz);
    qxqx2 = (qx * qx2);
    qxqy2 = (qx * qy2);
    qxqz2 = (qx * qz2);
    qxqw2 = (qw * qx2);
    qyqy2 = (qy * qy2);
    qyqz2 = (qy * qz2);
    qyqw2 = (qw * qy2);
    qzqz2 = (qz * qz2);
    qzqw2 = (qw * qz2);
    mCol0 = Vector3(((1.0f - qyqy2) - qzqz2), (qxqy2 + qzqw2), (qxqz2 - qyqw2));
    mCol1 = Vector3((qxqy2 - qzqw2), ((1.0f - qxqx2) - qzqz2), (qyqz2 + qxqw2));
    mCol2 = Vector3((qxqz2 + qyqw2), (qyqz2 - qxqw2), ((1.0f - qxqx2) - qyqy2));
}

inline Matrix3::Matrix3(const Vector3 & _col0, const Vector3 & _col1, const Vector3 & _col2)
{
    mCol0 = _col0;
    mCol1 = _col1;
    mCol2 = _col2;
}

inline Matrix3 & Matrix3::setCol0(const Vector3 & _col0)
{
    mCol0 = _col0;
    return *this;
}

inline Matrix3 & Matrix3::setCol1(const Vector3 & _col1)
{
    mCol1 = _col1;
    return *this;
}

inline Matrix3 & Matrix3::setCol2(const Vector3 & _col2)
{
    mCol2 = _col2;
    return *this;
}

inline Matrix3 & Matrix3::setCol(int col, const Vector3 & vec)
{
    *(&mCol0 + col) = vec;
    return *this;
}

inline Matrix3 & Matrix3::setRow(int row, const Vector3 & vec)
{
    mCol0.setElem(row, vec.getElem(0));
    mCol1.setElem(row, vec.getElem(1));
    mCol2.setElem(row, vec.getElem(2));
    return *this;
}

inline Matrix3 & Matrix3::setElem(int col, int row, float val)
{
    Vector3 tmpV3_0;
    tmpV3_0 = this->getCol(col);
    tmpV3_0.setElem(row, val);
    this->setCol(col, tmpV3_0);
    return *this;
}

inline float Matrix3::getElem(int col, int row) const
{
    return this->getCol(col).getElem(row);
}

inline const Vector3 Matrix3::getCol0() const
{
    return mCol0;
}

inline const Vector3 Matrix3::getCol1() const
{
    return mCol1;
}

inline const Vector3 Matrix3::getCol2() const
{
    return mCol2;
}

inline const Vector3 Matrix3::getCol(int col) const
{
    return *(&mCol0 + col);
}

inline const Vector3 Matrix3::getRow(int row) const
{
    return Vector3(mCol0.getElem(row), mCol1.getElem(row), mCol2.getElem(row));
}

inline Vector3 & Matrix3::operator[](int col)
{
    return *(&mCol0 + col);
}

inline const Vector3 Matrix3::operator[](int col) const
{
    return *(&mCol0 + col);
}

inline Matrix3 & Matrix3::operator = (const Matrix3 & mat)
{
    mCol0 = mat.mCol0;
    mCol1 = mat.mCol1;
    mCol2 = mat.mCol2;
    return *this;
}

inline const Matrix3 transpose(const Matrix3 & mat)
{
    return Matrix3(
        Vector3(mat.getCol0().getX(), mat.getCol1().getX(), mat.getCol2().getX()),
        Vector3(mat.getCol0().getY(), mat.getCol1().getY(), mat.getCol2().getY()),
        Vector3(mat.getCol0().getZ(), mat.getCol1().getZ(), mat.getCol2().getZ()));
}

inline const Matrix3 inverse(const Matrix3 & mat)
{
    Vector3 tmp0, tmp1, tmp2;
    float detinv;
    tmp0 = cross(mat.getCol1(), mat.getCol2());
    tmp1 = cross(mat.getCol2(), mat.getCol0());
    tmp2 = cross(mat.getCol0(), mat.getCol1());
    detinv = (1.0f / dot(mat.getCol2(), tmp2));
    return Matrix3(
        Vector3((tmp0.getX() * detinv), (tmp1.getX() * detinv), (tmp2.getX() * detinv)),
        Vector3((tmp0.getY() * detinv), (tmp1.getY() * detinv), (tmp2.getY() * detinv)),
        Vector3((tmp0.getZ() * detinv), (tmp1.getZ() * detinv), (tmp2.getZ() * detinv)));
}

inline float determinant(const Matrix3 & mat)
{
    return dot(mat.getCol2(), cross(mat.getCol0(), mat.getCol1()));
}

inline const Matrix3 Matrix3::operator + (const Matrix3 & mat) const
{
    return Matrix3((mCol0 + mat.mCol0),
                   (mCol1 + mat.mCol1),
                   (mCol2 + mat.mCol2));
}

inline const Matrix3 Matrix3::operator - (const Matrix3 & mat) const
{
    return Matrix3((mCol0 - mat.mCol0),
                   (mCol1 - mat.mCol1),
                   (mCol2 - mat.mCol2));
}

inline Matrix3 & Matrix3::operator += (const Matrix3 & mat)
{
    *this = *this + mat;
    return *this;
}

inline Matrix3 & Matrix3::operator -= (const Matrix3 & mat)
{
    *this = *this - mat;
    return *this;
}

inline const Matrix3 Matrix3::operator - () const
{
    return Matrix3((-mCol0), (-mCol1), (-mCol2));
}

inline const Matrix3 absPerElem(const Matrix3 & mat)
{
    return Matrix3(absPerElem(mat.getCol0()),
                   absPerElem(mat.getCol1()),
                   absPerElem(mat.getCol2()));
}

inline const Matrix3 Matrix3::operator * (float scalar) const
{
    return Matrix3((mCol0 * scalar), (mCol1 * scalar), (mCol2 * scalar));
}

inline Matrix3 & Matrix3::operator *= (float scalar)
{
    *this = *this * scalar;
    return *this;
}

inline const Matrix3 operator * (float scalar, const Matrix3 & mat)
{
    return mat * scalar;
}

inline const Vector3 Matrix3::operator * (const Vector3 & vec) const
{
    return Vector3((((mCol0.getX() * vec.getX()) + (mCol1.getX() * vec.getY())) + (mCol2.getX() * vec.getZ())),
                   (((mCol0.getY() * vec.getX()) + (mCol1.getY() * vec.getY())) + (mCol2.getY() * vec.getZ())),
                   (((mCol0.getZ() * vec.getX()) + (mCol1.getZ() * vec.getY())) + (mCol2.getZ() * vec.getZ())));
}

inline const Matrix3 Matrix3::operator * (const Matrix3 & mat) const
{
    return Matrix3((*this * mat.mCol0), (*this * mat.mCol1), (*this * mat.mCol2));
}

inline Matrix3 & Matrix3::operator *= (const Matrix3 & mat)
{
    *this = *this * mat;
    return *this;
}

inline const Matrix3 mulPerElem(const Matrix3 & mat0, const Matrix3 & mat1)
{
    return Matrix3(mulPerElem(mat0.getCol0(), mat1.getCol0()),
                   mulPerElem(mat0.getCol1(), mat1.getCol1()),
                   mulPerElem(mat0.getCol2(), mat1.getCol2()));
}

inline const Matrix3 Matrix3::identity()
{
    return Matrix3(Vector3::xAxis(), Vector3::yAxis(), Vector3::zAxis());
}

inline const Matrix3 Matrix3::rotationX(float radians)
{
    float s, c;
    s = sinf(radians);
    c = cosf(radians);
    return Matrix3(Vector3::xAxis(), Vector3(0.0f, c, s), Vector3(0.0f, -s, c));
}

inline const Matrix3 Matrix3::rotationY(float radians)
{
    float s, c;
    s = sinf(radians);
    c = cosf(radians);
    return Matrix3(Vector3(c, 0.0f, -s), Vector3::yAxis(), Vector3(s, 0.0f, c));
}

inline const Matrix3 Matrix3::rotationZ(float radians)
{
    float s, c;
    s = sinf(radians);
    c = cosf(radians);
    return Matrix3(Vector3(c, s, 0.0f), Vector3(-s, c, 0.0f), Vector3::zAxis());
}

inline const Matrix3 Matrix3::rotationZYX(const Vector3 & radiansXYZ)
{
    float sX, cX, sY, cY, sZ, cZ, tmp0, tmp1;
    sX = sinf(radiansXYZ.getX());
    cX = cosf(radiansXYZ.getX());
    sY = sinf(radiansXYZ.getY());
    cY = cosf(radiansXYZ.getY());
    sZ = sinf(radiansXYZ.getZ());
    cZ = cosf(radiansXYZ.getZ());
    tmp0 = (cZ * sY);
    tmp1 = (sZ * sY);
    return Matrix3(Vector3((cZ * cY), (sZ * cY), -sY),
                   Vector3(((tmp0 * sX) - (sZ * cX)), ((tmp1 * sX) + (cZ * cX)), (cY * sX)),
                   Vector3(((tmp0 * cX) + (sZ * sX)), ((tmp1 * cX) - (cZ * sX)), (cY * cX)));
}

inline const Matrix3 Matrix3::rotation(float radians, const Vector3 & unitVec)
{
    float x, y, z, s, c, oneMinusC, xy, yz, zx;
    s = sinf(radians);
    c = cosf(radians);
    x = unitVec.getX();
    y = unitVec.getY();
    z = unitVec.getZ();
    xy = (x * y);
    yz = (y * z);
    zx = (z * x);
    oneMinusC = (1.0f - c);
    return Matrix3(Vector3((((x * x) * oneMinusC) + c), ((xy * oneMinusC) + (z * s)), ((zx * oneMinusC) - (y * s))),
                   Vector3(((xy * oneMinusC) - (z * s)), (((y * y) * oneMinusC) + c), ((yz * oneMinusC) + (x * s))),
                   Vector3(((zx * oneMinusC) + (y * s)), ((yz * oneMinusC) - (x * s)), (((z * z) * oneMinusC) + c)));
}

inline const Matrix3 Matrix3::rotation(const Quat & unitQuat)
{
    return Matrix3(unitQuat);
}

inline const Matrix3 Matrix3::scale(const Vector3 & scaleVec)
{
    return Matrix3(Vector3(scaleVec.getX(), 0.0f, 0.0f),
                   Vector3(0.0f, scaleVec.getY(), 0.0f),
                   Vector3(0.0f, 0.0f, scaleVec.getZ()));
}

inline const Matrix3 appendScale(const Matrix3 & mat, const Vector3 & scaleVec)
{
    return Matrix3((mat.getCol0() * scaleVec.getX()),
                   (mat.getCol1() * scaleVec.getY()),
                   (mat.getCol2() * scaleVec.getZ()));
}

inline const Matrix3 prependScale(const Vector3 & scaleVec, const Matrix3 & mat)
{
    return Matrix3(mulPerElem(mat.getCol0(), scaleVec),
                   mulPerElem(mat.getCol1(), scaleVec),
                   mulPerElem(mat.getCol2(), scaleVec));
}

inline const Matrix3 select(const Matrix3 & mat0, const Matrix3 & mat1, bool select1)
{
    return Matrix3(select(mat0.getCol0(), mat1.getCol0(), select1),
                   select(mat0.getCol1(), mat1.getCol1(), select1),
                   select(mat0.getCol2(), mat1.getCol2(), select1));
}

#ifdef VECTORMATH_DEBUG

inline void print(const Matrix3 & mat)
{
    print(mat.getRow(0));
    print(mat.getRow(1));
    print(mat.getRow(2));
}

inline void print(const Matrix3 & mat, const char * name)
{
    printf("%s:\n", name);
    print(mat);
}

#endif // VECTORMATH_DEBUG

// ========================================================
// Matrix4
// ========================================================

inline Matrix4::Matrix4(const Matrix4 & mat)
{
    mCol0 = mat.mCol0;
    mCol1 = mat.mCol1;
    mCol2 = mat.mCol2;
    mCol3 = mat.mCol3;
}

inline Matrix4::Matrix4(float scalar)
{
    mCol0 = Vector4(scalar);
    mCol1 = Vector4(scalar);
    mCol2 = Vector4(scalar);
    mCol3 = Vector4(scalar);
}

inline Matrix4::Matrix4(const Transform3 & mat)
{
    mCol0 = Vector4(mat.getCol0(), 0.0f);
    mCol1 = Vector4(mat.getCol1(), 0.0f);
    mCol2 = Vector4(mat.getCol2(), 0.0f);
    mCol3 = Vector4(mat.getCol3(), 1.0f);
}

inline Matrix4::Matrix4(const Vector4 & _col0, const Vector4 & _col1, const Vector4 & _col2, const Vector4 & _col3)
{
    mCol0 = _col0;
    mCol1 = _col1;
    mCol2 = _col2;
    mCol3 = _col3;
}

inline Matrix4::Matrix4(const Matrix3 & mat, const Vector3 & translateVec)
{
    mCol0 = Vector4(mat.getCol0(), 0.0f);
    mCol1 = Vector4(mat.getCol1(), 0.0f);
    mCol2 = Vector4(mat.getCol2(), 0.0f);
    mCol3 = Vector4(translateVec,  1.0f);
}

inline Matrix4::Matrix4(const Quat & unitQuat, const Vector3 & translateVec)
{
    Matrix3 mat;
    mat = Matrix3(unitQuat);
    mCol0 = Vector4(mat.getCol0(), 0.0f);
    mCol1 = Vector4(mat.getCol1(), 0.0f);
    mCol2 = Vector4(mat.getCol2(), 0.0f);
    mCol3 = Vector4(translateVec,  1.0f);
}

inline Matrix4 & Matrix4::setCol0(const Vector4 & _col0)
{
    mCol0 = _col0;
    return *this;
}

inline Matrix4 & Matrix4::setCol1(const Vector4 & _col1)
{
    mCol1 = _col1;
    return *this;
}

inline Matrix4 & Matrix4::setCol2(const Vector4 & _col2)
{
    mCol2 = _col2;
    return *this;
}

inline Matrix4 & Matrix4::setCol3(const Vector4 & _col3)
{
    mCol3 = _col3;
    return *this;
}

inline Matrix4 & Matrix4::setCol(int col, const Vector4 & vec)
{
    *(&mCol0 + col) = vec;
    return *this;
}

inline Matrix4 & Matrix4::setRow(int row, const Vector4 & vec)
{
    mCol0.setElem(row, vec.getElem(0));
    mCol1.setElem(row, vec.getElem(1));
    mCol2.setElem(row, vec.getElem(2));
    mCol3.setElem(row, vec.getElem(3));
    return *this;
}

inline Matrix4 & Matrix4::setElem(int col, int row, float val)
{
    Vector4 tmpV3_0;
    tmpV3_0 = this->getCol(col);
    tmpV3_0.setElem(row, val);
    this->setCol(col, tmpV3_0);
    return *this;
}

inline float Matrix4::getElem(int col, int row) const
{
    return this->getCol(col).getElem(row);
}

inline const Vector4 Matrix4::getCol0() const
{
    return mCol0;
}

inline const Vector4 Matrix4::getCol1() const
{
    return mCol1;
}

inline const Vector4 Matrix4::getCol2() const
{
    return mCol2;
}

inline const Vector4 Matrix4::getCol3() const
{
    return mCol3;
}

inline const Vector4 Matrix4::getCol(int col) const
{
    return *(&mCol0 + col);
}

inline const Vector4 Matrix4::getRow(int row) const
{
    return Vector4(mCol0.getElem(row), mCol1.getElem(row), mCol2.getElem(row), mCol3.getElem(row));
}

inline Vector4 & Matrix4::operator[](int col)
{
    return *(&mCol0 + col);
}

inline const Vector4 Matrix4::operator[](int col) const
{
    return *(&mCol0 + col);
}

inline Matrix4 & Matrix4::operator = (const Matrix4 & mat)
{
    mCol0 = mat.mCol0;
    mCol1 = mat.mCol1;
    mCol2 = mat.mCol2;
    mCol3 = mat.mCol3;
    return *this;
}

inline const Matrix4 transpose(const Matrix4 & mat)
{
    return Matrix4(Vector4(mat.getCol0().getX(), mat.getCol1().getX(), mat.getCol2().getX(), mat.getCol3().getX()),
                   Vector4(mat.getCol0().getY(), mat.getCol1().getY(), mat.getCol2().getY(), mat.getCol3().getY()),
                   Vector4(mat.getCol0().getZ(), mat.getCol1().getZ(), mat.getCol2().getZ(), mat.getCol3().getZ()),
                   Vector4(mat.getCol0().getW(), mat.getCol1().getW(), mat.getCol2().getW(), mat.getCol3().getW()));
}

inline const Matrix4 inverse(const Matrix4 & mat)
{
    Vector4 res0, res1, res2, res3;
    float mA, mB, mC, mD, mE, mF, mG, mH, mI, mJ, mK, mL, mM, mN, mO, mP, tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, detInv;
    mA = mat.getCol0().getX();
    mB = mat.getCol0().getY();
    mC = mat.getCol0().getZ();
    mD = mat.getCol0().getW();
    mE = mat.getCol1().getX();
    mF = mat.getCol1().getY();
    mG = mat.getCol1().getZ();
    mH = mat.getCol1().getW();
    mI = mat.getCol2().getX();
    mJ = mat.getCol2().getY();
    mK = mat.getCol2().getZ();
    mL = mat.getCol2().getW();
    mM = mat.getCol3().getX();
    mN = mat.getCol3().getY();
    mO = mat.getCol3().getZ();
    mP = mat.getCol3().getW();
    tmp0 = ((mK * mD) - (mC * mL));
    tmp1 = ((mO * mH) - (mG * mP));
    tmp2 = ((mB * mK) - (mJ * mC));
    tmp3 = ((mF * mO) - (mN * mG));
    tmp4 = ((mJ * mD) - (mB * mL));
    tmp5 = ((mN * mH) - (mF * mP));
    res0.setX((((mJ * tmp1) - (mL * tmp3)) - (mK * tmp5)));
    res0.setY((((mN * tmp0) - (mP * tmp2)) - (mO * tmp4)));
    res0.setZ((((mD * tmp3) + (mC * tmp5)) - (mB * tmp1)));
    res0.setW((((mH * tmp2) + (mG * tmp4)) - (mF * tmp0)));
    detInv = (1.0f / ((((mA * res0.getX()) + (mE * res0.getY())) + (mI * res0.getZ())) + (mM * res0.getW())));
    res1.setX((mI * tmp1));
    res1.setY((mM * tmp0));
    res1.setZ((mA * tmp1));
    res1.setW((mE * tmp0));
    res3.setX((mI * tmp3));
    res3.setY((mM * tmp2));
    res3.setZ((mA * tmp3));
    res3.setW((mE * tmp2));
    res2.setX((mI * tmp5));
    res2.setY((mM * tmp4));
    res2.setZ((mA * tmp5));
    res2.setW((mE * tmp4));
    tmp0 = ((mI * mB) - (mA * mJ));
    tmp1 = ((mM * mF) - (mE * mN));
    tmp2 = ((mI * mD) - (mA * mL));
    tmp3 = ((mM * mH) - (mE * mP));
    tmp4 = ((mI * mC) - (mA * mK));
    tmp5 = ((mM * mG) - (mE * mO));
    res2.setX((((mL * tmp1) - (mJ * tmp3)) + res2.getX()));
    res2.setY((((mP * tmp0) - (mN * tmp2)) + res2.getY()));
    res2.setZ((((mB * tmp3) - (mD * tmp1)) - res2.getZ()));
    res2.setW((((mF * tmp2) - (mH * tmp0)) - res2.getW()));
    res3.setX((((mJ * tmp5) - (mK * tmp1)) + res3.getX()));
    res3.setY((((mN * tmp4) - (mO * tmp0)) + res3.getY()));
    res3.setZ((((mC * tmp1) - (mB * tmp5)) - res3.getZ()));
    res3.setW((((mG * tmp0) - (mF * tmp4)) - res3.getW()));
    res1.setX((((mK * tmp3) - (mL * tmp5)) - res1.getX()));
    res1.setY((((mO * tmp2) - (mP * tmp4)) - res1.getY()));
    res1.setZ((((mD * tmp5) - (mC * tmp3)) + res1.getZ()));
    res1.setW((((mH * tmp4) - (mG * tmp2)) + res1.getW()));
    return Matrix4((res0 * detInv), (res1 * detInv), (res2 * detInv), (res3 * detInv));
}

inline const Matrix4 affineInverse(const Matrix4 & mat)
{
    Transform3 affineMat;
    affineMat.setCol0(mat.getCol0().getXYZ());
    affineMat.setCol1(mat.getCol1().getXYZ());
    affineMat.setCol2(mat.getCol2().getXYZ());
    affineMat.setCol3(mat.getCol3().getXYZ());
    return Matrix4(inverse(affineMat));
}

inline const Matrix4 orthoInverse(const Matrix4 & mat)
{
    Transform3 affineMat;
    affineMat.setCol0(mat.getCol0().getXYZ());
    affineMat.setCol1(mat.getCol1().getXYZ());
    affineMat.setCol2(mat.getCol2().getXYZ());
    affineMat.setCol3(mat.getCol3().getXYZ());
    return Matrix4(orthoInverse(affineMat));
}

inline float determinant(const Matrix4 & mat)
{
    float dx, dy, dz, dw, mA, mB, mC, mD, mE, mF, mG, mH, mI, mJ, mK, mL, mM, mN, mO, mP, tmp0, tmp1, tmp2, tmp3, tmp4, tmp5;
    mA = mat.getCol0().getX();
    mB = mat.getCol0().getY();
    mC = mat.getCol0().getZ();
    mD = mat.getCol0().getW();
    mE = mat.getCol1().getX();
    mF = mat.getCol1().getY();
    mG = mat.getCol1().getZ();
    mH = mat.getCol1().getW();
    mI = mat.getCol2().getX();
    mJ = mat.getCol2().getY();
    mK = mat.getCol2().getZ();
    mL = mat.getCol2().getW();
    mM = mat.getCol3().getX();
    mN = mat.getCol3().getY();
    mO = mat.getCol3().getZ();
    mP = mat.getCol3().getW();
    tmp0 = ((mK * mD) - (mC * mL));
    tmp1 = ((mO * mH) - (mG * mP));
    tmp2 = ((mB * mK) - (mJ * mC));
    tmp3 = ((mF * mO) - (mN * mG));
    tmp4 = ((mJ * mD) - (mB * mL));
    tmp5 = ((mN * mH) - (mF * mP));
    dx = (((mJ * tmp1) - (mL * tmp3)) - (mK * tmp5));
    dy = (((mN * tmp0) - (mP * tmp2)) - (mO * tmp4));
    dz = (((mD * tmp3) + (mC * tmp5)) - (mB * tmp1));
    dw = (((mH * tmp2) + (mG * tmp4)) - (mF * tmp0));
    return ((((mA * dx) + (mE * dy)) + (mI * dz)) + (mM * dw));
}

inline const Matrix4 Matrix4::operator + (const Matrix4 & mat) const
{
    return Matrix4((mCol0 + mat.mCol0),
                   (mCol1 + mat.mCol1),
                   (mCol2 + mat.mCol2),
                   (mCol3 + mat.mCol3));
}

inline const Matrix4 Matrix4::operator - (const Matrix4 & mat) const
{
    return Matrix4((mCol0 - mat.mCol0),
                   (mCol1 - mat.mCol1),
                   (mCol2 - mat.mCol2),
                   (mCol3 - mat.mCol3));
}

inline Matrix4 & Matrix4::operator += (const Matrix4 & mat)
{
    *this = *this + mat;
    return *this;
}

inline Matrix4 & Matrix4::operator -= (const Matrix4 & mat)
{
    *this = *this - mat;
    return *this;
}

inline const Matrix4 Matrix4::operator - () const
{
    return Matrix4((-mCol0), (-mCol1), (-mCol2), (-mCol3));
}

inline const Matrix4 absPerElem(const Matrix4 & mat)
{
    return Matrix4(absPerElem(mat.getCol0()),
                   absPerElem(mat.getCol1()),
                   absPerElem(mat.getCol2()),
                   absPerElem(mat.getCol3()));
}

inline const Matrix4 Matrix4::operator * (float scalar) const
{
    return Matrix4((mCol0 * scalar),
                   (mCol1 * scalar),
                   (mCol2 * scalar),
                   (mCol3 * scalar));
}

inline Matrix4 & Matrix4::operator *= (float scalar)
{
    *this = *this * scalar;
    return *this;
}

inline const Matrix4 operator * (float scalar, const Matrix4 & mat)
{
    return mat * scalar;
}

inline const Vector4 Matrix4::operator * (const Vector4 & vec) const
{
    return Vector4(((((mCol0.getX() * vec.getX()) + (mCol1.getX() * vec.getY())) + (mCol2.getX() * vec.getZ())) + (mCol3.getX() * vec.getW())),
                   ((((mCol0.getY() * vec.getX()) + (mCol1.getY() * vec.getY())) + (mCol2.getY() * vec.getZ())) + (mCol3.getY() * vec.getW())),
                   ((((mCol0.getZ() * vec.getX()) + (mCol1.getZ() * vec.getY())) + (mCol2.getZ() * vec.getZ())) + (mCol3.getZ() * vec.getW())),
                   ((((mCol0.getW() * vec.getX()) + (mCol1.getW() * vec.getY())) + (mCol2.getW() * vec.getZ())) + (mCol3.getW() * vec.getW())));
}

inline const Vector4 Matrix4::operator * (const Vector3 & vec) const
{
    return Vector4((((mCol0.getX() * vec.getX()) + (mCol1.getX() * vec.getY())) + (mCol2.getX() * vec.getZ())),
                   (((mCol0.getY() * vec.getX()) + (mCol1.getY() * vec.getY())) + (mCol2.getY() * vec.getZ())),
                   (((mCol0.getZ() * vec.getX()) + (mCol1.getZ() * vec.getY())) + (mCol2.getZ() * vec.getZ())),
                   (((mCol0.getW() * vec.getX()) + (mCol1.getW() * vec.getY())) + (mCol2.getW() * vec.getZ())));
}

inline const Vector4 Matrix4::operator * (const Point3 & pnt) const
{
    return Vector4(((((mCol0.getX() * pnt.getX()) + (mCol1.getX() * pnt.getY())) + (mCol2.getX() * pnt.getZ())) + mCol3.getX()),
                   ((((mCol0.getY() * pnt.getX()) + (mCol1.getY() * pnt.getY())) + (mCol2.getY() * pnt.getZ())) + mCol3.getY()),
                   ((((mCol0.getZ() * pnt.getX()) + (mCol1.getZ() * pnt.getY())) + (mCol2.getZ() * pnt.getZ())) + mCol3.getZ()),
                   ((((mCol0.getW() * pnt.getX()) + (mCol1.getW() * pnt.getY())) + (mCol2.getW() * pnt.getZ())) + mCol3.getW()));
}

inline const Matrix4 Matrix4::operator * (const Matrix4 & mat) const
{
    return Matrix4((*this * mat.mCol0),
                   (*this * mat.mCol1),
                   (*this * mat.mCol2),
                   (*this * mat.mCol3));
}

inline Matrix4 & Matrix4::operator *= (const Matrix4 & mat)
{
    *this = *this * mat;
    return *this;
}

inline const Matrix4 Matrix4::operator * (const Transform3 & tfrm) const
{
    return Matrix4((*this * tfrm.getCol0()),
                   (*this * tfrm.getCol1()),
                   (*this * tfrm.getCol2()),
                   (*this * Point3(tfrm.getCol3())));
}

inline Matrix4 & Matrix4::operator *= (const Transform3 & tfrm)
{
    *this = *this * tfrm;
    return *this;
}

inline const Matrix4 mulPerElem(const Matrix4 & mat0, const Matrix4 & mat1)
{
    return Matrix4(mulPerElem(mat0.getCol0(), mat1.getCol0()),
                   mulPerElem(mat0.getCol1(), mat1.getCol1()),
                   mulPerElem(mat0.getCol2(), mat1.getCol2()),
                   mulPerElem(mat0.getCol3(), mat1.getCol3()));
}

inline const Matrix4 Matrix4::identity()
{
    return Matrix4(Vector4::xAxis(),
                   Vector4::yAxis(),
                   Vector4::zAxis(),
                   Vector4::wAxis());
}

inline Matrix4 & Matrix4::setUpper3x3(const Matrix3 & mat3)
{
    mCol0.setXYZ(mat3.getCol0());
    mCol1.setXYZ(mat3.getCol1());
    mCol2.setXYZ(mat3.getCol2());
    return *this;
}

inline const Matrix3 Matrix4::getUpper3x3() const
{
    return Matrix3(
    mCol0.getXYZ(),
    mCol1.getXYZ(),
    mCol2.getXYZ());
}

inline Matrix4 & Matrix4::setTranslation(const Vector3 & translateVec)
{
    mCol3.setXYZ(translateVec);
    return *this;
}

inline const Vector3 Matrix4::getTranslation() const
{
    return mCol3.getXYZ();
}

inline const Matrix4 Matrix4::rotationX(float radians)
{
    float s, c;
    s = sinf(radians);
    c = cosf(radians);
    return Matrix4(Vector4::xAxis(),
                   Vector4(0.0f,  c, s, 0.0f),
                   Vector4(0.0f, -s, c, 0.0f),
                   Vector4::wAxis());
}

inline const Matrix4 Matrix4::rotationY(float radians)
{
    float s, c;
    s = sinf(radians);
    c = cosf(radians);
    return Matrix4(Vector4(c, 0.0f, -s, 0.0f),
                   Vector4::yAxis(),
                   Vector4(s, 0.0f, c, 0.0f),
                   Vector4::wAxis());
}

inline const Matrix4 Matrix4::rotationZ(float radians)
{
    float s, c;
    s = sinf(radians);
    c = cosf(radians);
    return Matrix4(Vector4( c, s, 0.0f, 0.0f),
                   Vector4(-s, c, 0.0f, 0.0f),
                   Vector4::zAxis(),
                   Vector4::wAxis());
}

inline const Matrix4 Matrix4::rotationZYX(const Vector3 & radiansXYZ)
{
    float sX, cX, sY, cY, sZ, cZ, tmp0, tmp1;
    sX = sinf(radiansXYZ.getX());
    cX = cosf(radiansXYZ.getX());
    sY = sinf(radiansXYZ.getY());
    cY = cosf(radiansXYZ.getY());
    sZ = sinf(radiansXYZ.getZ());
    cZ = cosf(radiansXYZ.getZ());
    tmp0 = (cZ * sY);
    tmp1 = (sZ * sY);
    return Matrix4(Vector4((cZ * cY), (sZ * cY), -sY, 0.0f),
                   Vector4(((tmp0 * sX) - (sZ * cX)), ((tmp1 * sX) + (cZ * cX)), (cY * sX), 0.0f),
                   Vector4(((tmp0 * cX) + (sZ * sX)), ((tmp1 * cX) - (cZ * sX)), (cY * cX), 0.0f),
                   Vector4::wAxis());
}

inline const Matrix4 Matrix4::rotation(float radians, const Vector3 & unitVec)
{
    float x, y, z, s, c, oneMinusC, xy, yz, zx;
    s = sinf(radians);
    c = cosf(radians);
    x = unitVec.getX();
    y = unitVec.getY();
    z = unitVec.getZ();
    xy = (x * y);
    yz = (y * z);
    zx = (z * x);
    oneMinusC = (1.0f - c);
    return Matrix4(Vector4((((x * x) * oneMinusC) + c), ((xy * oneMinusC) + (z * s)), ((zx * oneMinusC) - (y * s)), 0.0f),
                   Vector4(((xy * oneMinusC) - (z * s)), (((y * y) * oneMinusC) + c), ((yz * oneMinusC) + (x * s)), 0.0f),
                   Vector4(((zx * oneMinusC) + (y * s)), ((yz * oneMinusC) - (x * s)), (((z * z) * oneMinusC) + c), 0.0f),
                   Vector4::wAxis());
}

inline const Matrix4 Matrix4::rotation(const Quat & unitQuat)
{
    return Matrix4(Transform3::rotation(unitQuat));
}

inline const Matrix4 Matrix4::scale(const Vector3 & scaleVec)
{
    return Matrix4(Vector4(scaleVec.getX(), 0.0f, 0.0f, 0.0f),
                   Vector4(0.0f, scaleVec.getY(), 0.0f, 0.0f),
                   Vector4(0.0f, 0.0f, scaleVec.getZ(), 0.0f),
                   Vector4::wAxis());
}

inline const Matrix4 appendScale(const Matrix4 & mat, const Vector3 & scaleVec)
{
    return Matrix4((mat.getCol0() * scaleVec.getX()),
                   (mat.getCol1() * scaleVec.getY()),
                   (mat.getCol2() * scaleVec.getZ()),
                   mat.getCol3());
}

inline const Matrix4 prependScale(const Vector3 & scaleVec, const Matrix4 & mat)
{
    Vector4 scale4;
    scale4 = Vector4(scaleVec, 1.0f);
    return Matrix4(mulPerElem(mat.getCol0(), scale4),
                   mulPerElem(mat.getCol1(), scale4),
                   mulPerElem(mat.getCol2(), scale4),
                   mulPerElem(mat.getCol3(), scale4));
}

inline const Matrix4 Matrix4::translation(const Vector3 & translateVec)
{
    return Matrix4(Vector4::xAxis(),
                   Vector4::yAxis(),
                   Vector4::zAxis(),
                   Vector4(translateVec, 1.0f));
}

inline const Matrix4 Matrix4::lookAt(const Point3 & eyePos, const Point3 & lookAtPos, const Vector3 & upVec)
{
    Matrix4 m4EyeFrame;
    Vector3 v3X, v3Y, v3Z;
    v3Y = normalize(upVec);
    v3Z = normalize((eyePos - lookAtPos));
    v3X = normalize(cross(v3Y, v3Z));
    v3Y = cross(v3Z, v3X);
    m4EyeFrame = Matrix4(Vector4(v3X), Vector4(v3Y), Vector4(v3Z), Vector4(eyePos));
    return orthoInverse(m4EyeFrame);
}

inline const Matrix4 Matrix4::perspective(float fovyRadians, float aspect, float zNear, float zFar)
{
    static const float VECTORMATH_PI_OVER_2 = 1.570796327f;

    float f, rangeInv;
    f = tanf(VECTORMATH_PI_OVER_2 - (0.5f * fovyRadians));
    rangeInv = (1.0f / (zNear - zFar));
    return Matrix4(Vector4((f / aspect), 0.0f, 0.0f, 0.0f),
                   Vector4(0.0f, f, 0.0f, 0.0f),
                   Vector4(0.0f, 0.0f, ((zNear + zFar) * rangeInv), -1.0f),
                   Vector4(0.0f, 0.0f, (((zNear * zFar) * rangeInv) * 2.0f), 0.0f));
}

inline const Matrix4 Matrix4::frustum(float left, float right, float bottom, float top, float zNear, float zFar)
{
    float sum_rl, sum_tb, sum_nf, inv_rl, inv_tb, inv_nf, n2;
    sum_rl = (right + left);
    sum_tb = (top + bottom);
    sum_nf = (zNear + zFar);
    inv_rl = (1.0f / (right - left));
    inv_tb = (1.0f / (top - bottom));
    inv_nf = (1.0f / (zNear - zFar));
    n2 = (zNear + zNear);
    return Matrix4(Vector4((n2 * inv_rl), 0.0f, 0.0f, 0.0f),
                   Vector4(0.0f, (n2 * inv_tb), 0.0f, 0.0f),
                   Vector4((sum_rl * inv_rl), (sum_tb * inv_tb), (sum_nf * inv_nf), -1.0f),
                   Vector4(0.0f, 0.0f, ((n2 * inv_nf) * zFar), 0.0f));
}

inline const Matrix4 Matrix4::orthographic(float left, float right, float bottom, float top, float zNear, float zFar)
{
    float sum_rl, sum_tb, sum_nf, inv_rl, inv_tb, inv_nf;
    sum_rl = (right + left);
    sum_tb = (top + bottom);
    sum_nf = (zNear + zFar);
    inv_rl = (1.0f / (right - left));
    inv_tb = (1.0f / (top - bottom));
    inv_nf = (1.0f / (zNear - zFar));
    return Matrix4(Vector4((inv_rl + inv_rl), 0.0f, 0.0f, 0.0f),
                   Vector4(0.0f, (inv_tb + inv_tb), 0.0f, 0.0f),
                   Vector4(0.0f, 0.0f, (inv_nf + inv_nf), 0.0f),
                   Vector4((-sum_rl * inv_rl), (-sum_tb * inv_tb), (sum_nf * inv_nf), 1.0f));
}

inline const Matrix4 select(const Matrix4 & mat0, const Matrix4 & mat1, bool select1)
{
    return Matrix4(select(mat0.getCol0(), mat1.getCol0(), select1),
                   select(mat0.getCol1(), mat1.getCol1(), select1),
                   select(mat0.getCol2(), mat1.getCol2(), select1),
                   select(mat0.getCol3(), mat1.getCol3(), select1));
}

#ifdef VECTORMATH_DEBUG

inline void print(const Matrix4 & mat)
{
    print(mat.getRow(0));
    print(mat.getRow(1));
    print(mat.getRow(2));
    print(mat.getRow(3));
}

inline void print(const Matrix4 & mat, const char * name)
{
    printf("%s:\n", name);
    print(mat);
}

#endif // VECTORMATH_DEBUG

// ========================================================
// Transform3
// ========================================================

inline Transform3::Transform3(const Transform3 & tfrm)
{
    mCol0 = tfrm.mCol0;
    mCol1 = tfrm.mCol1;
    mCol2 = tfrm.mCol2;
    mCol3 = tfrm.mCol3;
}

inline Transform3::Transform3(float scalar)
{
    mCol0 = Vector3(scalar);
    mCol1 = Vector3(scalar);
    mCol2 = Vector3(scalar);
    mCol3 = Vector3(scalar);
}

inline Transform3::Transform3(const Vector3 & _col0, const Vector3 & _col1, const Vector3 & _col2, const Vector3 & _col3)
{
    mCol0 = _col0;
    mCol1 = _col1;
    mCol2 = _col2;
    mCol3 = _col3;
}

inline Transform3::Transform3(const Matrix3 & tfrm, const Vector3 & translateVec)
{
    this->setUpper3x3(tfrm);
    this->setTranslation(translateVec);
}

inline Transform3::Transform3(const Quat & unitQuat, const Vector3 & translateVec)
{
    this->setUpper3x3(Matrix3(unitQuat));
    this->setTranslation(translateVec);
}

inline Transform3 & Transform3::setCol0(const Vector3 & _col0)
{
    mCol0 = _col0;
    return *this;
}

inline Transform3 & Transform3::setCol1(const Vector3 & _col1)
{
    mCol1 = _col1;
    return *this;
}

inline Transform3 & Transform3::setCol2(const Vector3 & _col2)
{
    mCol2 = _col2;
    return *this;
}

inline Transform3 & Transform3::setCol3(const Vector3 & _col3)
{
    mCol3 = _col3;
    return *this;
}

inline Transform3 & Transform3::setCol(int col, const Vector3 & vec)
{
    *(&mCol0 + col) = vec;
    return *this;
}

inline Transform3 & Transform3::setRow(int row, const Vector4 & vec)
{
    mCol0.setElem(row, vec.getElem(0));
    mCol1.setElem(row, vec.getElem(1));
    mCol2.setElem(row, vec.getElem(2));
    mCol3.setElem(row, vec.getElem(3));
    return *this;
}

inline Transform3 & Transform3::setElem(int col, int row, float val)
{
    Vector3 tmpV3_0;
    tmpV3_0 = this->getCol(col);
    tmpV3_0.setElem(row, val);
    this->setCol(col, tmpV3_0);
    return *this;
}

inline float Transform3::getElem(int col, int row) const
{
    return this->getCol(col).getElem(row);
}

inline const Vector3 Transform3::getCol0() const
{
    return mCol0;
}

inline const Vector3 Transform3::getCol1() const
{
    return mCol1;
}

inline const Vector3 Transform3::getCol2() const
{
    return mCol2;
}

inline const Vector3 Transform3::getCol3() const
{
    return mCol3;
}

inline const Vector3 Transform3::getCol(int col) const
{
    return *(&mCol0 + col);
}

inline const Vector4 Transform3::getRow(int row) const
{
    return Vector4(mCol0.getElem(row), mCol1.getElem(row), mCol2.getElem(row), mCol3.getElem(row));
}

inline Vector3 & Transform3::operator[](int col)
{
    return *(&mCol0 + col);
}

inline const Vector3 Transform3::operator[](int col) const
{
    return *(&mCol0 + col);
}

inline Transform3 & Transform3::operator = (const Transform3 & tfrm)
{
    mCol0 = tfrm.mCol0;
    mCol1 = tfrm.mCol1;
    mCol2 = tfrm.mCol2;
    mCol3 = tfrm.mCol3;
    return *this;
}

inline const Transform3 inverse(const Transform3 & tfrm)
{
    Vector3 tmp0, tmp1, tmp2, inv0, inv1, inv2;
    float detinv;
    tmp0 = cross(tfrm.getCol1(), tfrm.getCol2());
    tmp1 = cross(tfrm.getCol2(), tfrm.getCol0());
    tmp2 = cross(tfrm.getCol0(), tfrm.getCol1());
    detinv = (1.0f / dot(tfrm.getCol2(), tmp2));
    inv0 = Vector3((tmp0.getX() * detinv), (tmp1.getX() * detinv), (tmp2.getX() * detinv));
    inv1 = Vector3((tmp0.getY() * detinv), (tmp1.getY() * detinv), (tmp2.getY() * detinv));
    inv2 = Vector3((tmp0.getZ() * detinv), (tmp1.getZ() * detinv), (tmp2.getZ() * detinv));
    return Transform3(inv0, inv1, inv2,
                      Vector3((-((inv0 * tfrm.getCol3().getX()) + ((inv1 * tfrm.getCol3().getY()) + (inv2 * tfrm.getCol3().getZ()))))));
}

inline const Transform3 orthoInverse(const Transform3 & tfrm)
{
    Vector3 inv0, inv1, inv2;
    inv0 = Vector3(tfrm.getCol0().getX(), tfrm.getCol1().getX(), tfrm.getCol2().getX());
    inv1 = Vector3(tfrm.getCol0().getY(), tfrm.getCol1().getY(), tfrm.getCol2().getY());
    inv2 = Vector3(tfrm.getCol0().getZ(), tfrm.getCol1().getZ(), tfrm.getCol2().getZ());
    return Transform3(inv0, inv1, inv2,
                      Vector3((-((inv0 * tfrm.getCol3().getX()) + ((inv1 * tfrm.getCol3().getY()) + (inv2 * tfrm.getCol3().getZ()))))));
}

inline const Transform3 absPerElem(const Transform3 & tfrm)
{
    return Transform3(absPerElem(tfrm.getCol0()),
                      absPerElem(tfrm.getCol1()),
                      absPerElem(tfrm.getCol2()),
                      absPerElem(tfrm.getCol3()));
}

inline const Vector3 Transform3::operator * (const Vector3 & vec) const
{
    return Vector3((((mCol0.getX() * vec.getX()) + (mCol1.getX() * vec.getY())) + (mCol2.getX() * vec.getZ())),
                   (((mCol0.getY() * vec.getX()) + (mCol1.getY() * vec.getY())) + (mCol2.getY() * vec.getZ())),
                   (((mCol0.getZ() * vec.getX()) + (mCol1.getZ() * vec.getY())) + (mCol2.getZ() * vec.getZ())));
}

inline const Point3 Transform3::operator * (const Point3 & pnt) const
{
    return Point3(((((mCol0.getX() * pnt.getX()) + (mCol1.getX() * pnt.getY())) + (mCol2.getX() * pnt.getZ())) + mCol3.getX()),
                  ((((mCol0.getY() * pnt.getX()) + (mCol1.getY() * pnt.getY())) + (mCol2.getY() * pnt.getZ())) + mCol3.getY()),
                  ((((mCol0.getZ() * pnt.getX()) + (mCol1.getZ() * pnt.getY())) + (mCol2.getZ() * pnt.getZ())) + mCol3.getZ()));
}

inline const Transform3 Transform3::operator * (const Transform3 & tfrm) const
{
    return Transform3((*this * tfrm.mCol0),
                      (*this * tfrm.mCol1),
                      (*this * tfrm.mCol2),
                      Vector3((*this * Point3(tfrm.mCol3))));
}

inline Transform3 & Transform3::operator *= (const Transform3 & tfrm)
{
    *this = *this * tfrm;
    return *this;
}

inline const Transform3 mulPerElem(const Transform3 & tfrm0, const Transform3 & tfrm1)
{
    return Transform3(mulPerElem(tfrm0.getCol0(), tfrm1.getCol0()),
                      mulPerElem(tfrm0.getCol1(), tfrm1.getCol1()),
                      mulPerElem(tfrm0.getCol2(), tfrm1.getCol2()),
                      mulPerElem(tfrm0.getCol3(), tfrm1.getCol3()));
}

inline const Transform3 Transform3::identity()
{
    return Transform3(Vector3::xAxis(),
                      Vector3::yAxis(),
                      Vector3::zAxis(),
                      Vector3(0.0f));
}

inline Transform3 & Transform3::setUpper3x3(const Matrix3 & tfrm)
{
    mCol0 = tfrm.getCol0();
    mCol1 = tfrm.getCol1();
    mCol2 = tfrm.getCol2();
    return *this;
}

inline const Matrix3 Transform3::getUpper3x3() const
{
    return Matrix3(mCol0, mCol1, mCol2);
}

inline Transform3 & Transform3::setTranslation(const Vector3 & translateVec)
{
    mCol3 = translateVec;
    return *this;
}

inline const Vector3 Transform3::getTranslation() const
{
    return mCol3;
}

inline const Transform3 Transform3::rotationX(float radians)
{
    float s, c;
    s = sinf(radians);
    c = cosf(radians);
    return Transform3(Vector3::xAxis(),
                      Vector3(0.0f,  c, s),
                      Vector3(0.0f, -s, c),
                      Vector3(0.0f));
}

inline const Transform3 Transform3::rotationY(float radians)
{
    float s, c;
    s = sinf(radians);
    c = cosf(radians);
    return Transform3(Vector3(c, 0.0f, -s),
                      Vector3::yAxis(),
                      Vector3(s, 0.0f, c),
                      Vector3(0.0f));
}

inline const Transform3 Transform3::rotationZ(float radians)
{
    float s, c;
    s = sinf(radians);
    c = cosf(radians);
    return Transform3(Vector3( c, s, 0.0f),
                      Vector3(-s, c, 0.0f),
                      Vector3::zAxis(),
                      Vector3(0.0f));
}

inline const Transform3 Transform3::rotationZYX(const Vector3 & radiansXYZ)
{
    float sX, cX, sY, cY, sZ, cZ, tmp0, tmp1;
    sX = sinf(radiansXYZ.getX());
    cX = cosf(radiansXYZ.getX());
    sY = sinf(radiansXYZ.getY());
    cY = cosf(radiansXYZ.getY());
    sZ = sinf(radiansXYZ.getZ());
    cZ = cosf(radiansXYZ.getZ());
    tmp0 = (cZ * sY);
    tmp1 = (sZ * sY);
    return Transform3(Vector3((cZ * cY), (sZ * cY), -sY),
                      Vector3(((tmp0 * sX) - (sZ * cX)), ((tmp1 * sX) + (cZ * cX)), (cY * sX)),
                      Vector3(((tmp0 * cX) + (sZ * sX)), ((tmp1 * cX) - (cZ * sX)), (cY * cX)),
                      Vector3(0.0f));
}

inline const Transform3 Transform3::rotation(float radians, const Vector3 & unitVec)
{
    return Transform3(Matrix3::rotation(radians, unitVec), Vector3(0.0f));
}

inline const Transform3 Transform3::rotation(const Quat & unitQuat)
{
    return Transform3(Matrix3(unitQuat), Vector3(0.0f));
}

inline const Transform3 Transform3::scale(const Vector3 & scaleVec)
{
    return Transform3(Vector3(scaleVec.getX(), 0.0f, 0.0f),
                      Vector3(0.0f, scaleVec.getY(), 0.0f),
                      Vector3(0.0f, 0.0f, scaleVec.getZ()),
                      Vector3(0.0f));
}

inline const Transform3 appendScale(const Transform3 & tfrm, const Vector3 & scaleVec)
{
    return Transform3((tfrm.getCol0() * scaleVec.getX()),
                      (tfrm.getCol1() * scaleVec.getY()),
                      (tfrm.getCol2() * scaleVec.getZ()),
                      tfrm.getCol3());
}

inline const Transform3 prependScale(const Vector3 & scaleVec, const Transform3 & tfrm)
{
    return Transform3(mulPerElem(tfrm.getCol0(), scaleVec),
                      mulPerElem(tfrm.getCol1(), scaleVec),
                      mulPerElem(tfrm.getCol2(), scaleVec),
                      mulPerElem(tfrm.getCol3(), scaleVec));
}

inline const Transform3 Transform3::translation(const Vector3 & translateVec)
{
    return Transform3(Vector3::xAxis(),
                      Vector3::yAxis(),
                      Vector3::zAxis(),
                      translateVec);
}

inline const Transform3 select(const Transform3 & tfrm0, const Transform3 & tfrm1, bool select1)
{
    return Transform3(select(tfrm0.getCol0(), tfrm1.getCol0(), select1),
                      select(tfrm0.getCol1(), tfrm1.getCol1(), select1),
                      select(tfrm0.getCol2(), tfrm1.getCol2(), select1),
                      select(tfrm0.getCol3(), tfrm1.getCol3(), select1));
}

#ifdef VECTORMATH_DEBUG

inline void print(const Transform3 & tfrm)
{
    print(tfrm.getRow(0));
    print(tfrm.getRow(1));
    print(tfrm.getRow(2));
}

inline void print(const Transform3 & tfrm, const char * name)
{
    printf("%s:\n", name);
    print(tfrm);
}

#endif // VECTORMATH_DEBUG

// ========================================================
// Quat
// ========================================================

inline Quat::Quat(const Matrix3 & tfrm)
{
    float trace, radicand, scale, xx, yx, zx, xy, yy, zy, xz, yz, zz, tmpx, tmpy, tmpz, tmpw, qx, qy, qz, qw;
    int negTrace, ZgtX, ZgtY, YgtX;
    int largestXorY, largestYorZ, largestZorX;

    xx = tfrm.getCol0().getX();
    yx = tfrm.getCol0().getY();
    zx = tfrm.getCol0().getZ();
    xy = tfrm.getCol1().getX();
    yy = tfrm.getCol1().getY();
    zy = tfrm.getCol1().getZ();
    xz = tfrm.getCol2().getX();
    yz = tfrm.getCol2().getY();
    zz = tfrm.getCol2().getZ();

    trace = ((xx + yy) + zz);

    negTrace = (trace < 0.0f);
    ZgtX = zz > xx;
    ZgtY = zz > yy;
    YgtX = yy > xx;
    largestXorY = (!ZgtX || !ZgtY) && negTrace;
    largestYorZ = ( YgtX ||  ZgtX) && negTrace;
    largestZorX = ( ZgtY || !YgtX) && negTrace;

    if (largestXorY)
    {
        zz = -zz;
        xy = -xy;
    }
    if (largestYorZ)
    {
        xx = -xx;
        yz = -yz;
    }
    if (largestZorX)
    {
        yy = -yy;
        zx = -zx;
    }

    radicand = (((xx + yy) + zz) + 1.0f);
    scale = (0.5f * (1.0f / sqrtf(radicand)));

    tmpx = ((zy - yz) * scale);
    tmpy = ((xz - zx) * scale);
    tmpz = ((yx - xy) * scale);
    tmpw = (radicand * scale);
    qx = tmpx;
    qy = tmpy;
    qz = tmpz;
    qw = tmpw;

    if (largestXorY)
    {
        qx = tmpw;
        qy = tmpz;
        qz = tmpy;
        qw = tmpx;
    }
    if (largestYorZ)
    {
        tmpx = qx;
        tmpz = qz;
        qx = qy;
        qy = tmpx;
        qz = qw;
        qw = tmpz;
    }

    mX = qx;
    mY = qy;
    mZ = qz;
    mW = qw;
}

// ========================================================
// Misc free functions
// ========================================================

inline const Matrix3 outer(const Vector3 & tfrm0, const Vector3 & tfrm1)
{
    return Matrix3((tfrm0 * tfrm1.getX()),
                   (tfrm0 * tfrm1.getY()),
                   (tfrm0 * tfrm1.getZ()));
}

inline const Matrix4 outer(const Vector4 & tfrm0, const Vector4 & tfrm1)
{
    return Matrix4((tfrm0 * tfrm1.getX()),
                   (tfrm0 * tfrm1.getY()),
                   (tfrm0 * tfrm1.getZ()),
                   (tfrm0 * tfrm1.getW()));
}

inline const Vector3 rowMul(const Vector3 & vec, const Matrix3 & mat)
{
    return Vector3((((vec.getX() * mat.getCol0().getX()) + (vec.getY() * mat.getCol0().getY())) + (vec.getZ() * mat.getCol0().getZ())),
                   (((vec.getX() * mat.getCol1().getX()) + (vec.getY() * mat.getCol1().getY())) + (vec.getZ() * mat.getCol1().getZ())),
                   (((vec.getX() * mat.getCol2().getX()) + (vec.getY() * mat.getCol2().getY())) + (vec.getZ() * mat.getCol2().getZ())));
}

inline const Matrix3 crossMatrix(const Vector3 & vec)
{
    return Matrix3(Vector3(0.0f, vec.getZ(), -vec.getY()),
                   Vector3(-vec.getZ(), 0.0f, vec.getX()),
                   Vector3(vec.getY(), -vec.getX(), 0.0f));
}

inline const Matrix3 crossMatrixMul(const Vector3 & vec, const Matrix3 & mat)
{
    return Matrix3(cross(vec, mat.getCol0()), cross(vec, mat.getCol1()), cross(vec, mat.getCol2()));
}

} // namespace Scalar
} // namespace Vectormath

#endif // VECTORMATH_SCALAR_MATRIX_HPP

#endif // VECTORMATH_SCALAR_VECTORMATH_HPP

    using namespace Vectormath::Scalar;
    #define VECTORMATH_MODE_SCALAR 1
    #define VECTORMATH_MODE_SSE    0
#endif // Vectormath mode selection

// ================================================================================================
// -*- C++ -*-
// File: vectormath/vec2d.hpp
// Author: Guilherme R. Lampert
// Created on: 30/12/16
// Brief: 2D vector and point extensions to the original Vectormath library.
// ================================================================================================

#ifndef VECTORMATH_VEC2D_HPP
#define VECTORMATH_VEC2D_HPP

namespace Vectormath
{

class Vector2;
class Point2;

// ========================================================
// A 2-D unpadded vector (sizeof = 8 bytes)
// ========================================================

class Vector2
{
    float mX;
    float mY;

public:

    // Default constructor; does no initialization
    inline Vector2() { }

    // Construct a 2-D vector from x and y elements
    inline Vector2(float x, float y);

    // Copy elements from a 2-D point into a 2-D vector
    explicit inline Vector2(const Point2 & pnt);

    // Set all elements of a 2-D vector to the same scalar value
    explicit inline Vector2(float scalar);

    // Set the x element of a 2-D vector
    inline Vector2 & setX(float x);

    // Set the y element of a 2-D vector
    inline Vector2 & setY(float y);

    // Get the x element of a 2-D vector
    inline float getX() const;

    // Get the y element of a 2-D vector
    inline float getY() const;

    // Set an x or y element of a 2-D vector by index
    inline Vector2 & setElem(int idx, float value);

    // Get an x or y element of a 2-D vector by index
    inline float getElem(int idx) const;

    // Subscripting operator to set or get an element
    inline float & operator[](int idx);

    // Subscripting operator to get an element
    inline float operator[](int idx) const;

    // Add two 2-D vectors
    inline const Vector2 operator + (const Vector2 & vec) const;

    // Subtract a 2-D vector from another 2-D vector
    inline const Vector2 operator - (const Vector2 & vec) const;

    // Add a 2-D vector to a 2-D point
    inline const Point2 operator + (const Point2 & pnt) const;

    // Multiply a 2-D vector by a scalar
    inline const Vector2 operator * (float scalar) const;

    // Divide a 2-D vector by a scalar
    inline const Vector2 operator / (float scalar) const;

    // Perform compound assignment and addition with a 2-D vector
    inline Vector2 & operator += (const Vector2 & vec);

    // Perform compound assignment and subtraction by a 2-D vector
    inline Vector2 & operator -= (const Vector2 & vec);

    // Perform compound assignment and multiplication by a scalar
    inline Vector2 & operator *= (float scalar);

    // Perform compound assignment and division by a scalar
    inline Vector2 & operator /= (float scalar);

    // Negate all elements of a 2-D vector
    inline const Vector2 operator - () const;

    // Construct x axis
    static inline const Vector2 xAxis();

    // Construct y axis
    static inline const Vector2 yAxis();
};

// Multiply a 2-D vector by a scalar
//
inline const Vector2 operator * (float scalar, const Vector2 & vec);

// Compute the absolute value of a 2-D vector per element
//
inline const Vector2 absPerElem(const Vector2 & vec);

// Maximum of two 2-D vectors per element
//
inline const Vector2 maxPerElem(const Vector2 & vec0, const Vector2 & vec1);

// Minimum of two 2-D vectors per element
//
inline const Vector2 minPerElem(const Vector2 & vec0, const Vector2 & vec1);

// Maximum element of a 2-D vector
//
inline float maxElem(const Vector2 & vec);

// Minimum element of a 2-D vector
//
inline float minElem(const Vector2 & vec);

// Compute the dot product of two 2-D vectors
//
inline float dot(const Vector2 & vec0, const Vector2 & vec1);

// Compute the square of the length of a 2-D vector
//
inline float lengthSqr(const Vector2 & vec);

// Compute the length of a 2-D vector
//
inline float length(const Vector2 & vec);

// Normalize a 2-D vector
// NOTE:
// The result is unpredictable when all elements of vec are at or near zero.
//
inline const Vector2 normalize(const Vector2 & vec);

// Linear interpolation between two 2-D vectors
// NOTE:
// Does not clamp t between 0 and 1.
//
inline const Vector2 lerp(float t, const Vector2 & vec0, const Vector2 & vec1);

#ifdef VECTORMATH_DEBUG

// Print a 2-D vector
// NOTE:
// Function is only defined when VECTORMATH_DEBUG is defined.
//
inline void print(const Vector2 & vec);

// Print a 2-D vector and an associated string identifier
// NOTE:
// Function is only defined when VECTORMATH_DEBUG is defined.
//
inline void print(const Vector2 & vec, const char * name);

#endif // VECTORMATH_DEBUG

// ========================================================
// A 2-D unpadded point (sizeof = 8 bytes)
// ========================================================

class Point2
{
    float mX;
    float mY;

public:

    // Default constructor; does no initialization
    inline Point2() { }

    // Construct a 2-D point from x and y elements
    inline Point2(float x, float y);

    // Copy elements from a 2-D vector into a 2-D point
    explicit inline Point2(const Vector2 & vec);

    // Set all elements of a 2-D point to the same scalar value
    explicit inline Point2(float scalar);

    // Set the x element of a 2-D point
    inline Point2 & setX(float x);

    // Set the y element of a 2-D point
    inline Point2 & setY(float y);

    // Get the x element of a 2-D point
    inline float getX() const;

    // Get the y element of a 2-D point
    inline float getY() const;

    // Set an x or y element of a 2-D point by index
    inline Point2 & setElem(int idx, float value);

    // Get an x or y element of a 2-D point by index
    inline float getElem(int idx) const;

    // Subscripting operator to set or get an element
    inline float & operator[](int idx);

    // Subscripting operator to get an element
    inline float operator[](int idx) const;

    // Subtract a 2-D point from another 2-D point
    inline const Vector2 operator - (const Point2 & pnt) const;

    // Add a 2-D point to a 2-D vector
    inline const Point2 operator + (const Vector2 & vec) const;

    // Subtract a 2-D vector from a 2-D point
    inline const Point2 operator - (const Vector2 & vec) const;

    // Perform compound assignment and addition with a 2-D vector
    inline Point2 & operator += (const Vector2 & vec);

    // Perform compound assignment and subtraction by a 2-D vector
    inline Point2 & operator -= (const Vector2 & vec);
};

// Compute the absolute value of a 2-D point per element
//
inline const Point2 absPerElem(const Point2 & pnt);

// Maximum of two 2-D points per element
//
inline const Point2 maxPerElem(const Point2 & pnt0, const Point2 & pnt1);

// Minimum of two 2-D points per element
//
inline const Point2 minPerElem(const Point2 & pnt0, const Point2 & pnt1);

// Maximum element of a 2-D point
//
inline float maxElem(const Point2 & pnt);

// Minimum element of a 2-D point
//
inline float minElem(const Point2 & pnt);

// Compute the square of the distance of a 2-D point from the coordinate-system origin
//
inline float distSqrFromOrigin(const Point2 & pnt);

// Compute the distance of a 2-D point from the coordinate-system origin
//
inline float distFromOrigin(const Point2 & pnt);

// Compute the square of the distance between two 2-D points
//
inline float distSqr(const Point2 & pnt0, const Point2 & pnt1);

// Compute the distance between two 2-D points
//
inline float distp(const Point2 & pnt0, const Point2 & pnt1);

// Linear interpolation between two 2-D points
// NOTE:
// Does not clamp t between 0 and 1.
//
inline const Point2 lerp(float t, const Point2 & pnt0, const Point2 & pnt1);

#ifdef VECTORMATH_DEBUG

// Print a 2-D point
// NOTE:
// Function is only defined when VECTORMATH_DEBUG is defined.
//
inline void print(const Point2 & pnt);

// Print a 2-D point and an associated string identifier
// NOTE:
// Function is only defined when VECTORMATH_DEBUG is defined.
//
inline void print(const Point2 & pnt, const char * name);

#endif // VECTORMATH_DEBUG

// ================================================================================================
// Vector2 implementation
// ================================================================================================

inline Vector2::Vector2(float _x, float _y)
    : mX(_x), mY(_y)
{
}

inline Vector2::Vector2(const Point2 & pnt)
    : mX(pnt.getX()), mY(pnt.getY())
{
}

inline Vector2::Vector2(float scalar)
    : mX(scalar), mY(scalar)
{
}

inline Vector2 & Vector2::setX(float _x)
{
    mX = _x;
    return *this;
}

inline Vector2 & Vector2::setY(float _y)
{
    mY = _y;
    return *this;
}

inline float Vector2::getX() const
{
    return mX;
}

inline float Vector2::getY() const
{
    return mY;
}

inline Vector2 & Vector2::setElem(int idx, float value)
{
    *(&mX + idx) = value;
    return *this;
}

inline float Vector2::getElem(int idx) const
{
    return *(&mX + idx);
}

inline float & Vector2::operator[](int idx)
{
    return *(&mX + idx);
}

inline float Vector2::operator[](int idx) const
{
    return *(&mX + idx);
}

inline const Vector2 Vector2::operator + (const Vector2 & vec) const
{
    return Vector2((mX + vec.mX), (mY + vec.mY));
}

inline const Vector2 Vector2::operator - (const Vector2 & vec) const
{
    return Vector2((mX - vec.mX), (mY - vec.mY));
}

inline const Point2 Vector2::operator + (const Point2 & pnt) const
{
    return Point2((mX + pnt.getX()), (mY + pnt.getY()));
}

inline const Vector2 Vector2::operator * (float scalar) const
{
    return Vector2((mX * scalar), (mY * scalar));
}

inline const Vector2 Vector2::operator / (float scalar) const
{
    return Vector2((mX / scalar), (mY / scalar));
}

inline Vector2 & Vector2::operator += (const Vector2 & vec)
{
    mX += vec.mX;
    mY += vec.mY;
    return *this;
}

inline Vector2 & Vector2::operator -= (const Vector2 & vec)
{
    mX -= vec.mX;
    mY -= vec.mY;
    return *this;
}

inline Vector2 & Vector2::operator *= (float scalar)
{
    mX *= scalar;
    mY *= scalar;
    return *this;
}

inline Vector2 & Vector2::operator /= (float scalar)
{
    mX /= scalar;
    mY /= scalar;
    return *this;
}

inline const Vector2 Vector2::operator - () const
{
    return Vector2(-mX, -mY);
}

inline const Vector2 Vector2::xAxis()
{
    return Vector2(1.0f, 0.0f);
}

inline const Vector2 Vector2::yAxis()
{
    return Vector2(0.0f, 1.0f);
}

inline const Vector2 operator * (float scalar, const Vector2 & vec)
{
    return vec * scalar;
}

inline const Vector2 absPerElem(const Vector2 & vec)
{
    return Vector2(fabsf(vec.getX()), fabsf(vec.getY()));
}

inline const Vector2 maxPerElem(const Vector2 & vec0, const Vector2 & vec1)
{
    return Vector2((vec0.getX() > vec1.getX()) ? vec0.getX() : vec1.getX(),
                   (vec0.getY() > vec1.getY()) ? vec0.getY() : vec1.getY());
}

inline const Vector2 minPerElem(const Vector2 & vec0, const Vector2 & vec1)
{
    return Vector2((vec0.getX() < vec1.getX()) ? vec0.getX() : vec1.getX(),
                   (vec0.getY() < vec1.getY()) ? vec0.getY() : vec1.getY());
}

inline float maxElem(const Vector2 & vec)
{
    return (vec.getX() > vec.getY()) ? vec.getX() : vec.getY();
}

inline float minElem(const Vector2 & vec)
{
    return (vec.getX() < vec.getY()) ? vec.getX() : vec.getY();
}

inline float dot(const Vector2 & vec0, const Vector2 & vec1)
{
    float result;
    result = (vec0.getX() * vec1.getX());
    result = (result + (vec0.getY() * vec1.getY()));
    return result;
}

inline float lengthSqr(const Vector2 & vec)
{
    float result;
    result = (vec.getX() * vec.getX());
    result = (result + (vec.getY() * vec.getY()));
    return result;
}

inline float length(const Vector2 & vec)
{
    return sqrtf(lengthSqr(vec));
}

inline const Vector2 normalize(const Vector2 & vec)
{
    const float lenSqr = lengthSqr(vec);
    const float lenInv = (1.0f / sqrtf(lenSqr));
    return Vector2((vec.getX() * lenInv), (vec.getY() * lenInv));
}

inline const Vector2 lerp(float t, const Vector2 & vec0, const Vector2 & vec1)
{
    return (vec0 + ((vec1 - vec0) * t));
}

#ifdef VECTORMATH_DEBUG

inline void print(const Vector2 & vec)
{
    printf("( %f %f )\n", vec.getX(), vec.getY());
}

inline void print(const Vector2 & vec, const char * name)
{
    printf("%s: ( %f %f )\n", name, vec.getX(), vec.getY());
}

#endif // VECTORMATH_DEBUG

// ================================================================================================
// Point2 implementation
// ================================================================================================

inline Point2::Point2(float _x, float _y)
    : mX(_x), mY(_y)
{
}

inline Point2::Point2(const Vector2 & vec)
    : mX(vec.getX()), mY(vec.getY())
{
}

inline Point2::Point2(float scalar)
    : mX(scalar), mY(scalar)
{
}

inline Point2 & Point2::setX(float _x)
{
    mX = _x;
    return *this;
}

inline Point2 & Point2::setY(float _y)
{
    mY = _y;
    return *this;
}

inline float Point2::getX() const
{
    return mX;
}

inline float Point2::getY() const
{
    return mY;
}

inline Point2 & Point2::setElem(int idx, float value)
{
    *(&mX + idx) = value;
    return *this;
}

inline float Point2::getElem(int idx) const
{
    return *(&mX + idx);
}

inline float & Point2::operator[](int idx)
{
    return *(&mX + idx);
}

inline float Point2::operator[](int idx) const
{
    return *(&mX + idx);
}

inline const Vector2 Point2::operator - (const Point2 & pnt) const
{
    return Vector2((mX - pnt.mX), (mY - pnt.mY));
}

inline const Point2 Point2::operator + (const Vector2 & vec) const
{
    return Point2((mX + vec.getX()), (mY + vec.getY()));
}

inline const Point2 Point2::operator - (const Vector2 & vec) const
{
    return Point2((mX - vec.getX()), (mY - vec.getY()));
}

inline Point2 & Point2::operator += (const Vector2 & vec)
{
    mX += vec.getX();
    mY += vec.getY();
    return *this;
}

inline Point2 & Point2::operator -= (const Vector2 & vec)
{
    mX -= vec.getX();
    mY -= vec.getY();
    return *this;
}

inline const Point2 absPerElem(const Point2 & pnt)
{
    return Point2(fabsf(pnt.getX()), fabsf(pnt.getY()));
}

inline const Point2 maxPerElem(const Point2 & pnt0, const Point2 & pnt1)
{
    return Point2((pnt0.getX() > pnt1.getX()) ? pnt0.getX() : pnt1.getX(),
                  (pnt0.getY() > pnt1.getY()) ? pnt0.getY() : pnt1.getY());
}

inline const Point2 minPerElem(const Point2 & pnt0, const Point2 & pnt1)
{
    return Point2((pnt0.getX() < pnt1.getX()) ? pnt0.getX() : pnt1.getX(),
                  (pnt0.getY() < pnt1.getY()) ? pnt0.getY() : pnt1.getY());
}

inline float maxElem(const Point2 & pnt)
{
    return (pnt.getX() > pnt.getY()) ? pnt.getX() : pnt.getY();
}

inline float minElem(const Point2 & pnt)
{
    return (pnt.getX() < pnt.getY()) ? pnt.getX() : pnt.getY();
}

inline float distSqrFromOrigin(const Point2 & pnt)
{
    return lengthSqr(Vector2(pnt));
}

inline float distFromOrigin(const Point2 & pnt)
{
    return length(Vector2(pnt));
}

inline float distSqr(const Point2 & pnt0, const Point2 & pnt1)
{
    return lengthSqr(pnt1 - pnt0);
}

inline float distp(const Point2 & pnt0, const Point2 & pnt1)
{
    return length(pnt1 - pnt0);
}

inline const Point2 lerp(float t, const Point2 & pnt0, const Point2 & pnt1)
{
    return (pnt0 + ((pnt1 - pnt0) * t));
}

#ifdef VECTORMATH_DEBUG

inline void print(const Point2 & pnt)
{
    printf("( %f %f )\n", pnt.getX(), pnt.getY());
}

inline void print(const Point2 & pnt, const char * name)
{
    printf("%s: ( %f %f )\n", name, pnt.getX(), pnt.getY());
}

#endif // VECTORMATH_DEBUG

} // namespace Vectormath

#endif // VECTORMATH_VEC2D_HPP

// ================================================================================================
// -*- C++ -*-
// File: vectormath/common.hpp
// Author: Guilherme R. Lampert
// Created on: 30/12/16
// Brief: Extra helper functions added to the Vectormath library.
// ================================================================================================

#ifndef VECTORMATH_COMMON_HPP
#define VECTORMATH_COMMON_HPP

namespace Vectormath
{

inline float * toFloatPtr(Point2  & p)    { return reinterpret_cast<float *>(&p); } //  2 floats - default alignment
inline float * toFloatPtr(Point3  & p)    { return reinterpret_cast<float *>(&p); } //  4 floats - 16 bytes aligned
inline float * toFloatPtr(Vector2 & v)    { return reinterpret_cast<float *>(&v); } //  2 floats - default alignment
inline float * toFloatPtr(Vector3 & v)    { return reinterpret_cast<float *>(&v); } //  4 floats - 16 bytes aligned
inline float * toFloatPtr(Vector4 & v)    { return reinterpret_cast<float *>(&v); } //  4 floats - 16 bytes aligned
inline float * toFloatPtr(Quat    & q)    { return reinterpret_cast<float *>(&q); } //  4 floats - 16 bytes aligned
inline float * toFloatPtr(Matrix3 & m)    { return reinterpret_cast<float *>(&m); } // 12 floats - 16 bytes aligned
inline float * toFloatPtr(Matrix4 & m)    { return reinterpret_cast<float *>(&m); } // 16 floats - 16 bytes aligned
inline float * toFloatPtr(Transform3 & t) { return reinterpret_cast<float *>(&t); } // 16 floats - 16 bytes aligned

inline const float * toFloatPtr(const Point2  & p)    { return reinterpret_cast<const float *>(&p); }
inline const float * toFloatPtr(const Point3  & p)    { return reinterpret_cast<const float *>(&p); }
inline const float * toFloatPtr(const Vector2 & v)    { return reinterpret_cast<const float *>(&v); }
inline const float * toFloatPtr(const Vector3 & v)    { return reinterpret_cast<const float *>(&v); }
inline const float * toFloatPtr(const Vector4 & v)    { return reinterpret_cast<const float *>(&v); }
inline const float * toFloatPtr(const Quat    & q)    { return reinterpret_cast<const float *>(&q); }
inline const float * toFloatPtr(const Matrix3 & m)    { return reinterpret_cast<const float *>(&m); }
inline const float * toFloatPtr(const Matrix4 & m)    { return reinterpret_cast<const float *>(&m); }
inline const float * toFloatPtr(const Transform3 & t) { return reinterpret_cast<const float *>(&t); }

// Shorthand to discard the last element of a Vector4 and get a Point3.
inline Point3 toPoint3(const Vector4 & v4)
{
    return Point3(v4[0], v4[1], v4[2]);
}

// Convert from world (global) coordinates to local model coordinates.
// Input matrix must be the inverse of the model matrix, e.g.: 'inverse(modelMatrix)'.
inline Point3 worldPointToModel(const Matrix4 & invModelToWorldMatrix, const Point3 & point)
{
    return toPoint3(invModelToWorldMatrix * point);
}

// Makes a plane projection matrix that can be used for simple object shadow effects.
// The W component of the light position vector should be 1 for a point light and 0 for directional.
inline Matrix4 makeShadowMatrix(const Vector4 & plane, const Vector4 & light)
{
    Matrix4 shadowMat;
    const auto dot = (plane[0] * light[0]) +
                     (plane[1] * light[1]) +
                     (plane[2] * light[2]) +
                     (plane[3] * light[3]);

    shadowMat[0][0] = dot - (light[0] * plane[0]);
    shadowMat[1][0] =     - (light[0] * plane[1]);
    shadowMat[2][0] =     - (light[0] * plane[2]);
    shadowMat[3][0] =     - (light[0] * plane[3]);

    shadowMat[0][1] =     - (light[1] * plane[0]);
    shadowMat[1][1] = dot - (light[1] * plane[1]);
    shadowMat[2][1] =     - (light[1] * plane[2]);
    shadowMat[3][1] =     - (light[1] * plane[3]);

    shadowMat[0][2] =     - (light[2] * plane[0]);
    shadowMat[1][2] =     - (light[2] * plane[1]);
    shadowMat[2][2] = dot - (light[2] * plane[2]);
    shadowMat[3][2] =     - (light[2] * plane[3]);

    shadowMat[0][3] =     - (light[3] * plane[0]);
    shadowMat[1][3] =     - (light[3] * plane[1]);
    shadowMat[2][3] =     - (light[3] * plane[2]);
    shadowMat[3][3] = dot - (light[3] * plane[3]);

    return shadowMat;
}

} // namespace Vectormath

#endif // VECTORMATH_COMMON_HPP

using namespace Vectormath;

#endif // VECTORMATH_HPP
