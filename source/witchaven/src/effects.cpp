/****************************************************************************
            Witchaven

            WHFX.C

            Rafael Paiz

****************************************************************************/

#include "effects.h"
#include "player.h"
#include "sound.h"
#include "objects.h"
#include "witchaven.h"
#include "input.h"
#include "view.h"

#define LAVASIZ 128
#define LAVALOGSIZ 7
#define LAVAMAXDROPS 32

#define WATERSIZ 128
#define WATERLOGSIZ 7
#define WATERMAXDROPS 16 // bjd - this was 1 but contradicts the way arrays sized with this define were accessed

#define FLOORUNIT    16384L

int justwarpedfx = 0;
int lastbat = -1;

extern int justteleported;

extern short lavadrylandsector[32];
extern short lavadrylandcnt;


static char lavabakpic[(LAVASIZ+2)*(LAVASIZ+2)], lavainc[LAVASIZ];
static int lavanumdrops, lavanumframes;
static int lavadropx[LAVAMAXDROPS], lavadropy[LAVAMAXDROPS];
static int lavadropsiz[LAVAMAXDROPS], lavadropsizlookup[LAVAMAXDROPS];
static int lavaradx[32][128], lavarady[32][128], lavaradcnt[32];

static char waterbakpic[(WATERSIZ+2)*(WATERSIZ+2)], waterinc[WATERSIZ];
static int waternumdrops, waternumframes;
static int waterdropx[WATERMAXDROPS], waterdropy[WATERMAXDROPS];
static int waterdropsiz[WATERMAXDROPS], waterdropsizlookup[WATERMAXDROPS];
static int waterradx[32][128], waterrady[32][128], waterradcnt[32];

extern short revolvesector[], revolveang[], revolveclip[], revolvecnt;
extern int revolvex[16][16], revolvey[16][16];
extern int revolvepivotx[], revolvepivoty[];

extern short xpanningsectorlist[], xpanningsectorcnt;
extern short ypanningwalllist[], ypanningwallcnt;
extern short floorpanninglist[], floorpanningcnt;
extern short skypanlist[], skypancnt;
extern short crushsectorlist[], crushsectorcnt;
extern short crushsectoranim[], crushsectordone[];
extern short bobbingsectorcnt, bobbingsectorlist[];
extern short warpsectorlist[], warpsectorcnt;

char revolvesyncstat;
short revolvesyncang, revolvesyncrotang;
int revolvesyncx, revolvesyncy;

extern int mapon;


void initlava()
{
    for (int x = -16; x <= 16; x++)
    {
        for (int y = -16; y <= 16; y++)
        {
            int r = ksqrt(x * x + y * y);
            lavaradx[r][lavaradcnt[r]] = x;
            lavarady[r][lavaradcnt[r]] = y;
            lavaradcnt[r]++;
        }
    }

    for (int z = 0; z < 16; z++)
        lavadropsizlookup[z] = 8 / (ksqrt(z) + 1);

    for (int z = 0; z < LAVASIZ; z++)
        lavainc[z] = klabs((((z ^ 17) >> 4) & 7) - 4) + 12;

    lavanumdrops = 0;
    lavanumframes = 0;
}

void movelava(char* dapic)
{
    char dat, * ptr;
    int32_t x, y, z, zx, dalavadropsiz, dadropsizlookup;
    intptr_t offs, offs2;
    int32_t dalavax, dalavay;

    z = 3;
    if (lavanumdrops + z >= LAVAMAXDROPS)
        z = LAVAMAXDROPS - lavanumdrops - 1;

    while (z >= 0)
    {
        lavadropx[lavanumdrops] = (rand() & (LAVASIZ - 1));
        lavadropy[lavanumdrops] = (rand() & (LAVASIZ - 1));
        lavadropsiz[lavanumdrops] = 1;
        lavanumdrops++;
        z--;
    }

    z = lavanumdrops - 1;
    while (z >= 0)
    {
        dadropsizlookup = lavadropsizlookup[lavadropsiz[z]] * (((z & 1) << 1) - 1);
        dalavadropsiz = lavadropsiz[z];
        dalavax = lavadropx[z]; dalavay = lavadropy[z];
        for (zx = lavaradcnt[lavadropsiz[z]] - 1; zx >= 0; zx--)
        {
            offs = (((lavaradx[dalavadropsiz][zx] + dalavax) & (LAVASIZ - 1)) << LAVALOGSIZ);
            offs += ((lavarady[dalavadropsiz][zx] + dalavay) & (LAVASIZ - 1));
            dapic[offs] += dadropsizlookup;

            if (dapic[offs] < 192)
                dapic[offs] = 192;
        }

        lavadropsiz[z]++;
        if (lavadropsiz[z] > 10)
        {
            lavanumdrops--;
            lavadropx[z] = lavadropx[lavanumdrops];
            lavadropy[z] = lavadropy[lavanumdrops];
            lavadropsiz[z] = lavadropsiz[lavanumdrops];
        }
        z--;
    }

    //Back up dapic with 1 pixel extra on each boundary
    //(to prevent anding for wrap-around)
    offs = ((intptr_t)dapic);
    offs2 = (LAVASIZ + 2) + 1 + ((intptr_t)lavabakpic);
    for (x = 0; x < LAVASIZ; x++)
    {
        copybuf((char*)offs, (char*)offs2, LAVASIZ >> 2);
        offs += LAVASIZ;
        offs2 += LAVASIZ + 2;
    }
    for (y = 0; y < LAVASIZ; y++)
    {
        lavabakpic[y + 1] = dapic[y + ((LAVASIZ - 1) << LAVALOGSIZ)];
        lavabakpic[y + 1 + (LAVASIZ + 1) * (LAVASIZ + 2)] = dapic[y];
    }
    for (x = 0; x < LAVASIZ; x++)
    {
        lavabakpic[(x + 1) * (LAVASIZ + 2)] = dapic[(x << LAVALOGSIZ) + (LAVASIZ - 1)];
        lavabakpic[(x + 1) * (LAVASIZ + 2) + (LAVASIZ + 1)] = dapic[x << LAVALOGSIZ];
    }
    lavabakpic[0] = dapic[LAVASIZ * LAVASIZ - 1];
    lavabakpic[LAVASIZ + 1] = dapic[LAVASIZ * (LAVASIZ - 1)];
    lavabakpic[(LAVASIZ + 2) * (LAVASIZ + 1)] = dapic[LAVASIZ - 1];
    lavabakpic[(LAVASIZ + 2) * (LAVASIZ + 2) - 1] = dapic[0];

    for (z = (LAVASIZ + 2) * (LAVASIZ + 2) - 4; z >= 0; z -= 4)
    {
        lavabakpic[z + 0] &= 31;
        lavabakpic[z + 1] &= 31;
        lavabakpic[z + 2] &= 31;
        lavabakpic[z + 3] &= 31;
    }

    for (x = LAVASIZ - 1; x >= 0; x--)
    {
        offs = (x + 1) * (LAVASIZ + 2) + 1;
        ptr = (char*)((x << LAVALOGSIZ) + (intptr_t)dapic);

        zx = ((x + lavanumframes) & (LAVASIZ - 1));

        offs2 = LAVASIZ - 1;
        for (y = offs; y < offs + LAVASIZ; y++)
        {
            dat = lavainc[(offs2--) & zx];
            dat += lavabakpic[y - (LAVASIZ + 2) - 1];
            dat += lavabakpic[y - (LAVASIZ + 2)];
            dat += lavabakpic[y - (LAVASIZ + 2) + 1];
            dat += lavabakpic[y - 1];
            dat += lavabakpic[y + 1];
            dat += lavabakpic[y + (LAVASIZ + 2)];
            dat += lavabakpic[y + (LAVASIZ + 2) - 1];
            *ptr++ = (dat >> 3) + 192;
        }
    }

    lavanumframes++;
}

