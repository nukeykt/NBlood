// LIGHTS
#ifndef prlight_h_
# define prlight_h_

#define             PR_MAXLIGHTS            1024
#define             PR_MAXPLANELIGHTS       32
#define             SHADOW_DEPTH_OFFSET     30
#define             PR_MAXLIGHTPRIORITY     6

typedef struct      s_prplanelist {
    struct s_prplane*       plane;
    struct s_prplanelist*   n;
}                   _prplanelist;

#pragma pack(push,1)
typedef struct      s_prlight {
    union {
        struct
        {
            int32_t x, y, z;
        };
        vec3_t xyz;
        vec2_t xy;
    };
    int32_t         horiz, range;
    int16_t         angle, faderadius, radius, sector;
    uint8_t         color[3], priority;
    int8_t          minshade, maxshade;
    int16_t         tilenum, owner;
    struct          {
        int         emitshadow  : 1;
        int         negative    : 1;
    }               publicflags;
    // internal members
    float           proj[16];
    float           transform[16];
    float           frustum[5 * 4];
    int32_t         rtindex;
    struct          {
        int         active      : 1;
        int         invalidate  : 1;
        int         isinview    : 1;
    }               flags;
    uint32_t        lightmap;
    _prplanelist*   planelist;
    int32_t         planecount;
}                   _prlight;

extern _prlight     prlights[PR_MAXLIGHTS];
extern int32_t      lightcount;
#pragma pack(pop)

#endif
