#pragma once

#include "compat.h"

#define RT_TILENUM 2851
#define RT_BOARDNUM 34
#define RT_PALNUM 12


#define RT_TILE8BIT 0x8000

enum rt_con_t {
    RT_CON_DEFINELEVELNAME,     // 0
    RT_CON_ACTOR,               // 1
    RT_CON_ADDAMMO,             // 2
    RT_CON_IFRND,               // 3
    RT_CON_ENDA,                // 4
    RT_CON_IFCANSEE,            // 5
    RT_CON_IFHITWEAPON,         // 6
    RT_CON_ACTION,              // 7
    RT_CON_IFPDISTL,            // 8
    RT_CON_IFPDISTG,            // 9
    RT_CON_ELSE,                // 10
    RT_CON_STRENGTH,            // 11
    RT_CON_BREAK,               // 12
    RT_CON_SHOOT,               // 13
    RT_CON_PALFROM,             // 14
    RT_CON_SOUND,               // 15
    RT_CON_FALL,                // 16
    RT_CON_STATE,               // 17
    RT_CON_ENDS,                // 18
    RT_CON_DEFINE,              // 19
    RT_CON_COMMENT,             // 20
    RT_CON_IFAI,                // 21
    RT_CON_KILLIT,              // 22
    RT_CON_ADDWEAPON,           // 23
    RT_CON_AI,                  // 24
    RT_CON_ADDPHEALTH,          // 25
    RT_CON_IFDEAD,              // 26
    RT_CON_IFSQUISHED,          // 27
    RT_CON_SIZETO,              // 28
    RT_CON_LEFTBRACE,           // 29
    RT_CON_RIGHTBRACE,          // 30
    RT_CON_SPAWN,               // 31
    RT_CON_MOVE,                // 32
    RT_CON_IFWASWEAPON,         // 33
    RT_CON_IFACTION,            // 34
    RT_CON_IFACTIONCOUNT,       // 35
    RT_CON_RESETACTIONCOUNT,    // 36
    RT_CON_DEBRIS,              // 37
    RT_CON_PSTOMP,              // 38
    RT_CON_BLOCKCOMMENT,        // 39
    RT_CON_CSTAT,               // 40
    RT_CON_IFMOVE,              // 41
    RT_CON_RESETPLAYER,         // 42
    RT_CON_IFONWATER,           // 43
    RT_CON_IFINWATER,           // 44
    RT_CON_IFCANSHOOTTARGET,    // 45
    RT_CON_IFCOUNT,             // 46
    RT_CON_RESETCOUNT,          // 47
    RT_CON_ADDINVENTORY,        // 48
    RT_CON_IFACTORNOTSTAYPUT,   // 49
    RT_CON_HITRADIUS,           // 50
    RT_CON_IFP,                 // 51
    RT_CON_COUNT,               // 52
    RT_CON_IFACTOR,             // 53
    RT_CON_MUSIC,               // 54
    RT_CON_INCLUDE,             // 55
    RT_CON_IFSTRENGTH,          // 56
    RT_CON_DEFINESOUND,         // 57
    RT_CON_GUTS,                // 58
    RT_CON_IFSPAWNEDBY,         // 59
    RT_CON_GAMESTARTUP,         // 60
    RT_CON_WACKPLAYER,          // 61
    RT_CON_IFGAPZL,             // 62
    RT_CON_IFHITSPACE,          // 63
    RT_CON_IFOUTSIDE,           // 64
    RT_CON_IFMULTIPLAYER,       // 65
    RT_CON_OPERATE,             // 66
    RT_CON_IFINSPACE,           // 67
    RT_CON_DEBUG,               // 68
    RT_CON_ENDOFGAME,           // 69
    RT_CON_IFBULLETNEAR,        // 70
    RT_CON_IFRESPAWN,           // 71
    RT_CON_IFFLOORDISTL,        // 72
    RT_CON_IFCEILINGDISTL,      // 73
    RT_CON_SPRITEPAL,           // 74
    RT_CON_IFPINVENTORY,        // 75
    RT_CON_BETANAME,            // 76
    RT_CON_CACTOR,              // 77
    RT_CON_IFPHEALTHL,          // 78
    RT_CON_DEFINEQUOTE,         // 79
    RT_CON_QUOTE,               // 80
    RT_CON_IFINOUTERSPACE,      // 81
    RT_CON_IFNOTMOVING,         // 82
    RT_CON_RESPAWNHITAG,        // 83
    RT_CON_TIP,                 // 84
    RT_CON_IFSPRITEPAL,         // 85
    RT_CON_MONEY,               // 86
    RT_CON_SOUNDONCE,           // 87
    RT_CON_ADDKILLS,            // 88
    RT_CON_STOPSOUND,           // 89
    RT_CON_IFAWAYFROMWALL,      // 90
    RT_CON_IFCANSEETARGET,      // 91
    RT_CON_GLOBALSOUND,         // 92
    RT_CON_LOTSOFGLASS,         // 93
    RT_CON_IFGOTWEAPONCE,       // 94
    RT_CON_GETLASTPAL,          // 95
    RT_CON_PKICK,               // 96
    RT_CON_MIKESND,             // 97
    RT_CON_USERACTOR,           // 98
    RT_CON_SIZEAT,              // 99
    RT_CON_ADDSTRENGTH,         // 100
    RT_CON_CSTATOR,             // 101
    RT_CON_MAIL,                // 102
    RT_CON_PAPER,               // 103
    RT_CON_TOSSWEAPON,          // 104
    RT_CON_SLEEPTIME,           // 105
    RT_CON_NULLOP,              // 106
    RT_CON_DEFINEVOLUMENAME,    // 107
    RT_CON_DEFINESKILLNAME,     // 108
    RT_CON_IFNOSOUNDS,          // 109
    RT_CON_CLIPDIST,            // 110
    RT_CON_IFANGDIFFL,          // 111
};