void initwater()
{
    for (int x = -16; x <= 16; x++)
    {
        for (int y = -16; y <= 16; y++)
        {
            int r = ksqrt(x * x + y * y);
            waterradx[r][waterradcnt[r]] = x;
            waterrady[r][waterradcnt[r]] = y;
            waterradcnt[r]++;
        }
    }

    for (int z = 0; z < WATERMAXDROPS; z++)
        waterdropsizlookup[z] = 8 / (ksqrt(z) + 1);

    for (int z = 0; z < WATERSIZ; z++)
        waterinc[z] = klabs((((z ^ 17) >> 4) & 7) - 4) + 12;

    waternumdrops = 0;
    waternumframes = 0;
}

void movewater(char* dapic)
{
    char dat, * ptr;
    int32_t x, y, z, zx, dawaterdropsiz, dadropsizlookup;
    intptr_t offs, offs2;
    int32_t dawaterx, dawatery;

    z = 3;
    if (waternumdrops + z >= WATERMAXDROPS)
        z = WATERMAXDROPS - waternumdrops - 1;

    while (z >= 0)
    {
        waterdropx[waternumdrops] = (rand() & (WATERSIZ - 1));
        waterdropy[waternumdrops] = (rand() & (WATERSIZ - 1));
        waterdropsiz[waternumdrops] = 1;
        waternumdrops++;
        z--;
    }

    z = waternumdrops - 1;

    while (z >= 0)
    {
        dadropsizlookup = waterdropsizlookup[waterdropsiz[z]] * (((z & 1) << 1) - 1);
        dawaterdropsiz = waterdropsiz[z];
        dawaterx = waterdropx[z]; dawatery = waterdropy[z];
        for (zx = waterradcnt[waterdropsiz[z]] - 1; zx >= 0; zx--)
        {
            offs = (((waterradx[dawaterdropsiz][zx] + dawaterx) & (WATERSIZ - 1)) << WATERLOGSIZ);
            offs += ((waterrady[dawaterdropsiz][zx] + dawatery) & (WATERSIZ - 1));
            dapic[offs] += dadropsizlookup;
            if (dapic[offs] < 224) dapic[offs] = 224;
        }

        waterdropsiz[z]++;
        if (waterdropsiz[z] > 10)
        {
            waternumdrops--;
            waterdropx[z] = waterdropx[waternumdrops];
            waterdropy[z] = waterdropy[waternumdrops];
            waterdropsiz[z] = waterdropsiz[waternumdrops];
        }
        z--;
    }

    offs = ((intptr_t)dapic);
    offs2 = (WATERSIZ + 2) + 1 + ((intptr_t)waterbakpic);

    for (x = 0; x < WATERSIZ; x++)
    {
        copybuf((char*)offs, (char*)offs2, WATERSIZ >> 2);
        offs += WATERSIZ;
        offs2 += WATERSIZ + 2;
    }
    for (y = 0; y < WATERSIZ; y++)
    {
        waterbakpic[y + 1] = dapic[y + ((WATERSIZ - 1) << WATERLOGSIZ)];
        waterbakpic[y + 1 + (WATERSIZ + 1) * (WATERSIZ + 2)] = dapic[y];
    }
    for (x = 0; x < WATERSIZ; x++)
    {
        waterbakpic[(x + 1) * (WATERSIZ + 2)] = dapic[(x << WATERLOGSIZ) + (WATERSIZ - 1)];
        waterbakpic[(x + 1) * (WATERSIZ + 2) + (WATERSIZ + 1)] = dapic[x << WATERLOGSIZ];
    }

    waterbakpic[0] = dapic[WATERSIZ * WATERSIZ - 1];
    waterbakpic[WATERSIZ + 1] = dapic[WATERSIZ * (WATERSIZ - 1)];
    waterbakpic[(WATERSIZ + 2) * (WATERSIZ + 1)] = dapic[WATERSIZ - 1];
    waterbakpic[(WATERSIZ + 2) * (WATERSIZ + 2) - 1] = dapic[0];

    for (z = (WATERSIZ + 2) * (WATERSIZ + 2) - 4; z >= 0; z -= 4)
    {
        waterbakpic[z + 0] &= 15;
        waterbakpic[z + 1] &= 15;
        waterbakpic[z + 2] &= 15;
        waterbakpic[z + 3] &= 15;
    }

    for (x = WATERSIZ - 1; x >= 0; x--)
    {
        offs = (x + 1) * (WATERSIZ + 2) + 1;
        ptr = (char*)((x << WATERLOGSIZ) + (intptr_t)dapic);

        zx = ((x + waternumframes) & (WATERSIZ - 1));

        offs2 = WATERSIZ - 1;
        for (y = offs; y < offs + WATERSIZ; y++)
        {
            dat = waterinc[(offs2--) & zx];
            dat += waterbakpic[y - (WATERSIZ + 2) - 1];
            dat += waterbakpic[y - (WATERSIZ + 2)];
            dat += waterbakpic[y - (WATERSIZ + 2) + 1];
            dat += waterbakpic[y - 1];
            dat += waterbakpic[y + 1];
            dat += waterbakpic[y + (WATERSIZ + 2) + 1];
            dat += waterbakpic[y + (WATERSIZ + 2)];
            dat += waterbakpic[y + (WATERSIZ + 2) - 1];
            *ptr++ = (dat >> 3) + 223;
        }
    }

    waternumframes++;
}

