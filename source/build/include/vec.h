#pragma once

#ifndef vec_h__
#define vec_h__

#include "compat.h"

#ifdef __cplusplus
template <typename T> struct vec2
{
    T x, y;

    inline bool operator==(vec2<T> const c) { return (x == c.x) & (y == c.y); }
    inline bool operator!=(vec2<T> const c) { return (x != c.x) | (y != c.y); }

    vec2<T> &operator+=(const vec2<T> rhs) { x += rhs.x; y += rhs.y; return *this; }
    vec2<T> &operator-=(const vec2<T> rhs) { x -= rhs.x; y -= rhs.y; return *this; }
    vec2<T> &operator*=(const vec2<T> rhs) { x *= rhs.x; y *= rhs.y; return *this; }

    const vec2<T> operator+(const vec2<T> rhs) const { return { (T)(x + rhs.x), (T)(y + rhs.y) }; }
    const vec2<T> operator-(const vec2<T> rhs) const { return { (T)(x - rhs.x), (T)(y - rhs.y) }; }
    const vec2<T> operator*(const vec2<T> rhs) const { return { (T)(x * rhs.x), (T)(y * rhs.y) }; }
};

template <typename T> struct vec3
{
    union {
        struct { T x, y, z; };
        struct { T d, u, v; };
        vec2<T> xy;
    };
    inline bool operator==(vec3<T> const &c) { return (x == c.x) & (y == c.y) & (z == c.z); }
    inline bool operator!=(vec3<T> const &c) { return (x != c.x) | (y != c.y) | (z != c.z); }

    vec3<T> &operator+=(const vec3<T>& rhs) { x += rhs.x; y += rhs.y; z+= rhs.z; return *this; }
    vec3<T> &operator-=(const vec3<T>& rhs) { x -= rhs.x; y -= rhs.y; z-= rhs.z; return *this; }
    vec3<T> &operator*=(const vec3<T>& rhs) { x *= rhs.x; y *= rhs.y; z*= rhs.z; return *this; }

    const vec3<T> operator+(const vec3<T> &rhs) const { return { (T)(x + rhs.x), (T)(y + rhs.y), (T)(z + rhs.z) }; }
    const vec3<T> operator-(const vec3<T> &rhs) const { return { (T)(x - rhs.x), (T)(y - rhs.y), (T)(z - rhs.z) }; }
    const vec3<T> operator*(const vec3<T> &rhs) const { return { (T)(x * rhs.x), (T)(y * rhs.y), (T)(z * rhs.z) }; }

    vec3<T> &operator+=(const vec2<T>& rhs) { xy += rhs; return *this; }
    vec3<T> &operator-=(const vec2<T>& rhs) { xy -= rhs; return *this; }
    vec3<T> &operator*=(const vec2<T>& rhs) { xy *= rhs; return *this; }
};

template <typename T> struct vec4
{
    union {
        struct { T x, y, z, a; };
        struct { T d, u, v, w; };
        vec3<T> xyz;
        vec2<T> xy;
    };
    inline bool operator==(vec4<T> const &c) { return (x == c.x) & (y == c.y) & (z == c.z) & (a == c.a); }
    inline bool operator!=(vec4<T> const &c) { return (x != c.x) | (y != c.y) | (z != c.z) | (a != c.a); }

    vec4<T> &operator+=(const vec4<T>& rhs) { x += rhs.x; y += rhs.y; z+= rhs.z; a += rhs.a; return *this; }
    vec4<T> &operator-=(const vec4<T>& rhs) { x -= rhs.x; y -= rhs.y; z-= rhs.z; a -= rhs.a; return *this; }
    vec4<T> &operator*=(const vec4<T>& rhs) { x *= rhs.x; y *= rhs.y; z*= rhs.z; a *= rhs.a; return *this; }

    const vec4<T> operator+(const vec4<T> &rhs) const { return { (T)(x + rhs.x), (T)(y + rhs.y), (T)(z + rhs.z), (T)(a + rhs.a) }; }
    const vec4<T> operator-(const vec4<T> &rhs) const { return { (T)(x - rhs.x), (T)(y - rhs.y), (T)(z - rhs.z), (T)(a - rhs.a) }; }
    const vec4<T> operator*(const vec4<T> &rhs) const { return { (T)(x * rhs.x), (T)(y * rhs.y), (T)(z * rhs.z), (T)(a * rhs.a) }; }

    vec4<T> &operator+=(const vec3<T>& rhs) { xyz += rhs; return *this; }
    vec4<T> &operator-=(const vec3<T>& rhs) { xyz -= rhs; return *this; }
    vec4<T> &operator*=(const vec3<T>& rhs) { xyz *= rhs; return *this; }

    vec4<T> &operator+=(const vec2<T>& rhs) { xy += rhs; return *this; }
    vec4<T> &operator-=(const vec2<T>& rhs) { xy -= rhs; return *this; }
    vec4<T> &operator*=(const vec2<T>& rhs) { xy *= rhs; return *this; }
};

typedef struct vec2<double> vec2d_t;
typedef struct vec3<double> vec3d_t;

typedef struct vec2<float> vec2f_t;
typedef struct vec3<float> vec3f_t;
typedef struct vec4<float> vec4f_t;

typedef struct vec2<int16_t> vec2_16_t;
typedef struct vec3<int16_t> vec3_16_t;

typedef struct vec2<uint16_t> vec2_u16_t;
typedef struct vec3<uint16_t> vec3_u16_t;

typedef struct vec2<int32_t> vec2_t;
typedef struct vec3<int32_t> vec3_t;
typedef struct vec4<int32_t> vec4_t;

typedef struct vec2<uint32_t> vec2u_t;

#else // __cplusplus

typedef struct { int16_t x, y; } vec2_16_t;
typedef struct { union { struct { int16_t x, y, z; }; vec2_16_t xy; }; } vec3_16_t;

typedef struct { int32_t x, y; } vec2_t;
typedef struct { union { struct { int32_t x, y, z; }; vec2_t xy; };                } vec3_t;
typedef struct { union { struct { int32_t x, y, z, a; }; vec3_t xyz; vec2_t xy; }; } vec4_t;

typedef struct { uint32_t x, y; } vec2u_t;
typedef struct { float x, y; }    vec2f_t;
typedef struct { double x, y; }   vec2d_t;

typedef struct { union { union { struct { float x, y, z; };    struct { float d, u, v; }; };  vec2f_t xy; };              } vec3f_t;
typedef struct { union { union { struct { double x, y, z; };   struct { double d, u, v; }; }; vec2d_t xy; };              } vec3d_t;
typedef struct { union { union { struct { float x, y, z, w; }; struct { float d, u, v; }; };  vec3f_t xyz; vec2f_t xy; }; } vec4f_t;

#endif // __cplusplus

// basic sanity checks
EDUKE32_STATIC_ASSERT(sizeof(vec2_t) == sizeof(int32_t) * 2);
EDUKE32_STATIC_ASSERT(sizeof(vec3_t) == sizeof(int32_t) * 3);
EDUKE32_STATIC_ASSERT(sizeof(vec3f_t) == sizeof(float) * 3);
EDUKE32_STATIC_ASSERT(sizeof(vec3d_t) == sizeof(double) * 3);

#endif // vec_h__
