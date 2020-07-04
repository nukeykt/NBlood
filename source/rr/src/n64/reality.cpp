// Copyright: 2020 Nuke.YKT, EDuke32 developers
// License: GPLv2

#ifdef USE_OPENGL
#include "build.h"
#include "colmatch.h"
#include "vfs.h"
#include "crc32.h"
#include "reality.h"
#include "../duke3d.h"
#include "../gamedef.h"


// int rt_gamestate = 2, rt_gamestatus = 0;

buildvfs_kfd rt_group = buildvfs_kfd_invalid;
static bool rt_group_init;

static boardinfo_t boardinfo[RT_BOARDNUM];

uint16_t rt_palette[RT_PALNUM][256];

static void RT_LoadPalette(void);

int rt_vtxnum;

rt_vertex_t *rt_sectvtx;
rt_walltype *rt_wall;
rt_sectortype *rt_sector;

int rt_boardnum;

char *rt_rom;

int rt_romcountry; // 0 - USA, 1 - EU

int rt_romoffset = 0;

int rt_levelnum;
// int rt_level7unlock, rt_level18unlock, rt_level28unlock, rt_level29unlock, rt_level33unlock;

struct {
    int usaoffset, euoffset;
} rt_offsettable[GO_MAXOFFSETS] = {
    0xa4500, 0xa4530,
    0xb1784, 0xb17b4,
    0x8f6f8, 0x8f728,
    0xb4784, 0xb47b4,
    0xbba50-0x80105e50, 0xbba80-0x80105e80,
    0xc0880, 0xc08b0,
    0x7bc6e0, 0x7bc710,
    0x7bd580, 0x7bd5b0,
    0x7bbfc0, 0x7bbff0,
    0x5fe770, 0x5fe7a0,
    0x60b330, 0x60b360,
    0xb4990, 0xb49c0,
    0xb4be8, 0xb4c18,
    0xb4e40, 0xb4e70,
    0xb5098, 0xb50c8,
    0x8a4a8, 0x8a4d8,
    0x90bf0, 0x90c20,
    0xc2270, 0xc22a0,
    0x3e9550, 0x3e9580,
    0x895b0, 0x895e0,
};

#define RTROMSIZE 8388608
buildvfs_kfd RT_InitGRP(const char *filename)
{
    auto grp = openfrompath(filename, BO_RDONLY|BO_BINARY, BS_IREAD);
    if (grp != buildvfs_kfd_invalid)
    {
        rt_group = grp;
        rt_rom = (char*)Xmalloc(RTROMSIZE);
        if (!rt_rom)
            return -1;
        int byteorder = 0;
        if (strstr(filename, ".n64"))
            byteorder = 1;
        if (strstr(filename, ".v64"))
            byteorder = 2;

        lseek(rt_group, 0, SEEK_SET);
        if (read(rt_group, rt_rom, RTROMSIZE) != RTROMSIZE)
        {
            Xfree(rt_rom);
            rt_rom = nullptr;
            initprintf("Error reading ROM file");
            return -1;
        }
        switch (byteorder)
        {
        case 0:
        default:
            break;
        case 1:
            for (int i = 0; i < RTROMSIZE / 2; i++)
            {
                uint16_t *ptr = (uint16_t*)&rt_rom[i * 2];
                *ptr = B_SWAP16(*ptr);
            }
            break;
        case 2:
            for (int i = 0; i < RTROMSIZE / 4; i++)
            {
                uint32_t *ptr = (uint32_t*)&rt_rom[i * 4];
                *ptr = B_SWAP32(*ptr);
            }
            break;
        }
        if (memcmp(&rt_rom[32], "DUKE NUKEM", 10) != 0)
        {
            Xfree(rt_rom);
            rt_rom = nullptr;
            initprintf("Invalid ROM file");
            return -1;
        }
        switch (rt_rom[62])
        {
        case 'E':
            rt_romcountry = 0;
            break;
        case 'P':
            rt_romcountry = 1;
            break;
        default:
            Xfree(rt_rom);
            rt_rom = nullptr;
            initprintf("Invalid ROM file");
            return -1;
        }
        rt_group_init = true;

        return rt_group;
    }

    return -1;
}