void skypanfx()
{
    for (int i = 0; i < skypancnt; i++) {
        sector[skypanlist[i]].ceilingxpanning = -((lockclock >> 2) & 255);
    }
}

void panningfx()
{
    Player* plr = &player[pyrn];

    //
    // n ne e se s sw w nw 
    // 0  1 2  3 4  5 6  7
    //
    for (int i = 0; i < floorpanningcnt; i++)
    {
        short whichdir = sector[floorpanninglist[i]].lotag - 80;

        switch (whichdir)
        {
            case 0:
            sector[floorpanninglist[i]].floorypanning = ((lockclock >> 2) & 255);
            break;
            case 1:
            sector[floorpanninglist[i]].floorxpanning = -((lockclock >> 2) & 255);
            sector[floorpanninglist[i]].floorypanning = ((lockclock >> 2) & 255);
            break;
            case 2:
            sector[floorpanninglist[i]].floorxpanning = -((lockclock >> 2) & 255);
            break;
            case 3:
            sector[floorpanninglist[i]].floorxpanning = -((lockclock >> 2) & 255);
            sector[floorpanninglist[i]].floorypanning = -((lockclock >> 2) & 255);
            break;
            case 4:
            sector[floorpanninglist[i]].floorypanning = -((lockclock >> 2) & 255);
            break;
            case 5:
            sector[floorpanninglist[i]].floorxpanning = ((lockclock >> 2) & 255);
            sector[floorpanninglist[i]].floorypanning = -((lockclock >> 2) & 255);
            break;
            case 6:
            sector[floorpanninglist[i]].floorxpanning = ((lockclock >> 2) & 255);
            break;
            case 7:
            sector[floorpanninglist[i]].floorxpanning = ((lockclock >> 2) & 255);
            sector[floorpanninglist[i]].floorypanning = ((lockclock >> 2) & 255);
            break;
            default:
            sector[floorpanninglist[i]].floorxpanning = 0;
            sector[floorpanninglist[i]].floorypanning = 0;
            break;
        }
    }

    for (int i = 0; i < xpanningsectorcnt; i++)
    {
        short dasector = xpanningsectorlist[i];
        short startwall = sector[dasector].wallptr;
        short endwall = startwall + sector[dasector].wallnum - 1;
        for (int s = startwall; s <= endwall; s++)
        {
            wall[s].xpanning = ((lockclock >> 2) & 255);
        }
    }

    for (int i = 0; i < ypanningwallcnt; i++) {
        wall[ypanningwalllist[i]].ypanning = ~(lockclock & 255);
    }
}