#pragma pack(push, 1)

struct tileinfo_t {
    int32_t fileoff;
    int32_t waloff;
    int32_t picanm;
    int16_t sizx;
    int16_t sizy;
    uint16_t filesiz;
    int16_t dimx;
    int16_t dimy;
    uint16_t flags;
    int16_t tile;
    uint8_t pad[2];
};

struct boardinfo_t {
    uint32_t datastart;
    uint32_t dataend;
    uint32_t sectoroffset;
    uint32_t walloffset;
    uint32_t spriteoffset;
    uint32_t numsector;
    uint32_t numsprites;
    uint32_t numwall;
    uint32_t posx;
    uint32_t posy;
    uint32_t posz;
    uint32_t ang;
    float sky[6];
};

struct rt_sectortype {
    int32_t ceilingz;
    int32_t floorz;
    int16_t wallptr;
    int16_t wallnum;
    int16_t ceilingstat;
    int16_t floorstat;
    int16_t ceilingpicnum;
    int16_t ceilingheinum;
    int16_t floorpicnum;
    int16_t floorheinum;
    int16_t lotag;
    int16_t hitag;
    int16_t extra;
    int16_t floorvertexptr;
    int16_t ceilingvertexptr;
    int8_t ceilingshade;
    uint8_t ceilingpal;
    uint8_t ceilingxpanning;
    uint8_t ceilingypanning;
    int8_t floorshade;
    uint8_t floorpal;
    uint8_t floorxpanning;
    uint8_t floorypanning;
    uint8_t visibility;
    uint8_t floorvertexnum;
    uint8_t ceilingvertexnum;
    uint8_t pad2[3];
};

struct rt_walltype {
    int32_t x, y;
    int16_t point2;
    int16_t nextwall;
    int16_t nextsector;
    int16_t cstat;
    int16_t picnum;
    int16_t overpicnum;
    int16_t lotag;
    int16_t hitag;
    int16_t extra;
    int16_t sectnum;
    int8_t shade;
    uint8_t pal;
    uint8_t xrepeat;
    uint8_t yrepeat;
    uint8_t xpanning;
    uint8_t ypanning;
    uint8_t pad2[2];
};

struct rt_spritetype {
    int32_t x, y, z;
    int16_t cstat;
    int16_t picnum;
    int16_t sectnum;
    int16_t statnum;
    int16_t ang;
    int16_t owner;
    int16_t xvel;
    int16_t yvel;
    int16_t zvel;
    int16_t lotag;
    int16_t hitag;
    int16_t extra;
    int8_t shade;
    uint8_t pal;
    uint8_t clipdist;
    uint8_t xrepeat;
    uint8_t yrepeat;
    uint8_t xoffset;
    uint8_t yoffset;
    uint8_t filler;
};

struct rt_vertex_t {
    int16_t x, y, z, u, v;
};

#pragma pack(pop)