void RT_ROMSeek(int offset)
{
    if (offset >= 0 && offset <= RTROMSIZE)
        rt_romoffset = offset;
}

int RT_ROMRead(void *ptr, int count)
{
    count = min(count, RTROMSIZE - rt_romoffset);
    memcpy(ptr, &rt_rom[rt_romoffset], count);
    rt_romoffset += count;
    return count;
}

int RT_ROMGetOffset(int offset)
{
    if (offset < 0 || offset >= GO_MAXOFFSETS)
        return 0;
    switch (rt_romcountry)
    {
    case 0:
        return rt_offsettable[offset].usaoffset;
    case 1:
        return rt_offsettable[offset].euoffset;
    default:
        break;
    }
    return 0;
}

void RT_Init(void)
{
    int boardOffset = RT_ROMGetOffset(GO_BOARDOFFSET);
    if (!rt_group_init)
        G_GameExit("RT_Init: No GRP initialized");

    paletteLoadFromDisk_replace = RT_LoadPalette;


    RT_ROMSeek(boardOffset);
    if (RT_ROMRead(boardinfo, sizeof(boardinfo)) != sizeof(boardinfo))
    {
        initprintf("RT_Init: file read error");
        return;
    }
    for (auto &b : boardinfo)
    {
        b.datastart = B_BIG32(b.datastart);
        b.dataend = B_BIG32(b.dataend);
        b.sectoroffset = B_BIG32(b.sectoroffset);
        b.walloffset = B_BIG32(b.walloffset);
        b.spriteoffset = B_BIG32(b.spriteoffset);
        b.numsector = B_BIG32(b.numsector);
        b.numsprites = B_BIG32(b.numsprites);
        b.numwall = B_BIG32(b.numwall);
        b.posx = B_BIG32(b.posx);
        b.posy = B_BIG32(b.posy);
        b.posz = B_BIG32(b.posz);
        b.ang = B_BIG32(b.ang);
        for (int j = 0; j < 6; j++)
        {
            int t = *(int*)&b.sky[j];
            *(int*)&b.sky[j] = B_BIG32(t);
        }
    }

    duke64 = true;

    // FIXME
    PolymostProcessVoxels_Callback = RT_GLInit;

    RT_BuildAngleTable();

    static int const wchoice[14] = {
        13, 12, 2, 3, 7, 4, 10, 6, 14, 8, 5, 1, 9, 0
    };

    for (int i = 0; i < 14; i++)
    {
        g_player[0].wchoice[i] = wchoice[i];
    }

    globalflags |= GLOBAL_NO_GL_TILESHADES;
}

struct {
    int volume, level;
    const char *filename;
} rt_musicdefs[] = {
    MAXVOLUMES, 0, "grabbag.mid",
    MAXVOLUMES, 1, "briefing.mid",
    0, 0, "stalker.mid",
    0, 1, "dethtoll.mid",
    0, 2, "streets.mid",
    0, 3, "watrwld1.mid",
    0, 4, "thecall.mid",
    0, 5, "snake1.mid",
    0, 6, "snake1.mid",
    0, 7, "prepd.mid",
    0, 8, "futurmil.mid",
    0, 9, "storm.mid",
    0, 10, "gutwrnch.mid",
    0, 11, "robocrep.mid",
    0, 12, "stalag.mid",
    0, 13, "pizzed.mid",
    0, 14, "alienz.mid",
    0, 15, "xplasma.mid",
    0, 16, "alfredh.mid",
    0, 17, "alfredh.mid",
    0, 18, "intents.mid",
    0, 19, "inhiding.mid",
    0, 20, "fatcmdr.mid",
    0, 21, "names.mid",
    0, 22, "subway.mid",
    0, 23, "invader.mid",
    0, 24, "gotham.mid",
    0, 25, "233c.mid",
    0, 26, "lordofla.mid",
    0, 27, "urban.mid",
    0, 28, "restrict.mid",
    0, 29, "whomp.mid",
};