void crushingfx()
{
    int j, s;
    int32_t daz, goalz;

    for (int i = 0; i < crushsectorcnt; i++)
    {
        s = crushsectorlist[i];

        if (crushsectordone[s] == 1)
        {
            switch (crushsectoranim[s])
            {
                case 1:
                if ((j = getanimationgoal(&sector[s].floorz)) < 0)
                {
                    setanimation(&sector[s].floorz, sector[s].ceilingz, 64);
                    crushsectordone[s] = 2;
                }
                break;
                case 2:
                if ((j = getanimationgoal(&sector[s].ceilingz)) < 0)
                {
                    setanimation(&sector[s].ceilingz, sector[s].floorz, 64);
                    crushsectordone[s] = 2;
                }
                break;
                case 3:
                if ((j = getanimationgoal(&sector[s].ceilingz)) < 0)
                {
                    daz = (sector[s].ceilingz + sector[s].floorz) >> 1;
                    setanimation(&sector[s].ceilingz, daz, 64);
                    setanimation(&sector[s].floorz, daz, 64);
                    crushsectordone[s] = 2;
                }
                break;
            }
        }

        if (crushsectordone[s] == 2)
        {
            switch (crushsectoranim[s])
            {
                case 1:
                if ((j = getanimationgoal(&sector[s].floorz)) < 0)
                {
                    goalz = sector[nextsectorneighborz(s, sector[s].floorz, 1, 1)].floorz;
                    setanimation(&sector[s].floorz, goalz, 64);
                    crushsectordone[s] = 1;
                }
                break;
                case 2:
                if ((j = getanimationgoal(&sector[s].ceilingz)) < 0)
                {
                    goalz = sector[nextsectorneighborz(s, sector[s].ceilingz, -1, -1)].ceilingz;
                    setanimation(&sector[s].ceilingz, goalz, 64);
                    crushsectordone[s] = 1;
                }
                break;
                case 3:
                if ((j = getanimationgoal(&sector[s].ceilingz)) < 0)
                {
                    goalz = sector[nextsectorneighborz(s, sector[s].ceilingz, -1, -1)].ceilingz;
                    setanimation(&sector[s].ceilingz, goalz, 64);
                    goalz = sector[nextsectorneighborz(s, sector[s].floorz, 1, 1)].floorz;
                    setanimation(&sector[s].floorz, goalz, 64);
                    crushsectordone[s] = 1;
                }
                break;
            }
        }
    }
}

void revolvefx()
{
    int32_t dax, day;
    Player* plr = &player[pyrn];

    for (int i = 0; i < revolvecnt; i++)
    {
        short startwall = sector[revolvesector[i]].wallptr;
        short endwall = startwall + sector[revolvesector[i]].wallnum - 1;

        revolveang[i] = ((revolveang[i] + 2048 - (synctics << 1)) & kAngleMask);

        for (int k = startwall; k <= endwall; k++)
        {
            vec2_t pivot;
            pivot.x = revolvepivotx[i];
            pivot.y = revolvepivoty[i];

            vec2_t p;
            p.x = revolvex[i][k - startwall];
            p.y = revolvey[i][k - startwall];

            vec2_t p2;
            rotatepoint(pivot, p, revolveang[i], &p2);

            dax = p2.x;
            day = p2.y;

            dragpoint(k, dax, day, 0); // CHECKME - flags
        }

        if (plr->sector == revolvesector[i])
        {
            revolvesyncang = plr->ang;
            revolvesyncrotang = 0;
            revolvesyncx = plr->x;
            revolvesyncy = plr->y;
            revolvesyncrotang = ((revolvesyncrotang + 2048 - (synctics << 1)) & kAngleMask);

            vec2_t pivot;
            pivot.x = revolvepivotx[i];
            pivot.y = revolvepivoty[i];

            vec2_t p;
            p.x = revolvesyncx;
            p.y = revolvesyncy;

            vec2_t p2;
            p2.x = plr->x;
            p2.y = plr->y;

            rotatepoint(pivot, p, revolvesyncrotang, &p2);

            plr->x = p2.x;
            plr->y = p2.y;

            plr->ang = ((revolvesyncang + revolvesyncrotang) & kAngleMask);
        }
    }
}

void bobbingsector()
{
    for (int i = 0; i < bobbingsectorcnt; i++)
    {
        int dasector = bobbingsectorlist[i];

        sector[dasector].floorz += Sin(lockclock << 4) >> 6;
    }
}


#define NEXTLEVEL 1
#define ENDOFDEMO 2

void teleporter()
{
    short dasector;
    short startwall, endwall;
    int i, j;
    int s;
    short daang;

    Player* plr = &player[pyrn];

    for (i = 0; i < warpsectorcnt; i++)
    {
        dasector = warpsectorlist[i];
        j = ((lockclock & 127) >> 2);

        if (j >= 16) j = 31 - j;
        {
            sector[dasector].ceilingshade = j;
            sector[dasector].floorshade = j;
            startwall = sector[dasector].wallptr;
            endwall = startwall + sector[dasector].wallnum - 1;
            for (s = startwall; s <= endwall; s++)
                wall[s].shade = j;
        }
    }

    if (sector[plr->sector].lotag == 10)
    {
        if (plr->sector != plr->oldsector)
        {
            daang = (short)plr->ang;
            warpfxsprite(plr->spritenum);
            warp(&plr->x, &plr->y, &plr->z, &daang, &plr->sector);
            warpfxsprite(plr->spritenum);
            plr->ang = (int)daang;
            justwarpedfx = 48;
            //JSA PLUTO
            playsound_loc(S_WARP, plr->x, plr->y);

            vec3_t pos;
            pos.x = plr->x;
            pos.y = plr->y;
            pos.z = plr->z + (32 << 8);

            setsprite(plr->spritenum, &pos);
        }
    }

    if (sector[plr->sector].lotag == 4002)
    {
        if (plr->treasure[8] == 1)
        {
            plr->treasure[8] = 0;
            if (plr->sector != plr->oldsector)
            {
                switch (sector[plr->sector].hitag)
                {
                    case NEXTLEVEL:
                    justteleported = 1;
                    mapon++;
                    vel = 0;
                    angvel = 0;
                    svel = 0;
                    playsound_loc(S_CHAINDOOR1, plr->x, plr->y);
                    loadnewlevel(mapon);
                    warpfxsprite(plr->spritenum);
                    // cleans up the keys (ivorykey etc)
                    plr->treasure[14] = plr->treasure[15] = plr->treasure[16] = plr->treasure[17] = 0;
                    plr->treasure[4] = plr->treasure[5] = 0;
                    SND_CheckLoops();
                    break;
                    case ENDOFDEMO:
                    playsound_loc(S_THUNDER1, plr->x, plr->y);
                    justteleported = 1;
                    victory();
                    SND_CheckLoops();
                    break;
                }
            }
        }
        else
        {
            // player need pentagram to teleport
            StatusMessage(360, "ITEM NEEDED");
        }
    }
}

