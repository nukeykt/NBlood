#pragma once

#ifndef vec_h__
#define vec_h__

#include "compat.h"

typedef struct vec2_ {
    int32_t x, y;
#ifdef __cplusplus
    inline bool operator==(struct vec2_ const &c) { return x == c.x && y == c.y; }
    inline bool operator!=(struct vec2_ const &c) { return x != c.x || y != c.y; }

    struct vec2_ &operator+=(const struct vec2_& rhs) { x += rhs.x; y += rhs.y; return *this; }
    struct vec2_ &operator-=(const struct vec2_& rhs) { x -= rhs.x; y -= rhs.y; return *this; }

    const struct vec2_ operator+(const struct vec2_ rhs) const { return { x + rhs.x, y + rhs.y }; }
    const struct vec2_ operator-(const struct vec2_ rhs) const { return { x - rhs.x, y - rhs.y }; }
#endif
} MAY_ALIAS vec2_t;

typedef struct vec2_16_ {
    int16_t x, y;
#ifdef __cplusplus
    inline bool operator==(struct vec2_16_ const &c) { return x == c.x && y == c.y; }
    inline bool operator!=(struct vec2_16_ const &c) { return x != c.x || y != c.y; }

    struct vec2_16_ &operator+=(const struct vec2_16_& rhs) { x += rhs.x; y += rhs.y; return *this; }
    struct vec2_16_ &operator-=(const struct vec2_16_& rhs) { x -= rhs.x; y -= rhs.y; return *this; }

    const struct vec2_16_ operator+(const struct vec2_16_ rhs) const { return { (int16_t)(x + rhs.x), (int16_t)(y + rhs.y) }; }
    const struct vec2_16_ operator-(const struct vec2_16_ rhs) const { return { (int16_t)(x - rhs.x), (int16_t)(y - rhs.y) }; }
#endif // __cplusplus
} MAY_ALIAS vec2_16_t;

typedef struct vec2u_ {
    uint32_t x, y;
#ifdef __cplusplus
    inline bool operator==(struct vec2u_ const &c) { return x == c.x && y == c.y; }
    inline bool operator!=(struct vec2u_ const &c) { return x != c.x || y != c.y; }

    struct vec2u_ &operator+=(const struct vec2u_& rhs) { x += rhs.x; y += rhs.y; return *this; }
    struct vec2u_ &operator-=(const struct vec2u_& rhs) { x -= rhs.x; y -= rhs.y; return *this; }

    const struct vec2u_ operator+(const struct vec2u_ rhs) const { return { x + rhs.x, y + rhs.y }; }
    const struct vec2u_ operator-(const struct vec2u_ rhs) const { return { x - rhs.x, y - rhs.y }; }
#endif // __cplusplus
} vec2u_t;

typedef struct vec2f_ {
    float x, y;
#ifdef __cplusplus
    inline bool operator==(struct vec2f_ const &c) { return x == c.x && y == c.y; }
    inline bool operator!=(struct vec2f_ const &c) { return x != c.x || y != c.y; }

    struct vec2f_ &operator+=(const struct vec2f_& rhs) { x += rhs.x; y += rhs.y; return *this; }
    struct vec2f_ &operator-=(const struct vec2f_& rhs) { x -= rhs.x; y -= rhs.y; return *this; }

    const struct vec2f_ operator+(const struct vec2f_ rhs) const { return { x + rhs.x, y + rhs.y }; }
    const struct vec2f_ operator-(const struct vec2f_ rhs) const { return { x - rhs.x, y - rhs.y }; }
#endif // __cplusplus
} vec2f_t;

typedef struct vec2d_ {
    double x, y;
#ifdef __cplusplus
    inline bool operator==(struct vec2d_ const &c) { return x == c.x && y == c.y; }
    inline bool operator!=(struct vec2d_ const &c) { return x != c.x || y != c.y; }

    struct vec2d_ &operator+=(const struct vec2d_& rhs) { x += rhs.x; y += rhs.y; return *this; }
    struct vec2d_ &operator-=(const struct vec2d_& rhs) { x -= rhs.x; y -= rhs.y; return *this; }

    const struct vec2d_ operator+(const struct vec2d_ rhs) const { return { x + rhs.x, y + rhs.y }; }
    const struct vec2d_ operator-(const struct vec2d_ rhs) const { return { x - rhs.x, y - rhs.y }; }
#endif // __cplusplus
} vec2d_t;

typedef struct vec3_ {
    union {
        struct { int32_t x, y, z; };
        vec2_t xy;
    };
#ifdef __cplusplus
    inline bool operator==(struct vec3_ const &c) { return x == c.x && y == c.y && z == c.z; }
    inline bool operator!=(struct vec3_ const &c) { return x != c.x || y != c.y || z != c.z; }

    struct vec3_ &operator+=(const struct vec3_& rhs) { x += rhs.x; y += rhs.y; z+= rhs.z; return *this; }
    struct vec3_ &operator-=(const struct vec3_& rhs) { x -= rhs.x; y -= rhs.y; z-= rhs.z; return *this; }

    struct vec3_ &operator+=(const struct vec2_& rhs) { xy += rhs; return *this; }
    struct vec3_ &operator-=(const struct vec2_& rhs) { xy -= rhs; return *this; }

    const struct vec3_ operator+(const struct vec3_ rhs) const { return { x + rhs.x, y + rhs.y, z + rhs.z }; }
    const struct vec3_ operator-(const struct vec3_ rhs) const { return { x - rhs.x, y - rhs.y, z + rhs.z }; }
#endif // __cplusplus
} MAY_ALIAS vec3_t;