const char* rt_level_names[] = {
    "HOLLYWOOD HOLOCAUST",
    "GUN CRAZY",
    "DEATH ROW",
    "TOXIC DUMP",
    "LAUNCH FACILITY",
    "THE ABYSS",
    "BATTLELORD",
    "DUKE-BURGER",
    "SPACEPORT",
    "INCUBATOR",
    "WARP FACTOR",
    "FUSION STATION",
    "OCCUPIED TERRITORY",
    "TIBERIUS STATION",
    "LUNAR REACTOR",
    "DARK SIDE",
    "DREADNOUGHT",
    "OVERLORD",
    "LUNATIC FRINGE",
    "RAW MEAT",
    "BANK ROLL",
    "FLOOD ZONE",
    "L.A. RUMBLE",
    "MOVIE SET",
    "RABID TRANSIT",
    "FAHRENHEIT",
    "HOTEL HELL",
    "STADIUM",
    "AREA 51",
    "FREEWAY",
    "CASTLE DUKENSTEIN",
    "PIRACY",
    "SHAFT",
    "NOCTIS LABYRINTHUS"
};

#define RT_QUOTES 131

int RT_PrepareScript(void)
{
    const int rt_scriptoffset = RT_ROMGetOffset(GO_SCRIPTOFFSET);
    const int rt_actoroffset = RT_ROMGetOffset(GO_ACTOROFFSET);
    const int rt_scriptsize = 0x34a1;
    int32_t *rt_script = (int32_t*)Xmalloc(rt_scriptsize*4);
    int16_t *rt_actorscrptr = (int16_t*)Xmalloc(6144*2);
    if (rt_script == nullptr)
    {
        initprintf("RT_PrepareScript: can't allocate memory\n");
        return 1;
    }

    RT_ROMSeek(rt_scriptoffset);
    if (RT_ROMRead(rt_script, rt_scriptsize*4) != rt_scriptsize*4)
    {
        initprintf("RT_PrepareScript: file read error\n");
        return 1;
    }

    RT_ROMSeek(rt_actoroffset);
    if (RT_ROMRead(rt_actorscrptr, 6144*2) != 6144*2)
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
    {
        apScript[i] = B_BIG32(rt_script[i]);
        if (apScript[i] > 50000000)
        {
            apScript[i] = (apScript[i]-50000000)/4;
        }
    }
    Xfree(rt_script);

    for (int i = 0; i < 6144; i++)
    {
        int ptr = (uint16_t)B_BIG16(rt_actorscrptr[i]);
        if (ptr)
            g_tile[i].execPtr = &apScript[ptr];
    }

    for (int i = 0; i <= MAXVOLUMES; i++)
    {
        for (int j = 0; j < MAXLEVELS; j++)
        {
            g_mapInfo[i * MAXLEVELS + j].filename = (char*)Xcalloc(1, 1);
            if (i == 0 && j < ARRAY_SIZE(rt_level_names))
                g_mapInfo[i * MAXLEVELS + j].name = strdup(rt_level_names[j]);
            else
                g_mapInfo[i * MAXLEVELS + j].name = (char*)Xcalloc(1, 1);
        }
    }

    g_maxPlayerHealth = g_player[0].ps->max_player_health = g_player[0].ps->max_shield_amount = 100;

    g_player[0].ps->max_ammo_amount[1] = 192;
    g_player[0].ps->max_ammo_amount[2] = 50;
    g_player[0].ps->max_ammo_amount[3] = 400;
    g_player[0].ps->max_ammo_amount[4] = 48;
    g_player[0].ps->max_ammo_amount[5] = 50;
    g_player[0].ps->max_ammo_amount[6] = 66;
    g_player[0].ps->max_ammo_amount[7] = 99;
    g_player[0].ps->max_ammo_amount[8] = 50;
    g_player[0].ps->max_ammo_amount[9] = 10;
    g_player[0].ps->max_ammo_amount[10] = 99;
    g_player[0].ps->max_ammo_amount[11] = 0;
    g_player[0].ps->max_ammo_amount[12] = 36;
    g_player[0].ps->max_ammo_amount[13] = 20;
    g_player[0].ps->max_ammo_amount[14] = 25;

    const int quote_offset = RT_ROMGetOffset(GO_QUOTEOFFSET);
    const int quote_delta = RT_ROMGetOffset(GO_QUOTEDELTA);
    const int quote_count = 131;

    int32_t *quote_table = (int32_t*)Xmalloc(quote_count * 4);
    RT_ROMSeek(quote_offset);
    if (RT_ROMRead(quote_table, quote_count*4) != quote_count*4)
    {
        initprintf("RT_PrepareScript: file read error\n");
        return 1;
    }
    for (int i = 0; i < quote_count; i++)
    {
        int const offset = B_BIG32(quote_table[i]) + quote_delta;
        C_AllocQuote(i);
        RT_ROMSeek(offset);
        RT_ROMRead(apStrings[i], MAXQUOTELEN-1);
        for (int j = 0; j < MAXQUOTELEN; j++)
        {
            if (apStrings[i][j] == 0)
            {
                for (; j < MAXQUOTELEN; j++)
                    apStrings[i][j] = 0;
                break;
            }
        }
    }
    Bfree(quote_table);

    for (int i = 0; i < ARRAY_SIZE(rt_musicdefs); i++)
    {
        C_DefineMusic(rt_musicdefs[i].volume, rt_musicdefs[i].level, rt_musicdefs[i].filename);
    }

    return 0;
}