void warp(int32_t* x, int32_t* y, int32_t* z, short* daang, short* dasector)
{
    short startwall, endwall, s;
    int32_t i, dax, day;

    for (int i = 0; i < warpsectorcnt; i++)
    {
        if (sector[warpsectorlist[i]].hitag == sector[*dasector].hitag && warpsectorlist[i] != *dasector)
        {
            *dasector = warpsectorlist[i];
            break;
        }
    }

    startwall = sector[*dasector].wallptr;
    endwall = startwall + sector[*dasector].wallnum - 1;
    dax = 0, day = 0;

    for (s = startwall; s <= endwall; s++)
    {
        dax += wall[s].x, day += wall[s].y;
        if (wall[s].nextsector >= 0)
        {
            i = s;
        }
    }

    *x = dax / (endwall - startwall + 1);
    *y = day / (endwall - startwall + 1);
    *z = sector[*dasector].floorz - (32 << 8);
    updatesector(*x, *y, dasector);
    dax = ((wall[i].x + wall[wall[i].point2].x) >> 1);
    day = ((wall[i].y + wall[wall[i].point2].y) >> 1);
    *daang = getangle(dax - *x, day - *y);
}

void warpsprite(short spritenum)
{
    short dasectnum = sprite[spritenum].sectnum;

    warpfxsprite(spritenum);
    warp(&sprite[spritenum].x, &sprite[spritenum].y, &sprite[spritenum].z, &sprite[spritenum].ang, &dasectnum);

    warpfxsprite(spritenum);

    vec3_t pos;
    pos.x = sprite[spritenum].x;
    pos.y = sprite[spritenum].y;
    pos.z = sprite[spritenum].z;

    setsprite(spritenum, &pos);
}

#define GRATEROT   16
#define GRATEVERT  512

void ironbars()
{
    int32_t spritenum;
    int32_t ironbarmove;

    for (int i = 0; i < ironbarscnt; i++)
    {
        if (ironbarsdone[i] == 1)
        {
            spritenum = ironbarsanim[i];

            switch (sprite[ironbarsanim[i]].hitag)
            {
                case 1:
                {
                    sprite[ironbarsanim[i]].ang += synctics << 1;
                    if (sprite[ironbarsanim[i]].ang > 2047)
                        sprite[ironbarsanim[i]].ang -= 2047;
                    ironbarmove = ironbarsgoal[i] += synctics << 1;

                    vec3_t pos;
                    pos.x = sprite[spritenum].x;
                    pos.y = sprite[spritenum].y;
                    pos.z = sprite[spritenum].z;

                    setsprite(spritenum, &pos);
                    if (ironbarsgoal[i] > 512)
                    {
                        ironbarsgoal[i] = 0;
                        sprite[ironbarsanim[i]].hitag = 2;
                        ironbarsdone[i] = 0;
                    }
                    break;
                }
                case 2:
                {
                    sprite[ironbarsanim[i]].ang -= synctics << 1;
                    if (sprite[ironbarsanim[i]].ang < 0)
                        sprite[ironbarsanim[i]].ang += 2047;
                    ironbarsgoal[i] += synctics << 1;

                    vec3_t pos;
                    pos.x = sprite[spritenum].x;
                    pos.y = sprite[spritenum].y;
                    pos.z = sprite[spritenum].z;

                    setsprite(spritenum, &pos);
                    if (ironbarsgoal[i] > 512)
                    {
                        ironbarsgoal[i] = 0;
                        sprite[ironbarsanim[i]].hitag = 1;
                        ironbarsdone[i] = 0;
                    }
                    break;
                }
                case 3:
                {
                    sprite[ironbarsanim[i]].z -= synctics << 4;
                    if (sprite[ironbarsanim[i]].z < ironbarsgoal[i])
                    {
                        sprite[ironbarsanim[i]].z = ironbarsgoal[i];
                        sprite[ironbarsanim[i]].hitag = 4;
                        ironbarsdone[i] = 0;
                        ironbarsgoal[i] = sprite[ironbarsanim[i]].z + 6000;
                    }

                    vec3_t pos;
                    pos.x = sprite[spritenum].x;
                    pos.y = sprite[spritenum].y;
                    pos.z = sprite[spritenum].z;

                    setsprite(spritenum, &pos);
                    break;
                }
                case 4:
                {
                    sprite[ironbarsanim[i]].z += synctics << 4;
                    if (sprite[ironbarsanim[i]].z > ironbarsgoal[i])
                    {
                        sprite[ironbarsanim[i]].z = ironbarsgoal[i];
                        sprite[ironbarsanim[i]].hitag = 3;
                        ironbarsdone[i] = 0;
                        ironbarsgoal[i] = sprite[ironbarsanim[i]].z - 6000;
                    }

                    vec3_t pos;
                    pos.x = sprite[spritenum].x;
                    pos.y = sprite[spritenum].y;
                    pos.z = sprite[spritenum].z;

                    setsprite(spritenum, &pos);
                    break;
                }
            }
        }
    }
}

