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


int rt_gamestate = 2, rt_gamestatus = 0;

buildvfs_kfd rt_group = buildvfs_kfd_invalid;
static bool rt_group_init;

static boardinfo_t boardinfo[RT_BOARDNUM];

uint16_t rt_palette[RT_PALNUM][256];

static void RT_LoadPalette(void);

rt_vertex_t *rt_sectvtx;
rt_walltype *rt_wall;
rt_sectortype *rt_sector;

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
    static int boardOffset = 0x8f6f8;
    if (!rt_group_init)
        G_GameExit("RT_Init: No GRP initialized");

    paletteLoadFromDisk_replace = RT_LoadPalette;


    Blseek(rt_group, boardOffset, SEEK_SET);
    if (Bread(rt_group, boardinfo, sizeof(boardinfo)) != sizeof(boardinfo))
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

}

#define RT_QUOTES 131

int RT_PrepareScript(void)
{
    const int rt_scriptoffset = 0xa4500;
    const int rt_actoroffset = 0xb1784;
    const int rt_scriptsize = 0x34a1;
    int32_t *rt_script = (int32_t*)Xmalloc(rt_scriptsize*4);
    int16_t *rt_actorscrptr = (int16_t*)Xmalloc(6144*2);
    if (rt_script == nullptr)
    {
        initprintf("RT_PrepareScript: can't allocate memory\n");
        return 1;
    }

    Blseek(rt_group, rt_scriptoffset, SEEK_SET);
    if (Bread(rt_group, rt_script, rt_scriptsize*4) != rt_scriptsize*4)
    {
        initprintf("RT_PrepareScript: file read error\n");
        return 1;
    }

    Blseek(rt_group, rt_actoroffset, SEEK_SET);
    if (Bread(rt_group, rt_actorscrptr, 6144*2) != 6144*2)
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

    g_shrinkerRadius = 680;

    const int quote_offset = 0xb4784;
    const int quote_delta = 0xbba50- 0x80105e50;
    const int quote_count = 131;

    int32_t *quote_table = (int32_t*)Xmalloc(quote_count * 4);
    Blseek(rt_group, quote_offset, SEEK_SET);
    if (Bread(rt_group, quote_table, quote_count*4) != quote_count*4)
    {
        initprintf("RT_PrepareScript: file read error\n");
        return 1;
    }
    for (int i = 0; i < quote_count; i++)
    {
        int const offset = B_BIG32(quote_table[i]) + quote_delta;
        C_AllocQuote(i);
        Blseek(rt_group, offset, SEEK_SET);
        Bread(rt_group, apStrings[i], MAXQUOTELEN-1);
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

    return 0;
}

static void RT_LoadPalette(void)
{
    const int paletteOffset = 0xc0880;
    for (int p = 0; p < RT_PALNUM; p++)
    {
        Blseek(rt_group, paletteOffset + 520 * p + 8, SEEK_SET);
        if (Bread(rt_group, rt_palette[p], 512) != 512)
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
    auto board = &boardinfo[boardnum];
    uint32_t siz = board->dataend - board->datastart;
    char *boardbuf = (char*)Xmalloc(siz);
    Blseek(rt_group, board->datastart, SEEK_SET);
    if (Bread(rt_group, boardbuf, siz) != siz)
    {
        initprintf("RT_LoadBoard: file read error");
        return;
    }

    int numsprites = board->numsprites;
    numsectors = board->numsector;
    numwalls = board->numwall;

    rt_sky_color[0][0] = board->sky[0];
    rt_sky_color[1][0] = board->sky[3];
    rt_sky_color[0][1] = board->sky[1];
    rt_sky_color[1][1] = board->sky[4];
    rt_sky_color[0][2] = board->sky[2];
    rt_sky_color[1][2] = board->sky[5];

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
}

#endif