static void RT_LoadPalette(void)
{
    const int paletteOffset = RT_ROMGetOffset(GO_PALETTEOFFSET);
    for (int p = 0; p < RT_PALNUM; p++)
    {
        RT_ROMSeek(paletteOffset + 520 * p + 8);
        if (RT_ROMRead(rt_palette[p], 512) != 512)
        {
            initprintf("RT_LoadPalette: file read error");
            return;
        }
        for (int i = 0; i < 256; i++)
        {
            rt_palette[p][i] = B_BIG16(rt_palette[p][i]);
        }
    }
    for (int i = 0; i < 256; i++)
    {
        int t = rt_palette[0][i];
        int r = (t >> 11) & 31;
        int g = (t >> 6) & 31;
        int b = (t >> 1) & 31;
        r = (r << 3) + (r >> 2);
        g = (g << 3) + (g >> 2);
        b = (b << 3) + (b >> 2);
        palette[i*3+0] = r;
        palette[i*3+1] = g;
        palette[i*3+2] = b;
    }

    //for (int i = 1; i < MAXBASEPALS; i++)
    //{
    //    basepaltable[i] = palette;
    //}

    numshades = 32;

    palookup[0] = (char*)Xaligned_alloc(16, (numshades+1) * 256);
    for (int i = 0; i <= numshades; i++)
    {
        for (int j = 0; j < 256; j++)
        {
            palookup[0][i * 256 + j] = j;
        }
    }

    for (int i = 1; i < MAXPALOOKUPS; i++)
    {
        palookup[i] = palookup[0];
    }

    for (int i = 0; i < MAXPALOOKUPS; i++)
    {
        palookupfogfactor[i] = 1.f;
    }

    blendtable[0] = (char*)Xcalloc(1, 256 * 256);

    paletteInitClosestColorMap(palette);

    paletteloaded |= PALETTE_MAIN | PALETTE_SHADE | PALETTE_TRANSLUC;
}