void sectorsounds()
{
    uint16_t sec;
    int index;

    if (!SoundMode)
        return;

    Player* plr = &player[pyrn];

    sec = sector[plr->sector].extra;

    if (sec)
    {
        if (sec & 32768)
        {
            // loop on/off sector
            if (sec & 1)
            { 
                // turn loop on if lsb is 1
                index = ((sec ^ 32769) >> 1);
                if (ambsoundarray[index].hsound == -1)
                    ambsoundarray[index].hsound = SND_PlaySound(ambsoundarray[index].soundnum, 0, 0, 0, -1);
            }
            else
            {
                // turn loop off if lsb is 0 and its playing
                index = ((sec ^ 32768) >> 1);
                if (ambsoundarray[index].hsound != -1)
                {
                    SND_StopLoop(ambsoundarray[index].hsound);
                    ambsoundarray[index].hsound = -1;
                }
            }
        }
        else
        {
            if (plr->z <= sector[plr->sector].floorz - (8 << 8))
                playsound_loc(sec, plr->x, plr->y);
        }
    }
}

int scarytime=-1;
int scarysize=0;

void scary()
{
    if (rand() > 32600 && rand() > 32600 && scarytime < 0)
    {
        scarytime = 180;
        scarysize = 30;
        SND_PlaySound(S_SCARYDUDE, 0, 0, 0, 0);
    }

    if (scarytime >= 0)
    {
        scarytime -= synctics << 1;
        scarysize += synctics << 1;
        if (scarytime > 140 && scarytime < 180)
            rotatesprite_fs(320 << 15, 200 << 15, scarysize << 9, 0, SCARY, 0, 0, 1 + 2);
        if (scarytime > 120 && scarytime < 139)
            rotatesprite_fs(320 << 15, 200 << 15, scarysize << 9, 0, SCARY + 1, 0, 0, 1 + 2);
        if (scarytime > 100 && scarytime < 119)
            rotatesprite_fs(320 << 15, 200 << 15, scarysize << 9, 0, SCARY + 2, 0, 0, 1 + 2);
        if (scarytime > 0 && scarytime < 99)
            rotatesprite_fs(320 << 15, 200 << 15, scarysize << 9, 0, SCARY + 3, 0, 0, 1 + 2);
    }
}

void dofx()
{
    lavadryland();
    scary();

    if (revolvecnt > 0)
        revolvefx();

    panningfx();
    teleporter();
    bobbingsector();

    if (ironbarscnt > 0)
        ironbars();

    if ((gotpic[ANILAVA >> 3] & (1 << (ANILAVA & 7))) > 0)
    {
        gotpic[ANILAVA >> 3] &= ~(1 << (ANILAVA & 7));
        if (waloff[ANILAVA] != -1)
        {
            movelava((char*)waloff[ANILAVA]);
        }
    }

    if ((gotpic[HEALTHWATER >> 3] & (1 << (HEALTHWATER & 7))) > 0)
    {
        gotpic[HEALTHWATER >> 3] &= ~(1 << (HEALTHWATER & 7));
        if (waloff[HEALTHWATER] != -1)
        {
            movewater((char*)waloff[HEALTHWATER]);
        }
    }

    thesplash();
    thunder();
    cracks();
}

int thunderflash;
int thundertime;