typedef struct vec3_16_ {
    union {
        struct { int16_t x, y, z; };
        vec2_16_t xy;
    };
#ifdef __cplusplus
    inline bool operator==(struct vec3_16_ const &c) { return x == c.x && y == c.y && z == c.z; }
    inline bool operator!=(struct vec3_16_ const &c) { return x != c.x || y != c.y || z != c.z; }

    struct vec3_16_ &operator+=(const struct vec3_16_& rhs) { x += rhs.x; y += rhs.y; z+= rhs.z; return *this; }
    struct vec3_16_ &operator-=(const struct vec3_16_& rhs) { x -= rhs.x; y -= rhs.y; z-= rhs.z; return *this; }

    struct vec3_16_ &operator+=(const struct vec2_16_& rhs) { xy += rhs; return *this; }
    struct vec3_16_ &operator-=(const struct vec2_16_& rhs) { xy -= rhs; return *this; }

    const struct vec3_16_ operator+(const struct vec3_16_ rhs) const { return { (int16_t)(x + rhs.x), (int16_t)(y + rhs.y), (int16_t)(z + rhs.z) }; }
    const struct vec3_16_ operator-(const struct vec3_16_ rhs) const { return { (int16_t)(x - rhs.x), (int16_t)(y - rhs.y), (int16_t)(z + rhs.z) }; }
#endif // __cplusplus
} MAY_ALIAS vec3_16_t;

typedef struct vec3f_ {
    union {
        struct {
            union { float x, d; };
            union { float y, u; };
            union { float z, v; };
        };
        vec2f_t xy;
    };
#ifdef __cplusplus
    inline bool operator==(struct vec3f_ const &c) { return x == c.x && y == c.y && z == c.z; }
    inline bool operator!=(struct vec3f_ const &c) { return x != c.x || y != c.y || z != c.z; }

    struct vec3f_ &operator+=(const struct vec3f_& rhs) { x += rhs.x; y += rhs.y; z+= rhs.z; return *this; }
    struct vec3f_ &operator-=(const struct vec3f_& rhs) { x -= rhs.x; y -= rhs.y; z-= rhs.z; return *this; }

    struct vec3f_ &operator+=(const struct vec2f_& rhs) { xy += rhs; return *this; }
    struct vec3f_ &operator-=(const struct vec2f_& rhs) { xy -= rhs; return *this; }

    const struct vec3f_ operator+(const struct vec3f_ rhs) const { return { x + rhs.x, y + rhs.y, z + rhs.z }; }
    const struct vec3f_ operator-(const struct vec3f_ rhs) const { return { x - rhs.x, y - rhs.y, z + rhs.z }; }
#endif // __cplusplus
} vec3f_t;

EDUKE32_STATIC_ASSERT(sizeof(vec3f_t) == sizeof(float) * 3);

typedef struct vec3d_ {
    union {
        struct {
            union { double x, d; };
            union { double y, u; };
            union { double z, v; };
        };
        vec2d_t xy;
    };
#ifdef __cplusplus
    inline bool operator==(struct vec3d_ const &c) { return x == c.x && y == c.y && z == c.z; }
    inline bool operator!=(struct vec3d_ const &c) { return x != c.x || y != c.y || z != c.z; }

    struct vec3d_ &operator+=(const struct vec3d_& rhs) { x += rhs.x; y += rhs.y; z+= rhs.z; return *this; }
    struct vec3d_ &operator-=(const struct vec3d_& rhs) { x -= rhs.x; y -= rhs.y; z-= rhs.z; return *this; }

    struct vec3d_ &operator+=(const struct vec2d_& rhs) { xy += rhs; return *this; }
    struct vec3d_ &operator-=(const struct vec2d_& rhs) { xy -= rhs; return *this; }

    const struct vec3d_ operator+(const struct vec3d_ rhs) const { return { x + rhs.x, y + rhs.y, z + rhs.z }; }
    const struct vec3d_ operator-(const struct vec3d_ rhs) const { return { x - rhs.x, y - rhs.y, z + rhs.z }; }
#endif // __cplusplus
} vec3d_t;

EDUKE32_STATIC_ASSERT(sizeof(vec3d_t) == sizeof(double) * 3);

typedef struct vec4_ {
    union {
        struct { int32_t x, y, z, a; };
        vec3_t xyz;
        vec2_t xy;
    };
#ifdef __cplusplus
    inline bool operator==(struct vec4_ const &c) { return x == c.x && y == c.y && z == c.z && a == c.a; }
    inline bool operator!=(struct vec4_ const &c) { return x != c.x || y != c.y || z != c.z || a != c.a; }
    struct vec4_ &operator+=(const struct vec4_& rhs) { x += rhs.x; y += rhs.y; z+= rhs.z; a += rhs.a; return *this; }
    struct vec4_ &operator-=(const struct vec4_& rhs) { x -= rhs.x; y -= rhs.y; z-= rhs.z; a -= rhs.a; return *this; }
#endif // __cplusplus
} vec4_t;

typedef struct vec4f_ {
    union {
        struct {
            union { float x, d; };
            union { float y, u; };
            union { float z, v; };
            float w;
        };
        vec3f_t xyz;
        vec2f_t xy;
    };
#ifdef __cplusplus
    inline bool operator==(struct vec4f_ const &c) { return x == c.x && y == c.y && z == c.z && w == c.w; }
    inline bool operator!=(struct vec4f_ const &c) { return x != c.x || y != c.y || z != c.z || w != c.w; }
    struct vec4f_ &operator+=(const struct vec4f_& rhs) { x += rhs.x; y += rhs.y; z+= rhs.z; w += rhs.w; return *this; }
    struct vec4f_ &operator-=(const struct vec4f_& rhs) { x -= rhs.x; y -= rhs.y; z-= rhs.z; w -= rhs.w; return *this; }
#endif // __cplusplus
} vec4f_t;

#endif // vec_h__