void RT_LoadBoard(int boardnum)
{
    const uint32_t baseOffset = 0x8035a800;
    rt_boardnum = boardnum;
    auto board = &boardinfo[boardnum];
    uint32_t siz = board->dataend - board->datastart;
    char *boardbuf = (char*)Xmalloc(siz);
    RT_ROMSeek(board->datastart);
    if (RT_ROMRead(boardbuf, siz) != siz)
    {
        initprintf("RT_LoadBoard: file read error");
        return;
    }

    int numsprites = board->numsprites;
    numsectors = board->numsector;
    numwalls = board->numwall;

    rt_sky_color[0].x = board->sky[0];
    rt_sky_color[1].x = board->sky[3];
    rt_sky_color[0].y = board->sky[1];
    rt_sky_color[1].y = board->sky[4];
    rt_sky_color[0].z = board->sky[2];
    rt_sky_color[1].z = board->sky[5];

    RT_MS_Reset();
    
    initspritelists();

    Bmemset(show2dsector, 0, sizeof(show2dsector));
    Bmemset(show2dsprite, 0, sizeof(show2dsprite));
    Bmemset(show2dwall, 0, sizeof(show2dwall));
    Bmemset(editwall, 0, sizeof(editwall));
#ifdef USE_STRUCT_TRACKERS
    Bmemset(sectorchanged, 0, sizeof(sectorchanged));
    Bmemset(spritechanged, 0, sizeof(spritechanged));
    Bmemset(wallchanged, 0, sizeof(wallchanged));
#endif

#ifdef USE_OPENGL
    Polymost_prepare_loadboard();
#endif


    g_player[0].ps->pos.x = board->posx * 2;
    g_player[0].ps->pos.y = board->posy * 2;
    g_player[0].ps->pos.z = board->posz * 32;
    g_player[0].ps->cursectnum = 4; // ???
    g_player[0].ps->over_shoulder_on = 0;
    g_player[0].ps->q16ang = fix16_from_int(board->ang);

    if (rt_sector)
        Xfree(rt_sector);
    if (rt_wall)
        Xfree(rt_wall);

    rt_sector = (rt_sectortype*)Xmalloc(sizeof(rt_sectortype) * numsectors);
    rt_wall = (rt_walltype*)Xmalloc(sizeof(rt_walltype) * numwalls);
    rt_spritetype* rt_sprite = (rt_spritetype*)Xmalloc(sizeof(rt_spritetype) * numsprites);

    RNCDecompress(&boardbuf[board->sectoroffset-baseOffset], (char*)rt_sector);
    RNCDecompress(&boardbuf[board->walloffset-baseOffset], (char*)rt_wall);
    RNCDecompress(&boardbuf[board->spriteoffset-baseOffset], (char*)rt_sprite);

    int vtxnum = 0;
    
    for (int i = 0; i < numsectors; i++)
    {
        sector[i].ceilingz = B_BIG32(rt_sector[i].ceilingz);
        sector[i].floorz = B_BIG32(rt_sector[i].floorz);
        sector[i].wallptr = B_BIG16(rt_sector[i].wallptr);
        sector[i].wallnum = B_BIG16(rt_sector[i].wallnum);
        sector[i].ceilingstat = B_BIG16(rt_sector[i].ceilingstat);
        sector[i].floorstat = B_BIG16(rt_sector[i].floorstat);
        sector[i].ceilingpicnum = B_BIG16(rt_sector[i].ceilingpicnum);
        sector[i].ceilingheinum = B_BIG16(rt_sector[i].ceilingheinum);
        sector[i].floorpicnum = B_BIG16(rt_sector[i].floorpicnum);
        sector[i].floorheinum = B_BIG16(rt_sector[i].floorheinum);
        sector[i].lotag = B_BIG16(rt_sector[i].lotag);
        sector[i].hitag = B_BIG16(rt_sector[i].hitag);
        sector[i].extra = B_BIG16(rt_sector[i].extra);
        rt_sector[i].floorvertexptr = B_BIG16(rt_sector[i].floorvertexptr);
        rt_sector[i].ceilingvertexptr = B_BIG16(rt_sector[i].ceilingvertexptr);
        sector[i].ceilingshade = rt_sector[i].ceilingshade;
        sector[i].ceilingpal = rt_sector[i].ceilingpal;
        sector[i].ceilingxpanning = rt_sector[i].ceilingxpanning;
        sector[i].ceilingypanning = rt_sector[i].ceilingypanning;
        sector[i].floorshade = rt_sector[i].floorshade;
        sector[i].floorpal = rt_sector[i].floorpal;
        sector[i].floorxpanning = rt_sector[i].floorxpanning;
        sector[i].floorypanning = rt_sector[i].floorypanning;
        sector[i].visibility = rt_sector[i].visibility;
        sector[i].fogpal = 0;

        vtxnum += rt_sector[i].floorvertexnum + rt_sector[i].ceilingvertexnum;
    }
    
    for (int i = 0; i < numwalls; i++)
    {
        wall[i].x = B_BIG32(rt_wall[i].x);
        wall[i].y = B_BIG32(rt_wall[i].y);
        wall[i].point2 = B_BIG16(rt_wall[i].point2);
        wall[i].nextwall = B_BIG16(rt_wall[i].nextwall);
        wall[i].nextsector = B_BIG16(rt_wall[i].nextsector);
        wall[i].cstat = B_BIG16(rt_wall[i].cstat);
        wall[i].picnum = B_BIG16(rt_wall[i].picnum);
        wall[i].overpicnum = B_BIG16(rt_wall[i].overpicnum);
        wall[i].lotag = B_BIG16(rt_wall[i].lotag);
        wall[i].hitag = B_BIG16(rt_wall[i].hitag);
        wall[i].extra = B_BIG16(rt_wall[i].extra);
        wall[i].shade = rt_wall[i].shade;
        wall[i].pal = rt_wall[i].pal;
        wall[i].xrepeat = rt_wall[i].xrepeat;
        wall[i].yrepeat = rt_wall[i].yrepeat;
        wall[i].xpanning = rt_wall[i].xpanning;
        wall[i].ypanning = rt_wall[i].ypanning;
        rt_wall[i].sectnum = B_BIG16(rt_wall[i].sectnum);
    }
    
    for (int i = 0; i < numsprites; i++)
    {
        sprite[i].x = B_BIG32(rt_sprite[i].x);
        sprite[i].y = B_BIG32(rt_sprite[i].y);
        sprite[i].z = B_BIG32(rt_sprite[i].z);
        sprite[i].cstat = B_BIG16(rt_sprite[i].cstat);
        sprite[i].picnum = B_BIG16(rt_sprite[i].picnum);
        sprite[i].sectnum = B_BIG16(rt_sprite[i].sectnum);
        sprite[i].statnum = B_BIG16(rt_sprite[i].statnum);
        sprite[i].ang = B_BIG16(rt_sprite[i].ang);
        sprite[i].owner = B_BIG16(rt_sprite[i].owner);
        sprite[i].xvel = B_BIG16(rt_sprite[i].xvel);
        sprite[i].yvel = B_BIG16(rt_sprite[i].yvel);
        sprite[i].zvel = B_BIG16(rt_sprite[i].zvel);
        sprite[i].lotag = B_BIG16(rt_sprite[i].lotag);
        sprite[i].hitag = B_BIG16(rt_sprite[i].hitag);
        sprite[i].extra = B_BIG16(rt_sprite[i].extra);
        sprite[i].shade = rt_sprite[i].shade;
        sprite[i].pal = rt_sprite[i].pal;
        sprite[i].xrepeat = rt_sprite[i].xrepeat;
        sprite[i].yrepeat = rt_sprite[i].yrepeat;
        sprite[i].xoffset = rt_sprite[i].xoffset;
        sprite[i].yoffset = rt_sprite[i].yoffset;
        sprite[i].blend = 0;
    }
    for (int i = 0; i < numsprites; i++)
    {
        insertsprite(sprite[i].sectnum, sprite[i].statnum);
    }
    updatesector(g_player[0].ps->pos.x, g_player[0].ps->pos.y, &g_player[0].ps->cursectnum);
    g_loadedMapVersion = 7;
#ifdef YAX_ENABLE
    yax_update(1);
#endif

    if (rt_sectvtx)
        Xfree(rt_sectvtx);

    rt_vtxnum = vtxnum * 3;
    rt_sectvtx = (rt_vertex_t*)Xmalloc(sizeof(rt_vertex_t) * vtxnum * 3);
    RNCDecompress(boardbuf, (char*)rt_sectvtx);
    
    for (int i = 0; i < vtxnum * 3; i++)
    {
        rt_sectvtx[i].x = B_BIG16(rt_sectvtx[i].x);
        rt_sectvtx[i].y = B_BIG16(rt_sectvtx[i].y);
        rt_sectvtx[i].z = B_BIG16(rt_sectvtx[i].z);
        rt_sectvtx[i].u = B_BIG16(rt_sectvtx[i].u);
        rt_sectvtx[i].v = B_BIG16(rt_sectvtx[i].v);
    }

    Xfree(boardbuf);
    Xfree(rt_sprite);

    if (boardnum == 27)
        RT_LoadBOSS2MDL();

    memset(explosions, 0, sizeof(explosions));
    memset(smoke, 0, sizeof(smoke));
}