void thunder()
{
    if (thunderflash == 0)
    {
        if ((gotpic[SKY >> 3] & (1 << (SKY & 7))) > 0)
        {
            gotpic[SKY >> 3] &= ~(1 << (SKY & 7));
            if (waloff[SKY] != -1)
            {
                visibility = 1024;
                if (rand() > 32700)
                {
                    thunderflash = 1;
                    thundertime = 120;
                }
            }
        }
        else if ((gotpic[SKY2 >> 3] & (1 << (SKY2 & 7))) > 0)
        {
            gotpic[SKY2 >> 3] &= ~(1 << (SKY2 & 7));
            if (waloff[SKY2] != -1)
            {
                visibility = 768;
                if (rand() > 32700)
                {
                    thunderflash = 1;
                    thundertime = 120;
                }
            }
        }
        else if ((gotpic[SKY3 >> 3] & (1 << (SKY3 & 7))) > 0)
        {
            gotpic[SKY3 >> 3] &= ~(1 << (SKY3 & 7));
            if (waloff[SKY3] != -1)
            {
                visibility = 512;
                if (rand() > 32700)
                {
                    thunderflash = 1;
                    thundertime = 120;
                }
            }
        }
        else if ((gotpic[SKY4 >> 3] & (1 << (SKY4 & 7))) > 0)
        {
            gotpic[SKY4 >> 3] &= ~(1 << (SKY4 & 7));
            if (waloff[SKY4] != -1)
            {
                visibility = 512;
                if (rand() > 32700)
                {
                    thunderflash = 1;
                    thundertime = 120;
                }
            }
        }
        else if ((gotpic[SKY5 >> 3] & (1 << (SKY5 & 7))) > 0)
        {
            gotpic[SKY5 >> 3] &= ~(1 << (SKY5 & 7));
            if (waloff[SKY5] != -1)
            {
                visibility = 1024;
                if (rand() > 32700)
                {
                    thunderflash = 1;
                    thundertime = 120;
                }
            }
        }
        else if ((gotpic[SKY6 >> 3] & (1 << (SKY6 & 7))) > 0)
        {
            gotpic[SKY6 >> 3] &= ~(1 << (SKY6 & 7));
            if (waloff[SKY6] != -1)
            {
                visibility = 512;
                if (rand() > 32700)
                {
                    thunderflash = 1;
                    thundertime = 120;
                }
            }
        }
        else if ((gotpic[SKY7 >> 3] & (1 << (SKY7 & 7))) > 0)
        {
            gotpic[SKY7 >> 3] &= ~(1 << (SKY7 & 7));
            if (waloff[SKY7] != -1)
            {
                visibility = 512;
                if (rand() > 32700)
                {
                    thunderflash = 1;
                    thundertime = 120;
                }
            }
        }
        else if ((gotpic[SKY8 >> 3] & (1 << (SKY8 & 7))) > 0)
        {
            gotpic[SKY8 >> 3] &= ~(1 << (SKY8 & 7));
            if (waloff[SKY8] != -1)
            {
                visibility = 1024;
                if (rand() > 32700)
                {
                    thunderflash = 1;
                    thundertime = 120;
                }
            }
        }
        else if ((gotpic[SKY9 >> 3] & (1 << (SKY9 & 7))) > 0)
        {
            gotpic[SKY9 >> 3] &= ~(1 << (SKY9 & 7));
            if (waloff[SKY9] != -1)
            {
                visibility = 2048;
                if (rand() > 32700)
                {
                    thunderflash = 1;
                    thundertime = 120;
                }
            }
        }
        else if ((gotpic[SKY10 >> 3] & (1 << (SKY10 & 7))) > 0)
        {
            gotpic[SKY10 >> 3] &= ~(1 << (SKY10 & 7));
            if (waloff[SKY10] != -1)
            {
                visibility = 1024;
                if (rand() > 32700)
                {
                    thunderflash = 1;
                    thundertime = 120;
                }
            }
        }
        else
            visibility = 1024;
    }
    else
    {
        thundertime -= synctics;
        if (thundertime < 0)
        {
            thunderflash = 0;
            brightness = gbrightness;
            setbrightness(brightness);

            SND_Sound(S_THUNDER1 + (rand() % 4));
            visibility = 1024;
        }
    }

    if (thunderflash == 1)
    {
        if (waloff[SKY] != -1)
        {
            int val = rand() % 4;
            brightness += val;

            switch (val)
            {
                case 0:
                visibility = 2048;
                break;
                case 1:
                visibility = 1024;
                break;
                case 2:
                visibility = 512;
                break;
                case 3:
                visibility = 256;
                break;
                default:
                visibility = 4096;
                break;
            }

            if (brightness > 8) {
                brightness = 0;
            }

            setbrightness(brightness);
        }
    }
}

void thesplash()
{
    Player* plr = &player[pyrn];

    if (sector[plr->sector].floorpicnum == WATER ||
        sector[plr->sector].floorpicnum == LAVA ||
        sector[plr->sector].floorpicnum == SLIME)
    {
        if (plr->sector != plr->oldsector)
        {
            if (sector[plr->sector].floorpicnum == WATER || sector[plr->sector].floorpicnum == SLIME)
                makeasplash(SPLASHAROO, plr);
            else
                makeasplash(LAVASPLASH, plr);
        }
    }
}

void makeasplash(int picnum, Player* plr)
{
    int32_t j = insertsprite(plr->sector, MASPLASH);
    sprite[j].x = plr->x;
    sprite[j].y = plr->y;
    sprite[j].z = sector[plr->sector].floorz + (tilesiz[picnum].y << 8);
    sprite[j].cstat = 0;        //Hitscan does not hit other bullets
    sprite[j].picnum = picnum;
    sprite[j].shade = 0;
    sprite[j].pal = 0;
    sprite[j].xrepeat = 64;
    sprite[j].yrepeat = 64;
    sprite[j].owner = 0;
    sprite[j].clipdist = 16;
    sprite[j].lotag = 8;
    sprite[j].hitag = 0;

    switch (picnum)
    {
        case SPLASHAROO:
        playsound_loc(S_SPLASH1 + (krand() % 3), sprite[j].x, sprite[j].y);
        break;
        case LAVASPLASH:
        break;
    }

    movesprite(j, (Cos(sprite[j].ang) * synctics) << 3, (((int)sintable[sprite[j].ang]) * synctics) << 3, 0, 4 << 8, 4 << 8, 0);
}

void makemonstersplash(int picnum, int i)
{
    if (sprite[i].picnum == FISH)
        return;

    int32_t j = insertsprite(sprite[i].sectnum, MASPLASH);
    sprite[j].x = sprite[i].x;
    sprite[j].y = sprite[i].y;
    sprite[j].z = sector[sprite[i].sectnum].floorz + (tilesiz[picnum].y << 8);
    sprite[j].cstat = 0;        //Hitscan does not hit other bullets
    sprite[j].picnum = picnum;
    sprite[j].shade = 0;

    if (sector[sprite[i].sectnum].floorpal == 9)
        sprite[j].pal = 9;
    else
        sprite[j].pal = 0;

    sprite[j].xrepeat = 64;
    sprite[j].yrepeat = 64;
    sprite[j].owner = 0;
    sprite[j].clipdist = 16;
    sprite[j].lotag = 8;
    sprite[j].hitag = 0;

    switch (picnum)
    {
        case SPLASHAROO:
        if (rand() % 2)
        {
            if ((gotpic[WATER >> 3] & (1 << (WATER & 7))) > 0)
            {
                gotpic[WATER >> 3] &= ~(1 << (WATER & 7));
                if (waloff[WATER] != -1)
                {
                    if (rand() % 2)
                    {
                        playsound_loc(S_SPLASH1 + (krand() % 3), sprite[j].x, sprite[j].y);
                    }
                }
            }
        }
        if (rand() % 2)
        {
            if ((gotpic[SLIME >> 3] & (1 << (SLIME & 7))) > 0)
            {
                gotpic[SLIME >> 3] &= ~(1 << (SLIME & 7));
                if (waloff[SLIME] != -1)
                {
                    if (rand() % 2)
                    {
                        playsound_loc(S_SPLASH1 + (krand() % 3), sprite[j].x, sprite[j].y);
                    }
                }
            }
        }
        break;
        case LAVASPLASH:
        break;
    }
}

