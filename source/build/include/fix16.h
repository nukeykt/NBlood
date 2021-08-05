#ifndef __libfixmath_fix16_h__
#define __libfixmath_fix16_h__

#include "compat.h"

#ifdef __cplusplus
extern "C"
{
#endif


/* These options may let the optimizer to remove some calls to the functions.
 * Refer to http://gcc.gnu.org/onlinedocs/gcc/Function-Attributes.html
 */
#ifndef FIXMATH_FUNC_ATTRS
# ifdef __GNUC__
#   if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 6)
#     define FIXMATH_FUNC_ATTRS __attribute__((leaf, nothrow, const))
#   else
#     define FIXMATH_FUNC_ATTRS __attribute__((nothrow, const))
#   endif
# else
#   define FIXMATH_FUNC_ATTRS
# endif
#endif

#include <stdint.h>

typedef int32_t fix16_t;

static CONSTEXPR const fix16_t FIX16_MAX      = 0x7FFFFFFF; /*!< the maximum value of fix16_t */
static CONSTEXPR const fix16_t FIX16_MIN      = 0x80000000; /*!< the minimum value of fix16_t */
static CONSTEXPR const fix16_t FIX16_OVERFLOW = 0x80000000; /*!< the value used to indicate overflows */

static CONSTEXPR const fix16_t fix16_one  = 0x00010000;       /*!< fix16_t value of 1 */
static CONSTEXPR const fix16_t fix16_half = 0x00008000;       /*!< fix16_t value of 0.5 */
static CONSTEXPR const float   flt_f161r  = 1.f / 0x00010000;  /*!< reciprocal of fix16_one as a float */

/* Conversion functions between fix16_t and float/integer.
 * These are inlined to allow compiler to optimize away constant numbers
 */
static FORCE_INLINE CONSTEXPR int    fix16_to_int(fix16_t a)   { return (a + (a >= 0 ? fix16_half : -fix16_half)) / fix16_one; }
static FORCE_INLINE CONSTEXPR float  fix16_to_float(fix16_t a) { return (float)a * flt_f161r; }
static FORCE_INLINE CONSTEXPR double fix16_to_dbl(fix16_t a)   { return (double)a * flt_f161r; }

static FORCE_INLINE CONSTEXPR fix16_t fix16_from_int(int a)     { return a * fix16_one; }
static FORCE_INLINE CONSTEXPR fix16_t fix16_from_float(float a) { return (fix16_t)(a * fix16_one + (a >= 0 ? 0.5f : -0.5f)); }
static FORCE_INLINE CONSTEXPR fix16_t fix16_from_dbl(double a)  { return (fix16_t)(a * fix16_one + (a >= 0 ? 0.5 : -0.5)); }

/* Macro for defining fix16_t constant values.
   The functions above can't be used from e.g. global variable initializers,
   and their names are quite long also. This macro is useful for constants
   springled alongside code, e.g. F16(1.234).

   Note that the argument is evaluated multiple times, and also otherwise
   you should only use this for constant values. For runtime-conversions,
   use the functions above.
*/
#define F16(x) ((fix16_t)(((x) >= 0) ? ((x) * 65536.0 + 0.5) : ((x) * 65536.0 - 0.5)))

static FORCE_INLINE fix16_t fix16_abs(fix16_t const x)   { return abs(x); }
static FORCE_INLINE CONSTEXPR fix16_t fix16_floor(fix16_t const x) { return (x & 0xFFFF0000UL); }
static FORCE_INLINE CONSTEXPR fix16_t fix16_ceil(fix16_t const x)  { return (x & 0xFFFF0000UL) + ((x & 0x0000FFFFUL) ? fix16_one : 0); }
static FORCE_INLINE fix16_t fix16_min(fix16_t const x, fix16_t const y) { return min(x, y); }
static FORCE_INLINE fix16_t fix16_max(fix16_t const x, fix16_t const y) { return max(x, y); }
static FORCE_INLINE fix16_t fix16_clamp(fix16_t const x, fix16_t const lo, fix16_t const hi) { return clamp(x, lo, hi); }

/* Subtraction and addition with overflow detection. */
extern fix16_t fix16_add(fix16_t a, fix16_t b) FIXMATH_FUNC_ATTRS;
extern fix16_t fix16_sub(fix16_t a, fix16_t b) FIXMATH_FUNC_ATTRS;

/* Saturating arithmetic */
extern fix16_t fix16_sadd(fix16_t a, fix16_t b) FIXMATH_FUNC_ATTRS;
extern fix16_t fix16_ssub(fix16_t a, fix16_t b) FIXMATH_FUNC_ATTRS;

/*! Multiplies the two given fix16_t's and returns the result.
*/
extern fix16_t fix16_mul(fix16_t inArg0, fix16_t inArg1) FIXMATH_FUNC_ATTRS;

/*! Divides the first given fix16_t by the second and returns the result.
*/
extern fix16_t fix16_div(fix16_t a, fix16_t b) FIXMATH_FUNC_ATTRS;

/*! Performs a saturated multiplication (overflow-protected) of the two given fix16_t's and returns the result.
*/
extern fix16_t fix16_smul(fix16_t inArg0, fix16_t inArg1) FIXMATH_FUNC_ATTRS;

/*! Performs a saturated division (overflow-protected) of the first fix16_t by the second and returns the result.
*/
extern fix16_t fix16_sdiv(fix16_t inArg0, fix16_t inArg1) FIXMATH_FUNC_ATTRS;

/*! Divides the first given fix16_t by the second and returns the remainder.
*/
static FORCE_INLINE fix16_t fix16_mod(fix16_t x, fix16_t y) { return x %= y; }

/*! Performs a fast multiplication of the two given fix16_t's with truncation, and returns the result.
*/
static FORCE_INLINE fix16_t fix16_fast_trunc_mul(fix16_t x, fix16_t y) { return ((int64_t)x * y) >> 16; }

/*! Performs a fast multiplication of an int by a fix16_t with truncation, and returns an int.
*/
static FORCE_INLINE int fix16_fast_trunc_mul_int_by_fix16(int x, fix16_t y) { return ((int64_t)x * y) >> 16; }

/*! Returns the linear interpolation: (inArg0 * (1 - inFract)) + (inArg1 * inFract)
*/
extern fix16_t fix16_lerp8(fix16_t inArg0, fix16_t inArg1, uint8_t inFract) FIXMATH_FUNC_ATTRS;
extern fix16_t fix16_lerp16(fix16_t inArg0, fix16_t inArg1, uint16_t inFract) FIXMATH_FUNC_ATTRS;
extern fix16_t fix16_lerp32(fix16_t inArg0, fix16_t inArg1, uint32_t inFract) FIXMATH_FUNC_ATTRS;


static CONSTEXPR const fix16_t fix16_rad_to_deg_mult = 3754936;
static FORCE_INLINE fix16_t fix16_rad_to_deg(fix16_t radians) { return fix16_mul(radians, fix16_rad_to_deg_mult); }

static CONSTEXPR const fix16_t fix16_deg_to_rad_mult = 1144;
static FORCE_INLINE fix16_t fix16_deg_to_rad(fix16_t degrees) { return fix16_mul(degrees, fix16_deg_to_rad_mult); }


/*! Returns the square of the given fix16_t.
*/
static inline fix16_t fix16_sq(fix16_t x) { return fix16_mul(x, x); }

/*! Convert fix16_t value to a string.
 * Required buffer length for largest values is 13 bytes.
 */
extern void fix16_to_str(fix16_t value, char *buf, int decimals);

/*! Convert string to a fix16_t value
 * Ignores spaces at beginning and end. Returns fix16_overflow if
 * value is too large or there were garbage characters.
 */
extern fix16_t fix16_from_str(const char *buf);

#ifdef __cplusplus
}
#include "pragmas.h"
#include "fix16.hpp"
#endif

#endif