int RT_NextLevel(void)
{
#if 0
    if (ud.multimode > 1 && !ud.coop && dukematch_mode != 1)
    {
        if (rt_levelnum == 1 && rt_level7unlock)
            ud.secretlevel = 1;
        else
        {
            if ((rt_levelnum == 15 && rt_level18unlock)
                || (rt_levelnum == 23 && rt_level28unlock)
                || (rt_levelnum == 26 && rt_level29unlock)
                || (rt_levelnum == 32 && rt_level33unlock))
                ud.secretlevel = 1;
        }
    }
#endif
    int nextlevel = rt_levelnum;
    if (!ud.secretlevel)
    {
        switch (rt_levelnum)
        {
        case 6:
            nextlevel = 8;
            break;
        case 7:
            nextlevel = 2;
            break;
        case 17:
            nextlevel = 19;
            break;
        case 18:
            nextlevel = 16;
            break;
        case 27:
            nextlevel = 30;
            break;
        case 28:
            nextlevel = 24;
            break;
        case 29:
            nextlevel = 27;
            break;
        case 32:
            nextlevel = 0;
            break;
        case 33:
            nextlevel = 0;
            break;
        default:
            nextlevel++;
            break;
        }
    }
    else
    {
        switch (rt_levelnum)
        {
        case 1:
            // rt_level7unlock = 1;
            nextlevel = 7;
            break;
        case 7:
            nextlevel = 2;
            break;
        case 15:
            // rt_level18unlock = 1;
            nextlevel = 18;
            break;
        case 18:
            nextlevel = 16;
            break;
        case 23:
            // rt_level28unlock = 1;
            nextlevel = 28;
            break;
        case 26:
            // rt_level29unlock = 1;
            nextlevel = 29;
            break;
        case 28:
            nextlevel = 24;
            break;
        case 29:
            nextlevel = 27;
            break;
        case 32:
            // rt_level33unlock = 1;
            nextlevel = 33;
            break;
        case 33:
            nextlevel = 33;
            break;
        }
    }
    return nextlevel;
}

#endif