void bats(short k)
{
    int32_t j = insertsprite(sprite[k].sectnum, FLOCK);
    sprite[j].x = sprite[k].x;
    sprite[j].y = sprite[k].y;
    sprite[j].z = sprite[k].z;
    sprite[j].cstat = 0;
    sprite[j].picnum = BAT;
    sprite[j].shade = 0;
    sprite[j].xrepeat = 64;
    sprite[j].yrepeat = 64;
    sprite[j].ang = Sin(sprite[k].ang + ((krand() & 128) - 256));
    sprite[j].owner = k;
    sprite[j].clipdist = 16;
    sprite[j].lotag = 128;
    sprite[j].hitag = k;
    sprite[j].extra = 0;

    newstatus(j, FLOCK);

    if (sprite[k].extra == 1)
        lastbat = j;
}

void cracks()
{
    short datag;
    int32_t daz;
    int j, k;

    Player* plr = &player[pyrn];

    datag = sector[plr->sector].lotag;

    if (floorpanningcnt < 64)
    {
        if (datag >= 3500 && datag <= 3599)
        {
            sector[plr->sector].hitag = 0;
            daz = sector[plr->sector].floorz + (1024 * (sector[plr->sector].lotag - 3500));
            if ((j = setanimation(&sector[plr->sector].floorz, daz, 32)) >= 0)
            {
                sector[plr->sector].floorpicnum = LAVA1;
                sector[plr->sector].floorshade = -25;
                SND_PlaySound(S_CRACKING, 0, 0, 0, 0);
            }
            sector[plr->sector].lotag = 80;
            floorpanninglist[floorpanningcnt++] = plr->sector;
        }
    }

    if (datag >= 5100 && datag <= 5199)
    {
        sector[plr->sector].hitag = 0;

        daz = sector[plr->sector].floorz + (1024 * (sector[plr->sector].lotag - 5100));
        sector[plr->sector].lotag = 0;
    }

    if (datag >= 5200 && datag <= 5299)
    {
        sector[plr->sector].hitag = 0;

        daz = sector[plr->sector].floorz + (1024 * (sector[plr->sector].lotag - 5200));
        sector[plr->sector].lotag = 0;
    }

    if (datag == 3001)
    {
        sector[plr->sector].lotag = 0;

        for (k = 0; k < MAXSPRITES; k++)
        {
            if (sector[plr->sector].hitag == sprite[k].hitag)
            {
                sprite[k].lotag = 36;
                sprite[k].zvel = (krand() & 1024) + 512;
                newstatus(k, SHOVE);
            }
        }
    }
}

void lavadryland()
{
    short k;
    short s;
    int32_t daz;

    Player* plr = &player[pyrn];

    for (k = 0; k<lavadrylandcnt; k++)
    {
        s = lavadrylandsector[k];

        if (plr->sector==s&&sector[s].lotag>0)
        {
            sector[s].hitag = 0;

            switch (sector[s].floorpicnum)
            {
                case LAVA:
                case ANILAVA:
                case LAVA1:
                sector[s].floorpicnum = COOLLAVA;
                break;
                case SLIME:
                sector[s].floorpicnum = DRYSLIME;
                break;
                case WATER:
                case HEALTHWATER:
                sector[s].floorpicnum = DRYWATER;
                break;
                case LAVA2:
                sector[s].floorpicnum = COOLLAVA2;
                break;
            }

            daz = sector[s].floorz-(1024*(sector[s].lotag-900));

            sector[s].lotag = 0;
        }
    }
}

void warpfxsprite(int s)
{
    short daang;
    Player* plr = &player[pyrn];

    int32_t j = insertsprite(sprite[s].sectnum, WARPFX);

    sprite[j].x = sprite[s].x;
    sprite[j].y = sprite[s].y;
    sprite[j].z = sprite[s].z-(32<<8);
    sprite[j].cstat = 0;
    sprite[j].picnum = ANNIHILATE;

    if (s == plr->spritenum)
    {
        daang = plr->ang;
        sprite[j].ang = daang;
    }
    else
    {
        daang = sprite[s].ang;
        sprite[j].ang = daang;
    }

    sprite[j].xrepeat = 48;
    sprite[j].yrepeat = 48;
    sprite[j].clipdist = 16;
    sprite[j].extra = 0;
    sprite[j].shade = -31;
    sprite[j].xvel = ((krand()&256)-128);
    sprite[j].yvel = ((krand()&256)-128);
    sprite[j].zvel = ((krand()&256)-128);
    sprite[j].owner = s;
    sprite[j].lotag = 12;
    sprite[j].hitag = 0;
    sprite[j].pal = 0;

    int32_t daz = ((((int)sprite[j].zvel)*synctics)>>3);

    movesprite(j, (Cos(daang)*synctics)<<3, (Sin(daang)*synctics)<<3, daz, 4<<8, 4<<8, 1);
}
