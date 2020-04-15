// Copyright: 2020 Nuke.YKT, EDuke32 developers
// License: GPLv2

#ifdef USE_OPENGL
#include "build.h"
#include "vfs.h"
#include "crc32.h"
#include "duke3d.h"
#include "reality.h"
#include "gamedef.h"

buildvfs_kfd rt_group = buildvfs_kfd_invalid;
static bool rt_group_init;
static int32_t rt_scriptsize;

buildvfs_kfd RT_InitGRP(const char *filename)
{
    auto grp = openfrompath(filename, BO_RDONLY|BO_BINARY, BS_IREAD);
    if (grp != buildvfs_kfd_invalid)
    {
        rt_group_init = true;
        rt_group = grp;
    } 
    return rt_group;
}

void RT_Init(void)
{
    if (!rt_group_init)
        G_GameExit("RT_Init: No GRP initialized");

}

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

int RT_PrepareScript(void)
{
    const int rt_scriptoffset = 0xa4500;
    const int rt_scriptsize = 0x34a1;
    int32_t *rt_script = (int32_t*)Xmalloc(rt_scriptsize*4);
    if (rt_script == nullptr)
    {
        initprintf("RT_PrepareScript: can't allocate memory\n");
        return 1;
    }

    lseek(rt_group, rt_scriptoffset, SEEK_SET);
    if (read(rt_group, rt_script, rt_scriptsize*4) != rt_scriptsize*4)
    {
        initprintf("RT_PrepareScript: file read error\n");
        return 1;
    }

    g_scriptcrc = Bcrc32(NULL, 0, 0L);
    g_scriptcrc = Bcrc32(rt_script, rt_scriptsize*4, g_scriptcrc);

    g_scriptSize = rt_scriptsize;
    Bfree(apScript);
    apScript = (intptr_t *)Xcalloc(1, g_scriptSize * sizeof(intptr_t));
    for (int i = 0; i < rt_scriptsize; i++)
        apScript[i] = B_BIG32(rt_script[i]);
    return 0;
}

#endif
