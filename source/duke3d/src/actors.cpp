//-------------------------------------------------------------------------
/*
Copyright (C) 2016 EDuke32 developers and contributors

This file is part of EDuke32.

EDuke32 is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License version 2
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
//-------------------------------------------------------------------------

#define actors_c_

#ifndef EDUKE32_STANDALONE
#include <map>
#endif

#include "colmatch.h"
#include "duke3d.h"
#include "input.h"
#include "microprofile.h"
#include "screens.h"

#if KRANDDEBUG
# define ACTOR_STATIC
#else
# define ACTOR_STATIC static
#endif

uint8_t g_radiusDmgStatnums[bitmap_size(MAXSTATUS)];

#define DELETE_SPRITE_AND_CONTINUE(KX) do { A_DeleteSprite(KX); goto next_sprite; } while (0)

int32_t otherp;

int G_SetInterpolation(int32_t *const posptr)
{
    if (g_interpolationCnt >= MAXINTERPOLATIONS)
        return 1;

    for (bssize_t i = 0; i < g_interpolationCnt; ++i)
        if (curipos[i] == posptr)
            return 0;

    curipos[g_interpolationCnt] = posptr;
    oldipos[g_interpolationCnt] = *posptr;
    g_interpolationCnt++;
    return 0;
}

void G_StopInterpolation(const int32_t * const posptr)
{
    for (bssize_t i = 0; i < g_interpolationCnt; ++i)
        if (curipos[i] == posptr)
        {
            g_interpolationCnt--;
            oldipos[i] = oldipos[g_interpolationCnt];
            bakipos[i] = bakipos[g_interpolationCnt];
            curipos[i] = curipos[g_interpolationCnt];
        }
}

void G_DoInterpolations(int smoothRatio)
{
    if (g_interpolationLock++)
        return;

    int32_t ndelta = 0;

    for (bssize_t i = 0, j = 0; i < g_interpolationCnt; ++i)
    {
        int32_t const odelta = ndelta;
        bakipos[i] = *curipos[i];
        ndelta = (*curipos[i]) - oldipos[i];
        if (odelta != ndelta)
            j = mulscale16(ndelta, smoothRatio);
        *curipos[i] = oldipos[i] + j;
    }
}

void G_DoConveyorInterp(int smoothratio)
{
    for (int SPRITES_OF(STAT_EFFECTOR, i))
    {
        auto s = &sprite[i];
        auto a = &actor[i];

        if (s->picnum != SECTOREFFECTOR || a->t_data[4])
            continue;

        auto sect = &sector[s->sectnum];

        if ((int)(s->lotag == SE_24_CONVEYOR) | (int)(s->lotag == SE_34_CONVEYOR2))
            sect->floorxpanning -= (uint8_t)mulscale16(65536 - smoothratio, sect->floorxpanning - (sect->floorxpanning - (s->yvel >> 7)));
    }
}

void G_ResetConveyorInterp(void)
{
    for (int SPRITES_OF(STAT_EFFECTOR, i))
    {
        auto s = &sprite[i];
        auto a = &actor[i];

        if (s->picnum != SECTOREFFECTOR || a->t_data[4])
            continue;

        auto sect = &sector[s->sectnum];

        if ((int)(s->lotag == SE_24_CONVEYOR) | (int)(s->lotag == SE_34_CONVEYOR2))
            sect->floorxpanning = (uint8_t)actor[i].bpos.x;
    }
}

void G_ClearCameraView(DukePlayer_t *ps)
{
    ps->newowner = -1;
    ps->pos = ps->opos;
    ps->q16ang = ps->oq16ang;

    updatesector(ps->pos.x, ps->pos.y, &ps->cursectnum);
    P_UpdateScreenPal(ps);

    for (bssize_t SPRITES_OF(STAT_ACTOR, k))
        if (sprite[k].picnum==CAMERA1)
            sprite[k].yvel = 0;
}

void A_RadiusDamageObject_Internal(int const spriteNum, int const otherSprite, int const blastRadius, int spriteDist,
                                   int const zOffset, int const dmg1, int dmg2, int dmg3, int dmg4)
{
    auto const pSprite = (uspriteptr_t)&sprite[spriteNum];
    auto const pOther  = &sprite[otherSprite];

#ifndef EDUKE32_STANDALONE
    if (WORLDTOUR && pSprite->picnum == FLAMETHROWERFLAME)
    {
        // enemies in WT don't damage other enemies of the same type with FLAMETHROWERFLAME
        if (sprite[pSprite->owner].picnum == pOther->picnum && pOther->picnum != APLAYER)
            return;
    }
#endif

    // DEFAULT, ZOMBIEACTOR, MISC
    if (pOther->statnum == STAT_DEFAULT || pOther->statnum == STAT_ZOMBIEACTOR || pOther->statnum == STAT_MISC
#ifndef EDUKE32_STANDALONE
        || (!FURY && AFLAMABLE(pOther->picnum))
#endif
        )
    {
#ifndef EDUKE32_STANDALONE
        if (pSprite->picnum != SHRINKSPARK || (pOther->cstat&257))
#endif
        {
            if (A_CheckEnemySprite(pOther) && !cansee(pOther->x, pOther->y, pOther->z+zOffset, pOther->sectnum, pSprite->x, pSprite->y, pSprite->z+zOffset, pSprite->sectnum))
                return;

#ifndef EDUKE32_STANDALONE
            if (!FURY)
                A_DamageObject_Duke3D(otherSprite, spriteNum);
            else
#endif
                A_DamageObject_Generic(otherSprite, spriteNum);
        }
    }
    else if (pOther->extra >= 0 && (uspriteptr_t)pOther != pSprite && ((pOther->cstat & 257) ||
#ifndef EDUKE32_STANDALONE
        pOther->picnum == TRIPBOMB || pOther->picnum == QUEBALL || pOther->picnum == STRIPEBALL || pOther->picnum == DUKELYINGDEAD ||
#endif
        A_CheckEnemySprite(pOther)))
    {
#ifndef EDUKE32_STANDALONE
        if ((pSprite->picnum == SHRINKSPARK && pOther->picnum != SHARK && (otherSprite == pSprite->owner || pOther->xrepeat < 24))
            || (pSprite->picnum == MORTER && otherSprite == pSprite->owner))
            return;
#endif
        if (spriteDist >= blastRadius || !cansee(pOther->x, pOther->y, pOther->z - ZOFFSET3, pOther->sectnum,
                                                 pSprite->x, pSprite->y, pSprite->z - ZOFFSET4, pSprite->sectnum))
            return;

        if (A_CheckSpriteFlags(otherSprite, SFLAG_DAMAGEEVENT))
            if (VM_OnEventWithReturn(EVENT_DAMAGESPRITE, spriteNum, -1, otherSprite) < 0)
                return;

        auto &dmgActor = actor[otherSprite];

        dmgActor.htang = getangle(pOther->x - pSprite->x, pOther->y - pSprite->y);

        if ((pOther->extra > 0 && ((A_CheckSpriteFlags(spriteNum, SFLAG_PROJECTILE) && SpriteProjectile[spriteNum].workslike & PROJECTILE_RADIUS_PICNUM)
#ifndef EDUKE32_STANDALONE
            || pSprite->picnum == RPG
#endif
            ))
#ifndef EDUKE32_STANDALONE
            || (pSprite->picnum == SHRINKSPARK)
#endif
            )
            dmgActor.htpicnum = pSprite->picnum;
#ifndef EDUKE32_STANDALONE
        else if (WORLDTOUR && (pSprite->picnum == FLAMETHROWERFLAME || pSprite->picnum == LAVAPOOL
                 || (pSprite->picnum == FIREBALL && sprite[pSprite->owner].picnum == APLAYER)))
            dmgActor.htpicnum = FLAMETHROWERFLAME;
#endif
        else
            dmgActor.htpicnum = RADIUSEXPLOSION;

#ifndef EDUKE32_STANDALONE
        if (pSprite->picnum != SHRINKSPARK && (!WORLDTOUR || pSprite->picnum != LAVAPOOL))
#endif
        {
            // this is really weird
            int const k = blastRadius/3;
            int dmgBase = 0, dmgFuzz = 1;

            if (spriteDist < k)
                dmgBase = dmg3, dmgFuzz = dmg4;
            else if (spriteDist < k*2)
                dmgBase = dmg2, dmgFuzz = dmg3;
            else if (spriteDist < blastRadius)
                dmgBase = dmg1, dmgFuzz = dmg2;

            if (dmgBase == dmgFuzz)
                ++dmgFuzz;

            int dmgTotal = dmgBase + (krand()%(dmgFuzz-dmgBase));
            if ((duke3d_globalflags & DUKE3D_GLOBAL_ADDITIVE_HITRADIUS)
                    | (SpriteProjectile[spriteNum].workslike & PROJECTILE_HITRADIUS_ADDITIVE))
                dmgActor.htextra += dmgTotal;
            else
                dmgActor.htextra = dmgTotal;

            if (!A_CheckSpriteFlags(otherSprite, SFLAG_NODAMAGEPUSH))
            {
                if (pOther->xvel < 0) pOther->xvel = 0;
                pOther->xvel += (pSprite->extra<<2);
            }

            if (A_CheckSpriteFlags(otherSprite, SFLAG_DAMAGEEVENT))
                VM_OnEventWithReturn(EVENT_POSTDAMAGESPRITE, spriteNum, -1, otherSprite);

#ifndef EDUKE32_STANDALONE
            if (!FURY)
            {
                switch (tileGetMapping(pOther->picnum))
                {
                    case PODFEM1__:
                    case FEM1__:
                    case FEM2__:
                    case FEM3__:
                    case FEM4__:
                    case FEM5__:
                    case FEM6__:
                    case FEM7__:
                    case FEM8__:
                    case FEM9__:
                    case FEM10__:
                    case STATUE__:
                    case STATUEFLASH__:
                    case SPACEMARINE__:
                    case QUEBALL__:
                    case STRIPEBALL__:
                        A_DamageObject_Duke3D(otherSprite, spriteNum);
                        break;
                }
            }
#endif
        }
#ifndef EDUKE32_STANDALONE
        else if (!FURY && pSprite->extra == 0) dmgActor.htextra = 0;
#endif

        if (pOther->picnum != RADIUSEXPLOSION &&
            pSprite->owner >= 0 && sprite[pSprite->owner].statnum < MAXSTATUS)
        {
            if (pOther->picnum == APLAYER)
            {
                auto pPlayer = g_player[P_GetP(pOther)].ps;

                if (pPlayer->newowner >= 0)
                    G_ClearCameraView(pPlayer);
            }

            dmgActor.htowner = pSprite->owner;
        }
    }
}

#define MAXDAMAGESECTORS 128

void A_RadiusDamage(int const spriteNum, int const blastRadius, int const dmg1, int const dmg2, int const dmg3, int const dmg4)
{
    // Allow checking for radius damage in EVENT_DAMAGE(SPRITE/WALL/FLOOR/CEILING) events.
    decltype(ud.returnvar) const parms = { blastRadius, dmg1, dmg2, dmg3, dmg4 };
    Bmemcpy(ud.returnvar, parms, sizeof(parms));

    auto const pSprite = (uspriteptr_t)&sprite[spriteNum];

    int16_t numSectors, sectorList[MAXDAMAGESECTORS];
    uint8_t * const sectorMap = (uint8_t *)Balloca(bitmap_size(numsectors));
    bfirst_search_init(sectorList, sectorMap, &numSectors, numsectors, pSprite->sectnum);

#ifndef EDUKE32_STANDALONE
    int wallDamage = true;

    // rockets from the Devastator skip propagating damage to other sectors
    if (!FURY && (pSprite->picnum == RPG && pSprite->xrepeat < 11))
        wallDamage = false;
#endif

    int const forceFromRadiusDamage = max<int>((blastRadius * dmg4) - min<int>(UINT16_MAX, dist(pSprite, &g_player[myconnectindex].ps->pos) << 3), 0);
    I_AddForceFeedback(forceFromRadiusDamage, forceFromRadiusDamage, dmg3);

    auto wallTouched = (uint8_t *)Balloca(bitmap_size(numwalls));
    Bmemset(wallTouched, 0, bitmap_size(numwalls));

    auto wallCanSee = (uint8_t *)Balloca(bitmap_size(numwalls));
    Bmemset(wallCanSee, 0, bitmap_size(numwalls));

    for (int sectorCount=0; sectorCount < numSectors; ++sectorCount)
    {
        int const   sectorNum  = sectorList[sectorCount];
        auto const &listSector = sector[sectorNum];

        vec2_t  closest  = {};
        int32_t distance = INT32_MAX;

        int const startWall = listSector.wallptr;
        int const endWall   = listSector.wallnum + startWall;

        int w = startWall;

        for (auto pWall = (uwallptr_t)&wall[startWall]; w < endWall; ++w, ++pWall)
        {
            vec2_t  p        = pSprite->xy;
            int32_t walldist = blastRadius - 1;

            if (!bitmap_test(wallTouched, w))
                walldist = getwalldist(p, w, &p);

            if (walldist < blastRadius)
            {
                if (walldist < distance)
                {
                    distance = walldist;
                    closest  = p;
                }

                int16_t aSector = sectorNum;
                vec3_t  vect    = { (((pWall->x + wall[pWall->point2].x) >> 1) + pSprite->x) >> 1,
                                    (((pWall->y + wall[pWall->point2].y) >> 1) + pSprite->y) >> 1, pSprite->z };

                updatesector(vect.x, vect.y, &aSector);

                if (aSector == -1)
                {
                    vect.xy = p;
                    aSector   = sectorNum;
                }

                bitmap_set(wallTouched, w);

                if (pWall->nextwall != -1)
                    bitmap_set(wallTouched, pWall->nextwall);

                if (bitmap_test(wallCanSee, w) || cansee(vect.x, vect.y, vect.z, aSector, pSprite->x, pSprite->y, pSprite->z, pSprite->sectnum))
                {
                    bitmap_set(wallCanSee, w);

                    if (pWall->nextwall != -1)
                        bitmap_set(wallCanSee, pWall->nextwall);

#ifndef EDUKE32_STANDALONE
                    if (wallDamage)
#endif
                        A_DamageWall_Internal(spriteNum, w, { p.x, p.y, pSprite->z }, pSprite->picnum);
                }

                int const nextSector = pWall->nextsector;

                if (nextSector >= 0)
                    bfirst_search_try(sectorList, sectorMap, &numSectors, nextSector);

                if (numSectors == MAXDAMAGESECTORS)
                {
                    LOG_F(WARNING, "Sprite %d tried to damage more than %d sectors!", spriteNum, MAXDAMAGESECTORS);
                    goto wallsfinished;
                }
            }
        }

        if (distance >= blastRadius)
            continue;

        int32_t floorZ, ceilZ;
        getzsofslope(sectorNum, closest.x, closest.y, &ceilZ, &floorZ);

        if (((ceilZ - pSprite->z) >> 8) < blastRadius)
            Sect_DamageCeiling_Internal(spriteNum, sectorNum);

        if (((pSprite->z - floorZ) >> 8) < blastRadius)
            Sect_DamageFloor_Internal(spriteNum, sectorNum);
    }

wallsfinished:
    int const randomZOffset = -ZOFFSET2 + (krand()&(ZOFFSET5-1));

    for (int sectorCount=0; sectorCount < numSectors; ++sectorCount)
    {
        int damageSprite = headspritesect[sectorList[sectorCount]];

        while (damageSprite >= 0)
        {
            int const nextSprite = nextspritesect[damageSprite];
            auto      pDamage    = (uspriteptr_t)&sprite[damageSprite];

            if (pDamage != pSprite && bitmap_test(g_radiusDmgStatnums, pDamage->statnum))
            {
                int spriteDist = dist(pSprite, pDamage);

                if (pDamage->picnum == APLAYER)
                {
                    int const  playerNum = P_Get(damageSprite);
                    auto const pPlayer   = g_player[playerNum].ps;

                    spriteDist = FindDistance3D(pSprite->x - pDamage->x, pSprite->y - pDamage->y, pSprite->z - (pDamage->z - pPlayer->spritezoffset));
                }

                if (spriteDist < blastRadius)
                    A_RadiusDamageObject_Internal(spriteNum, damageSprite, blastRadius, spriteDist, randomZOffset, dmg1, dmg2, dmg3, dmg4);
            }

            damageSprite = nextSprite;
        }
    }
}

// Maybe do a projectile transport via an SE7.
// <spritenum>: the projectile
// <i>: the SE7
// <fromunderp>: below->above change?
static int32_t Proj_MaybeDoTransport(int32_t spriteNum, uspriteptr_t const pSEffector, int32_t fromunderp, int32_t daz)
{
    if (((int32_t) totalclock & UINT8_MAX) == actor[spriteNum].lasttransport)
        return 0;

    auto const pSprite = &sprite[spriteNum];
    auto const otherse = (uspriteptr_t)&sprite[pSEffector->owner];

    actor[spriteNum].lasttransport = ((int32_t) totalclock & UINT8_MAX);

    pSprite->x += (otherse->x - pSEffector->x);
    pSprite->y += (otherse->y - pSEffector->y);

    // above->below
    pSprite->z = (!fromunderp) ? sector[otherse->sectnum].ceilingz - daz + sector[pSEffector->sectnum].floorz
                               : sector[otherse->sectnum].floorz - daz + sector[pSEffector->sectnum].ceilingz;
    // below->above

    actor[spriteNum].bpos = sprite[spriteNum].xyz;
    changespritesect(spriteNum, otherse->sectnum);

    return 1;
}

// Check whether sprite <s> is on/in a non-SE7 water sector.
// <othersectptr>: if not NULL, the sector on the other side.
int A_CheckNoSE7Water(uspriteptr_t const pSprite, int sectNum, int sectLotag, int32_t *pOther)
{
    if (sectLotag == ST_1_ABOVE_WATER || sectLotag == ST_2_UNDERWATER)
    {
        int const otherSect =
        yax_getneighborsect(pSprite->x, pSprite->y, sectNum, sectLotag == ST_1_ABOVE_WATER ? YAX_FLOOR : YAX_CEILING);
        int const otherLotag = (sectLotag == ST_1_ABOVE_WATER) ? ST_2_UNDERWATER : ST_1_ABOVE_WATER;

        // If submerging, the lower sector MUST have lotag 2.
        // If emerging, the upper sector MUST have lotag 1.
        // This way, the x/y coordinates where above/below water
        // changes can happen are the same.
        if (otherSect >= 0 && sector[otherSect].lotag == otherLotag)
        {
            if (pOther)
                *pOther = otherSect;
            return 1;
        }
    }

    return 0;
}

// Check whether to do a z position update of sprite <spritenum>.
// Returns:
//  0 if no.
//  1 if yes, but stayed inside [actor[].ceilingz+1, actor[].floorz].
// <0 if yes, but passed a TROR no-SE7 water boundary. -returnvalue-1 is the
//       other-side sector number.
static int32_t A_CheckNeedZUpdate(int32_t spriteNum, int32_t zChange, int32_t *pZcoord,
    int32_t *ceilhit, int32_t *florhit)
{
    if (zChange == 0)
        return 0;

    auto const pSprite = (uspriteptr_t)&sprite[spriteNum];
    int const  newZ    = pSprite->z + (zChange >> 1);

    *pZcoord = newZ;

    if ((pSprite->cstat & CSTAT_SPRITE_ALIGNMENT_FLOOR) == 0)
    {
        int const clipDist = A_GetClipdist(spriteNum);
        VM_GetZRange(spriteNum, ceilhit, florhit, pSprite->statnum == STAT_PROJECTILE ? clipDist << 3 : clipDist);
    }

    if (newZ > actor[spriteNum].ceilingz && newZ <= actor[spriteNum].floorz)
        return 1;

#ifdef YAX_ENABLE
    int const sectNum   = pSprite->sectnum;
    int const sectLotag = sector[sectNum].lotag;
    int32_t   otherSect;

    // Non-SE7 water.
    // PROJECTILE_CHSECT
    if ((zChange < 0 && sectLotag == ST_2_UNDERWATER) || (zChange > 0 && sectLotag == ST_1_ABOVE_WATER))
    {
        if (A_CheckNoSE7Water(pSprite, sprite[spriteNum].sectnum, sectLotag, &otherSect))
        {
            A_Spawn(spriteNum, WATERSPLASH2);
            // NOTE: Don't tweak its z position afterwards like with
            // SE7-induced projectile teleportation. It doesn't look good
            // with TROR water.

            actor[spriteNum].flags |= SFLAG_DIDNOSE7WATER;
            return -otherSect-1;
        }
    }
#endif

    return 2;
}

int A_GetClipdist(int spriteNum)
{
    auto const pSprite  = &sprite[spriteNum];
    int        clipDist = pSprite->clipdist << 2;

    if (!A_CheckSpriteFlags(spriteNum, SFLAG_REALCLIPDIST))
    {
        if (pSprite->statnum == STAT_PROJECTILE)
        {
            if ((SpriteProjectile[spriteNum].workslike & PROJECTILE_REALCLIPDIST) == 0)
                clipDist = 16;
        }
        else if ((pSprite->cstat & CSTAT_SPRITE_ALIGNMENT) == CSTAT_SPRITE_ALIGNMENT_WALL)
            clipDist = 0;
        else if (A_CheckEnemySprite(pSprite))
        {
            if (pSprite->xrepeat > 60)
                clipDist = 1024;
#ifndef EDUKE32_STANDALONE
            else if (!FURY && pSprite->picnum == LIZMAN)
                clipDist = 292;
#endif
            else if (!A_CheckSpriteFlags(spriteNum, SFLAG_BADGUY))
                clipDist = 192;
        }
    }

    return clipDist;
}

int32_t A_MoveSpriteClipdist(int32_t spriteNum, vec3_t const &change, uint32_t clipType, int32_t clipDist)
{
    auto const   pSprite = &sprite[spriteNum];
    int const    isEnemy = A_CheckEnemySprite(pSprite);
    vec2_t const oldPos  = pSprite->xy;

    // check to make sure the netcode didn't leave a deleted sprite in the sprite lists.
    Bassert((unsigned)pSprite->sectnum < MAXSECTORS);

#ifndef EDUKE32_STANDALONE
    if (!FURY && (pSprite->statnum == STAT_MISC || (isEnemy && pSprite->xrepeat < 4)))
    {
        pSprite->x += change.x;
        pSprite->y += change.y;
        pSprite->z += change.z;

        if (isEnemy)
            setsprite(spriteNum, &pSprite->xyz);

        return 0;
    }
#endif

    setsprite(spriteNum, &pSprite->xyz);

    if (!(change.x|change.y|change.z))
        return 0;

    int16_t   newSectnum = pSprite->sectnum;
#ifndef EDUKE32_STANDALONE
    int const oldSectnum = newSectnum;
#endif

    // Handle horizontal movement first.

    int returnValue;
    int32_t diffZ;
    spriteheightofs(spriteNum, &diffZ, 1);

    if (pSprite->statnum == STAT_PROJECTILE)
        returnValue = clipmovex(&pSprite->xyz, &newSectnum, change.x << 13, change.y << 13, clipDist, diffZ >> 3, diffZ >> 3, clipType, 1);
    else
    {
        pSprite->z -= diffZ >> 1;
        returnValue = clipmove(&pSprite->xyz, &newSectnum, change.x << 13, change.y << 13, clipDist, ZOFFSET6, ZOFFSET6, clipType);
        pSprite->z += diffZ >> 1;
    }

    // Testing: For some reason the assert below this was tripping for clients
    EDUKE32_UNUSED int16_t   dbg_ClipMoveSectnum = newSectnum;

    if (isEnemy)
    {
        // Handle potential stayput condition (map-provided or hard-coded).
        if (newSectnum < 0 || ((actor[spriteNum].stayput >= 0 && actor[spriteNum].stayput != newSectnum)
                || ((g_tile[pSprite->picnum].flags & SFLAG_NOWATERSECTOR) && sector[newSectnum].lotag == ST_1_ABOVE_WATER)
#ifndef EDUKE32_STANDALONE
                || (!FURY && pSprite->picnum == BOSS2 && pSprite->pal == 0 && sector[newSectnum].lotag != ST_3)
                || (!FURY && (pSprite->picnum == BOSS1 || pSprite->picnum == BOSS2) && sector[newSectnum].lotag == ST_1_ABOVE_WATER)
                || (!FURY && sector[oldSectnum].lotag != ST_1_ABOVE_WATER && sector[newSectnum].lotag == ST_1_ABOVE_WATER
                    && (pSprite->picnum == LIZMAN || (pSprite->picnum == LIZTROOP && pSprite->zvel == 0)))
#endif
                ))
        {
            pSprite->xy = oldPos;

            // NOTE: in Duke3D, LIZMAN on water takes on random angle here.

            setsprite(spriteNum, &pSprite->xyz);

            if (newSectnum < 0)
                newSectnum = 0;

            return 16384+newSectnum;
        }

        if ((returnValue&49152) >= 32768 && actor[spriteNum].cgg==0)
            pSprite->ang += 768;
    }

    EDUKE32_UNUSED int16_t   dbg_newSectnum2 = newSectnum;

    if (newSectnum == -1)
    {
        newSectnum = pSprite->sectnum;
//        OSD_Printf("%s:%d wtf\n",__FILE__,__LINE__);
    }
    else if (newSectnum != pSprite->sectnum)
    {
        changespritesect(spriteNum, newSectnum);
        // A_GetZLimits(spritenum);
    }

    Bassert(newSectnum == pSprite->sectnum);

    int newZ = pSprite->z;
    int32_t ceilhit, florhit;
    int const doZUpdate = change.z ? A_CheckNeedZUpdate(spriteNum, change.z, &newZ, &ceilhit, &florhit) : 0;

    // Update sprite's z positions and (for TROR) maybe the sector number.
    if (doZUpdate == 2)
    {
        if (returnValue == 0)
            returnValue = change.z < 0 ? ceilhit : florhit;
    }
    else if (doZUpdate)
    {
        pSprite->z = newZ;
#ifdef YAX_ENABLE
        if (doZUpdate < 0)
        {
            // If we passed a TROR no-SE7 water boundary, signal to the outside
            // that the ceiling/floor was not hit. However, this is not enough:
            // later, code checks for (retval&49152)!=49152
            // [i.e. not "was ceiling or floor hit", but "was no sprite hit"]
            // and calls G_WeaponHitCeilingOrFloor() then, so we need to set
            // actor[].flags |= SFLAG_DIDNOSE7WATER in A_CheckNeedZUpdate()
            // previously.
            // XXX: Why is this contrived data flow necessary? (If at all.)
            changespritesect(spriteNum, -doZUpdate-1);
            return 0;
        }

        if (yax_getbunch(newSectnum, (change.z>0))>=0
                && (SECTORFLD(newSectnum,stat, (change.z>0))&yax_waltosecmask(clipType))==0)
        {
            setspritez(spriteNum, &pSprite->xyz);
        }
#endif
    }
    else if (change.z != 0 && returnValue == 0)
        returnValue = 16384+newSectnum;

    if (returnValue == 16384 + newSectnum)
    {
        if (pSprite->statnum == STAT_PROJECTILE)
        {
            // Projectile sector changes due to transport SEs (SE7_PROJECTILE).
            // PROJECTILE_CHSECT
            for (bssize_t SPRITES_OF(STAT_TRANSPORT, otherSpriteNum))
            {
                if (sprite[otherSpriteNum].sectnum == newSectnum)
                {
                    int const sectLotag = sector[newSectnum].lotag;

                    if (sectLotag == ST_1_ABOVE_WATER && newZ >= actor[spriteNum].floorz)
                        if (Proj_MaybeDoTransport(spriteNum, (uspriteptr_t)&sprite[otherSpriteNum], 0, newZ))
                            return 0;

                    if (sectLotag == ST_2_UNDERWATER && newZ <= actor[spriteNum].ceilingz)
                        if (Proj_MaybeDoTransport(spriteNum, (uspriteptr_t)&sprite[otherSpriteNum], 1, newZ))
                            return 0;
                }
            }
        }
    }

    return returnValue;
}

int32_t block_deletesprite = 0;

#ifdef POLYMER
static void A_DeleteLight(int32_t s)
{
    if (practor[s].lightId >= 0)
        polymer_deletelight(practor[s].lightId);
    practor[s].lightId = -1;
    practor[s].lightptr = NULL;
}

void G_DeleteAllLights(void)
{
    for (int i=0; i<MAXSPRITES; i++)
        A_DeleteLight(i);
}

void G_Polymer_UnInit(void)
{
    G_DeleteAllLights();
}
#endif

// deletesprite() game wrapper
void A_DeleteSprite(int spriteNum)
{
    if (EDUKE32_PREDICT_FALSE(block_deletesprite))
    {
        LOG_F(ERROR, "A_DeleteSprite(): tried to remove sprite %d in EVENT_EGS!", spriteNum);
        return;
    }

    if (VM_HaveEvent(EVENT_KILLIT))
    {
        int32_t playerDist;
        int playerNum = A_FindPlayer(&sprite[spriteNum], &playerDist);

        if (VM_ExecuteEvent(EVENT_KILLIT, spriteNum, playerNum, playerDist))
            return;
    }

#ifdef POLYMER
    if (practor[spriteNum].lightptr != NULL && videoGetRenderMode() == REND_POLYMER)
        A_DeleteLight(spriteNum);
#endif

    // AMBIENT_SFX_PLAYING
    if (sprite[spriteNum].picnum == MUSICANDSFX && actor[spriteNum].t_data[0] == 1)
        S_StopEnvSound(sprite[spriteNum].lotag, spriteNum);

#ifdef NETCODE_DISABLE
    deletesprite(spriteNum);
#else
    Net_DeleteSprite(spriteNum);
#endif
}

void A_AddToDeleteQueue(int spriteNum)
{
    if (g_netClient || (g_deleteQueueSize == 0)) // [75] Clients should not use SpriteDeletionQueue[] and just set the sprites invisible immediately in A_DeleteSprite
    {
        A_DeleteSprite(spriteNum);
        return;
    }

    auto &deleteSpriteNum = SpriteDeletionQueue[g_spriteDeleteQueuePos];

    if (deleteSpriteNum >= 0 && actor[deleteSpriteNum].flags & SFLAG_QUEUEDFORDELETE)
        A_DeleteSprite(deleteSpriteNum);

    deleteSpriteNum = spriteNum;
    actor[spriteNum].flags |= SFLAG_QUEUEDFORDELETE;
    g_spriteDeleteQueuePos = (g_spriteDeleteQueuePos+1)%g_deleteQueueSize;
}

void A_SpawnMultiple(int spriteNum, int tileNum, int spawnCnt)
{
    auto const pSprite = &sprite[spriteNum];

    for (; spawnCnt>0; spawnCnt--)
    {
        int const j = A_InsertSprite(pSprite->sectnum, pSprite->x, pSprite->y, pSprite->z - (krand() % (47 << 8)), tileNum, -32, 8,
                               8, krand() & 2047, 0, 0, spriteNum, 5);
        A_Spawn(-1, j);
        sprite[j].cstat = krand()&12;
    }
}

#ifndef EDUKE32_STANDALONE
void A_DoGuts(int spriteNum, int tileNum, int spawnCnt)
{
    auto const pSprite = (uspriteptr_t)&sprite[spriteNum];
    vec2_t     repeat  = { 32, 32 };

    if (A_CheckEnemySprite(pSprite) && pSprite->xrepeat < 16)
        repeat.x = repeat.y = 8;

    int gutZ   = pSprite->z - ZOFFSET3;
    int floorz = getflorzofslope(pSprite->sectnum, pSprite->x, pSprite->y);

    if (gutZ > (floorz-ZOFFSET3))
        gutZ = floorz-ZOFFSET3;

    if (pSprite->picnum == COMMANDER)
        gutZ -= (24<<8);

    for (bssize_t j=spawnCnt; j>0; j--)
    {
        int const i = A_InsertSprite(pSprite->sectnum, pSprite->x + (krand() & 255) - 128,
                                     pSprite->y + (krand() & 255) - 128, gutZ - (krand() & 8191), tileNum, -32, repeat.x,
                                     repeat.y, krand() & 2047, 48 + (krand() & 31), -512 - (krand() & 2047), spriteNum, 5);

        if (PN(i) == JIBS2)
        {
            sprite[i].xrepeat >>= 2;
            sprite[i].yrepeat >>= 2;
        }

        sprite[i].pal = pSprite->pal;
    }
}
#endif

static int32_t G_ToggleWallInterpolation(int32_t wallNum, int32_t setInterpolation)
{
    if (setInterpolation)
    {
        return G_SetInterpolation(&wall[wallNum].x) || G_SetInterpolation(&wall[wallNum].y);
    }
    else
    {
        G_StopInterpolation(&wall[wallNum].x);
        G_StopInterpolation(&wall[wallNum].y);
        return 0;
    }
}

void Sect_ToggleInterpolation(int sectNum, int setInterpolation)
{
    for (bssize_t j = sector[sectNum].wallptr, endwall = sector[sectNum].wallptr + sector[sectNum].wallnum; j < endwall; j++)
    {
        G_ToggleWallInterpolation(j, setInterpolation);

        int const nextWall = wall[j].nextwall;

        if (nextWall >= 0)
        {
            G_ToggleWallInterpolation(nextWall, setInterpolation);
            G_ToggleWallInterpolation(wall[nextWall].point2, setInterpolation);
        }
    }
}

static int32_t move_rotfixed_sprite(int32_t spriteNum, int32_t pivotSpriteNum, int32_t pivotAngle)
{
    if ((ROTFIXSPR_STATNUMP(sprite[spriteNum].statnum) ||
         ((sprite[spriteNum].statnum == STAT_ACTOR || sprite[spriteNum].statnum == STAT_ZOMBIEACTOR) &&
          A_CheckSpriteFlags(spriteNum, SFLAG_ROTFIXED))) &&
        actor[spriteNum].t_data[7] == (ROTFIXSPR_MAGIC | pivotSpriteNum))
    {
        rotatevec(*(vec2_t *)&actor[spriteNum].t_data[8], pivotAngle & 2047, &sprite[spriteNum].xy);
        sprite[spriteNum].x += sprite[pivotSpriteNum].x;
        sprite[spriteNum].y += sprite[pivotSpriteNum].y;
        return 0;
    }

    return 1;
}

void A_MoveSector(int spriteNum)
{
    // T1,T2 and T3 are used for all the sector moving stuff!!!

    int32_t    playerDist;
    auto const pSprite     = &sprite[spriteNum];
    int const  playerNum   = A_FindPlayer(pSprite, &playerDist);
    int const  rotateAngle = VM_OnEvent(EVENT_MOVESECTOR, spriteNum, playerNum, playerDist, T3(spriteNum));
    int        originIdx   = T2(spriteNum);

    pSprite->x += (pSprite->xvel * (sintable[(pSprite->ang + 512) & 2047])) >> 14;
    pSprite->y += (pSprite->xvel * (sintable[pSprite->ang & 2047])) >> 14;

    int const endWall = sector[pSprite->sectnum].wallptr + sector[pSprite->sectnum].wallnum;

    for (bssize_t wallNum = sector[pSprite->sectnum].wallptr; wallNum < endWall; wallNum++)
    {
        vec2_t const origin = g_origins[originIdx];
        vec2_t result;
        rotatevec(origin, rotateAngle & 2047, &result);
        dragpoint(wallNum, pSprite->x + result.x, pSprite->y + result.y, 0);

        originIdx++;
    }
}

void G_AddGameLight(int spriteNum, int sectNum, vec3_t const &offset, int lightRange, int lightRadius, int lightHoriz, uint32_t lightColor, int lightPrio)
{
#ifdef POLYMER
    uspriteptr_t s = (uspriteptr_t )&sprite[spriteNum];
    auto pr_actor = &practor[spriteNum];

    Bassert(sectNum != -1);

    if (videoGetRenderMode() != REND_POLYMER || pr_lighting != 1)
        return;

    if (pr_actor->lightptr == NULL)
    {
#pragma pack(push, 1)
        _prlight pr_light;
#pragma pack(pop)
        Bmemset(&pr_light, 0, sizeof(pr_light));

        pr_light.sector = sectNum;
        pr_light.xyz = s->xyz;
        updatesector(pr_light.x, pr_light.y, &pr_light.sector);
        pr_light.xyz -= offset;

        pr_light.color[0] = lightColor & 255;
        pr_light.color[1] = (lightColor >> 8) & 255;
        pr_light.color[2] = (lightColor >> 16) & 255;

        if (s->pal)
        {
            int colidx = paletteGetClosestColorWithBlacklist(pr_light.color[0], pr_light.color[1], pr_light.color[2], 254, PaletteIndexFullbright);
            colidx = palookup[s->pal][colidx];
            pr_actor->lightcolidx = colidx;
            pr_light.color[0] = curpalette[colidx].r;
            pr_light.color[1] = curpalette[colidx].g;
            pr_light.color[2] = curpalette[colidx].b;
        }
        else
            pr_actor->lightcolidx = 0;

        pr_light.radius = lightRadius;
        pr_light.priority = lightPrio;
        pr_light.tilenum = 0;
        pr_light.owner = spriteNum;
        pr_light.horiz = lightHoriz;

        pr_light.publicflags.emitshadow = 1;
        pr_light.publicflags.negative = 0;

        pr_actor->lightrange = pr_actor->olightrange = pr_actor->lightmaxrange = pr_light.range = lightRange;
        pr_actor->lightang = pr_actor->olightang = pr_light.angle = s->ang;
        pr_actor->lightoffset = offset;

        pr_actor->lightId = polymer_addlight(&pr_light);
        if (pr_actor->lightId >= 0)
            pr_actor->lightptr = &prlights[pr_actor->lightId];
        return;
    }

    _prlight &pr_light = *pr_actor->lightptr;

    if (lightRange < pr_actor->lightmaxrange >> 1)
        pr_actor->lightmaxrange = 0;

    if (lightRange > pr_actor->lightmaxrange || lightPrio != pr_light.priority || sprite[spriteNum].xyz != pr_light.xyz + offset
        || (lightRadius && pr_light.angle != s->ang))
    {
        if (lightRange > pr_actor->lightmaxrange)
            pr_actor->lightmaxrange = lightRange;

        pr_light.xyz = sprite[spriteNum].xyz;
        pr_light.xyz -= offset;
        pr_light.sector = sectNum;
        updatesector(pr_light.x, pr_light.y, &pr_light.sector);
        pr_light.flags.invalidate = 1;
    }

    pr_actor->lightoffset = offset;
    pr_light.priority = lightPrio;
    pr_actor->olightrange = pr_actor->lightrange;
    pr_light.range = pr_actor->lightrange = lightRange;
    pr_actor->olightang = pr_actor->lightang;
    pr_light.angle = pr_actor->lightang = s->ang;
    pr_light.color[0] = lightColor & 255;
    pr_light.color[1] = (lightColor >> 8) & 255;
    pr_light.color[2] = (lightColor >> 16) & 255;
    pr_light.horiz = lightHoriz;

    if (pr_actor->lightcolidx)
    {
        int colidx = pr_actor->lightcolidx;
        pr_light.color[0] = curpalette[colidx].r;
        pr_light.color[1] = curpalette[colidx].g;
        pr_light.color[2] = curpalette[colidx].b;
    }
#else
    auto unusedParameterWarningsOnConstReferencesSuck = offset;
    UNREFERENCED_PARAMETER(unusedParameterWarningsOnConstReferencesSuck);
    UNREFERENCED_PARAMETER(lightRadius);
    UNREFERENCED_PARAMETER(spriteNum);
    UNREFERENCED_PARAMETER(sectNum);
    UNREFERENCED_PARAMETER(lightRange);
    UNREFERENCED_PARAMETER(lightHoriz);
    UNREFERENCED_PARAMETER(lightColor);
    UNREFERENCED_PARAMETER(lightPrio);
#endif
}

void G_InterpolateLights(int smoothratio)
{
#ifdef POLYMER
    uint16_t const unumsectors = (unsigned)numsectors;

    for (int i=0;i<PR_MAXLIGHTS;i++)
    {
        auto &pr_light = prlights[i];

        if (!pr_light.flags.active || (unsigned)pr_light.owner >= MAXSPRITES)
            continue;

        uspriteptr_t pSprite = (uspriteptr_t )&sprite[pr_light.owner];

        if ((unsigned)pr_light.sector >= unumsectors || (unsigned)pSprite->sectnum >= unumsectors || pSprite->statnum == MAXSTATUS)
        {
            polymer_deletelight(i);
            continue;
        }

        auto pActor = &actor[pr_light.owner];
        auto &pr_actor = practor[pr_light.owner];

        pr_light.xyz = pSprite->xyz;
        pr_light.xyz -= pr_actor.lightoffset;
        pr_light.xyz -= { mulscale16(65536 - smoothratio, pSprite->x - pActor->bpos.x),
                         mulscale16(65536 - smoothratio, pSprite->y - pActor->bpos.y),
                         mulscale16(65536 - smoothratio, pSprite->z - pActor->bpos.z) };

        if (pSprite->picnum != SECTOREFFECTOR)
        pr_light.xy -= { sintable[(fix16_to_int(CAMERA(q16ang)) + 512) & 2047] >> 10,
                         sintable[fix16_to_int(CAMERA(q16ang)) & 2047] >> 10 };

        pr_light.range = pr_actor.lightrange - mulscale16(65536 - smoothratio, pr_actor.lightrange - pr_actor.olightrange);
        pr_light.angle = pr_actor.lightang - mulscale16(65536 - smoothratio, ((pr_actor.lightang + 1024 - pr_actor.olightang) & 2047) - 1024);
    }
#else
    UNREFERENCED_PARAMETER(smoothratio);
#endif
}

ACTOR_STATIC void A_MaybeAwakenBadGuys(int const spriteNum)
{
    if (sprite[spriteNum].sectnum == MAXSECTORS)
        return;

    if (A_CheckSpriteFlags(spriteNum, SFLAG_WAKEUPBADGUYS))
    {
        auto const pSprite = (uspriteptr_t)&sprite[spriteNum];

        for (bssize_t nextSprite, SPRITES_OF_STAT_SAFE(STAT_ZOMBIEACTOR, spriteNum, nextSprite))
        {
            if (A_CheckEnemySprite(&sprite[spriteNum]))
            {
                if (sprite[spriteNum].sectnum == pSprite->sectnum
                    || sprite[spriteNum].sectnum == nextsectorneighborz(pSprite->sectnum, sector[pSprite->sectnum].floorz, 1, 1)
                    || cansee(pSprite->x, pSprite->y, pSprite->z - PHEIGHT, pSprite->sectnum, sprite[spriteNum].x, sprite[spriteNum].y,
                              sprite[spriteNum].z - PHEIGHT, sprite[spriteNum].sectnum))
                {
                    actor[spriteNum].timetosleep = SLEEPTIME;
                    A_PlayAlertSound(spriteNum);
                    changespritestat(spriteNum, STAT_ACTOR);

                    if (A_CheckSpriteFlags(spriteNum, SFLAG_WAKEUPBADGUYS))
                        A_MaybeAwakenBadGuys(spriteNum);
                }
            }
        }
    }
}


// sleeping monsters, etc
ACTOR_STATIC void G_MoveZombieActors(void)
{
    int spriteNum = headspritestat[STAT_ZOMBIEACTOR], canSeePlayer;

    while (spriteNum >= 0)
    {
        int const  nextSprite = nextspritestat[spriteNum];
        int32_t    playerDist;
        auto const pSprite   = &sprite[spriteNum];
        int const  playerNum = A_FindPlayer(pSprite, &playerDist);
        auto const pPlayer   = g_player[playerNum].ps;

        if (sprite[pPlayer->i].extra > 0)
        {
            if (playerDist < 30000)
            {
                actor[spriteNum].timetosleep++;
                if (actor[spriteNum].timetosleep >= (playerDist>>8))
                {
                    if (pPlayer->newowner == -1 && A_CheckEnemySprite(pSprite))
                    {
                        vec3_t const p = { pPlayer->pos.x + 64 - (krand() & 127),
                                           pPlayer->pos.y + 64 - (krand() & 127),
                                           pPlayer->pos.z - (krand() % ZOFFSET5) };

                        int16_t pSectnum = pPlayer->cursectnum;

                        updatesector(p.x, p.y, &pSectnum);

                        if (pSectnum == -1)
                        {
                            spriteNum = nextSprite;
                            continue;
                        }

                        vec3_t const s = { pSprite->x + 64 - (krand() & 127),
                                           pSprite->y + 64 - (krand() & 127),
                                           pSprite->z - (krand() % (52 << 8)) };

                        int16_t sectNum = pSprite->sectnum;

                        updatesector(s.x, s.y, &sectNum);

                        if (sectNum == -1)
                        {
                            spriteNum = nextSprite;
                            continue;
                        }

                        canSeePlayer = cansee(s.x, s.y, s.z, sectNum, p.x, p.y, p.z, pSectnum);
                    }
                    else
                        canSeePlayer = cansee(pSprite->x, pSprite->y, pSprite->z - ((krand() & 31) << 8), pSprite->sectnum, pPlayer->opos.x,
                            pPlayer->opos.y, pPlayer->opos.z - ((krand() & 31) << 8), pPlayer->cursectnum);

                    if (canSeePlayer)
                    {
                        switch (tileGetMapping(pSprite->picnum))
                        {
#ifndef EDUKE32_STANDALONE
                            case RUBBERCAN__:
                            case EXPLODINGBARREL__:
                            case WOODENHORSE__:
                            case HORSEONSIDE__:
                            case CANWITHSOMETHING__:
                            case CANWITHSOMETHING2__:
                            case CANWITHSOMETHING3__:
                            case CANWITHSOMETHING4__:
                            case FIREBARREL__:
                            case FIREVASE__:
                            case NUKEBARREL__:
                            case NUKEBARRELDENTED__:
                            case NUKEBARRELLEAKED__:
                            case TRIPBOMB__:
                                if (!FURY)
                                {
                                    pSprite->shade = ((sector[pSprite->sectnum].ceilingstat & 1) && !A_CheckSpriteFlags(spriteNum, SFLAG_NOSHADE))
                                                     ? sector[pSprite->sectnum].ceilingshade
                                                     : sector[pSprite->sectnum].floorshade;
                                    actor[spriteNum].timetosleep = 0;
                                    changespritestat(spriteNum, STAT_STANDABLE);
                                    break;
                                }
                                fallthrough__;

                            case RECON__:
                                if (!FURY && pSprite->picnum == RECON)
                                    CS(spriteNum) |= 257;
                                fallthrough__;
#endif
                            default:
                                if (A_CheckSpriteFlags(spriteNum, SFLAG_USEACTIVATOR) && sector[sprite[spriteNum].sectnum].lotag & 16384)
                                    break;

                                actor[spriteNum].timetosleep = 0;
                                A_PlayAlertSound(spriteNum);
                                changespritestat(spriteNum, STAT_ACTOR);

                                if (A_CheckSpriteFlags(spriteNum, SFLAG_WAKEUPBADGUYS))
                                    A_MaybeAwakenBadGuys(spriteNum);

                                break;
                        }
                    }
                    else
                        actor[spriteNum].timetosleep = 0;
                }
            }

            if (A_CheckEnemySprite(pSprite) && !A_CheckSpriteFlags(spriteNum,SFLAG_NOSHADE))
            {
                pSprite->shade = (sector[pSprite->sectnum].ceilingstat & 1)
                                ? sector[pSprite->sectnum].ceilingshade
                                : sector[pSprite->sectnum].floorshade;
            }
        }

        spriteNum = nextSprite;
    }
}

// stupid name, but it's what the function does.
static FORCE_INLINE int G_FindExplosionInSector(int const sectNum)
{
    for (bssize_t SPRITES_OF(STAT_MISC, i))
        if (PN(i) == EXPLOSION2 && sectNum == SECT(i))
            return i;

    return -1;
}

static FORCE_INLINE void P_Nudge(int playerNum, int spriteNum, int shiftLeft)
{
    g_player[playerNum].ps->vel.x += actor[spriteNum].htextra * (sintable[(actor[spriteNum].htang + 512) & 2047]) << shiftLeft;
    g_player[playerNum].ps->vel.y += actor[spriteNum].htextra * (sintable[actor[spriteNum].htang & 2047]) << shiftLeft;
}

int A_IncurDamage(int const spriteNum)
{
    auto const pSprite = &sprite[spriteNum];
    auto const pActor  = &actor[spriteNum];

    // dmg->picnum check: safety, since it might have been set to <0 from CON.
    if (pActor->htextra < 0 || pSprite->extra < 0 || pActor->htpicnum < 0)
    {
        pActor->htextra = -1;
        return -1;
    }

    if (pSprite->picnum != APLAYER)
    {
        if (pActor->htextra == 0 && STANDALONE_EVAL(false, pActor->htpicnum == SHRINKSPARK) && pSprite->xrepeat < 24)
            return -1;

        int32_t playerDist;
        int playerNum = A_FindPlayer(pSprite, &playerDist);

        if (VM_OnEvent(EVENT_PREACTORDAMAGE, spriteNum, playerNum, playerDist, 0))
            return -1;

        pSprite->extra -= pActor->htextra;

        if ((unsigned)pSprite->owner < MAXSPRITES && sprite[pSprite->owner].statnum < MAXSTATUS && STANDALONE_EVAL(true, pSprite->picnum != RECON))
            pSprite->owner = pActor->htowner;

        pActor->htextra = -1;
        return pActor->htpicnum;
    }
    else
    {
        if (ud.god && STANDALONE_EVAL(true, pActor->htpicnum != SHRINKSPARK))
            return -1;

        int const playerNum = P_GetP(pSprite);

#ifndef EDUKE32_STANDALONE
        if (!FURY && pActor->htowner >= 0 && (sprite[pActor->htowner].picnum == APLAYER))
        {
            if (
                (ud.ffire == 0) &&
                (spriteNum != pActor->htowner) &&       // Not damaging self.
                ((g_gametypeFlags[ud.coop] & GAMETYPE_PLAYERSFRIENDLY) ||
                ((g_gametypeFlags[ud.coop] & GAMETYPE_TDM) && g_player[playerNum].ps->team == g_player[P_Get(pActor->htowner)].ps->team))
                )
                {
                    // Nullify damage and cancel.
                    pActor->htowner = -1;
                    pActor->htextra = -1;
                    return -1;
                }
        }
#endif
        pSprite->extra -= pActor->htextra;

        if (pActor->htowner >= 0 && pSprite->extra <= 0 && STANDALONE_EVAL(true, pActor->htpicnum != FREEZEBLAST))
        {
            int const damageOwner = pActor->htowner;
            pSprite->extra        = 0;

            g_player[playerNum].ps->wackedbyactor = damageOwner;

            if (sprite[damageOwner].picnum == APLAYER && playerNum != P_Get(damageOwner))
                g_player[playerNum].ps->frag_ps = P_Get(damageOwner);

            pActor->htowner = g_player[playerNum].ps->i;
        }

        switch (tileGetMapping(pActor->htpicnum))
        {
            case RADIUSEXPLOSION__:
            case SEENINE__:
#ifndef EDUKE32_STANDALONE
            case RPG__:
            case HYDRENT__:
            case HEAVYHBOMB__:
            case OOZFILTER__:
            case EXPLODINGBARREL__:
#endif
                P_Nudge(playerNum, spriteNum, 2);
                break;

            default:
            {
                int const isRPG = (A_CheckSpriteTileFlags(pActor->htpicnum, SFLAG_PROJECTILE) && (g_tile[pActor->htpicnum].proj->workslike & PROJECTILE_RPG));
                P_Nudge(playerNum, spriteNum, 1 << isRPG);
                break;
            }
        }

        pActor->htextra = -1;
        return pActor->htpicnum;
    }
}

void A_MoveCyclers(void)
{
    for (bssize_t i=g_cyclerCnt-1; i>=0; i--)
    {
        int16_t *const pCycler     = g_cyclers[i];
        int const      sectNum     = pCycler[0];
        int            spriteShade = pCycler[2];
        int const      floorShade  = pCycler[3];
        int            sectorShade = clamp(floorShade + (sintable[pCycler[1] & 2047] >> 10), spriteShade, floorShade);

        pCycler[1] += sector[sectNum].extra;

        if (pCycler[5]) // angle 1536...
        {
            walltype *pWall = &wall[sector[sectNum].wallptr];

            for (bssize_t wallsLeft = sector[sectNum].wallnum; wallsLeft > 0; wallsLeft--, pWall++)
            {
                if (pWall->hitag != 1)
                {
                    pWall->shade = sectorShade;

                    if ((pWall->cstat&2) && pWall->nextwall >= 0)
                        wall[pWall->nextwall].shade = sectorShade;
                }
            }

            sector[sectNum].floorshade = sector[sectNum].ceilingshade = sectorShade;
        }
    }
}

void A_MoveDummyPlayers(void)
{
    int spriteNum = headspritestat[STAT_DUMMYPLAYER];

    while (spriteNum >= 0)
    {
        int const  playerNum     = P_Get(OW(spriteNum));
        auto const pPlayer       = g_player[playerNum].ps;
        int const  nextSprite    = nextspritestat[spriteNum];
        int const  playerSectnum = pPlayer->cursectnum;

        if (pPlayer->on_crane >= 0 || (playerSectnum >= 0 && sector[playerSectnum].lotag != ST_1_ABOVE_WATER) || sprite[pPlayer->i].extra <= 0
#ifdef YAX_ENABLE
           || yax_getbunch(pPlayer->cursectnum, YAX_FLOOR) >= 0
#endif
           )
        {
            pPlayer->dummyplayersprite = -1;
            DELETE_SPRITE_AND_CONTINUE(spriteNum);
        }
        else
        {
            if (pPlayer->on_ground && pPlayer->on_warping_sector == 1 && playerSectnum >= 0 && sector[playerSectnum].lotag == ST_1_ABOVE_WATER)
            {
                CS(spriteNum) = 257;
                SZ(spriteNum) = sector[SECT(spriteNum)].ceilingz+(27<<8);
                SA(spriteNum) = fix16_to_int(pPlayer->q16ang);
                if (T1(spriteNum) == 8)
                    T1(spriteNum) = 0;
                else T1(spriteNum)++;
            }
            else
            {
                if (sector[SECT(spriteNum)].lotag != ST_2_UNDERWATER) SZ(spriteNum) = sector[SECT(spriteNum)].floorz;
                CS(spriteNum) = 32768;
            }
        }

        sprite[spriteNum].xyz += pPlayer->pos.xy - pPlayer->opos.xy;
        setsprite(spriteNum, &sprite[spriteNum].xyz);

next_sprite:
        spriteNum = nextSprite;
    }
}


static int P_Submerge(int, DukePlayer_t *, int, int);
static int P_Emerge(int, DukePlayer_t *, int, int);
static void P_FinishWaterChange(int, DukePlayer_t *, int, int, int);

static FORCE_INLINE fix16_t P_GetQ16AngleDeltaForTic(DukePlayer_t const *pPlayer) { return getq16angledelta(pPlayer->oq16ang, pPlayer->q16ang); }

ACTOR_STATIC void G_MovePlayers(void)
{
    int spriteNum = headspritestat[STAT_PLAYER];

    while (spriteNum >= 0)
    {
        int const  nextSprite = nextspritestat[spriteNum];
        auto const pSprite    = &sprite[spriteNum];
        int const  playerNum  = P_GetP(pSprite);
        auto &     thisPlayer = g_player[playerNum];
        auto const pPlayer    = thisPlayer.ps;

        if (pSprite->owner >= 0)
        {
            if (pPlayer->newowner >= 0)  //Looking thru the camera
            {
                pSprite->x              = pPlayer->opos.x;
                pSprite->y              = pPlayer->opos.y;
                pSprite->z              = pPlayer->opos.z + pPlayer->spritezoffset;
                actor[spriteNum].bpos.z = pSprite->z;
                pSprite->ang            = fix16_to_int(pPlayer->oq16ang);

                setsprite(spriteNum, &pSprite->xyz);
            }
            else
            {
                int32_t otherPlayerDist;
#ifdef YAX_ENABLE
                // TROR water submerge/emerge
                int const playerSectnum = pSprite->sectnum;
                int const sectorLotag   = sector[playerSectnum].lotag;
                int32_t   otherSector;

                if (A_CheckNoSE7Water((uspriteptr_t)pSprite, playerSectnum, sectorLotag, &otherSector))
                {
                    // NOTE: Compare with G_MoveTransports().
                    pPlayer->on_warping_sector = 1;

                    if ((sectorLotag == ST_1_ABOVE_WATER ?
                        P_Submerge(P_GetP(pSprite), pPlayer, playerSectnum, otherSector) :
                        P_Emerge(P_GetP(pSprite), pPlayer, playerSectnum, otherSector)) == 1)
                        P_FinishWaterChange(spriteNum, pPlayer, sectorLotag, -1, otherSector);
                }
#endif
                if (g_netServer || ud.multimode > 1)
                    otherp = P_FindOtherPlayer(P_GetP(pSprite), &otherPlayerDist);
                else
                {
                    otherp = P_GetP(pSprite);
                    otherPlayerDist = 0;
                }

                if (G_TileHasActor(sprite[spriteNum].picnum))
                    A_Execute(spriteNum, P_GetP(pSprite), otherPlayerDist);

                if (pPlayer->newowner < 0)
                {
                    pPlayer->q16angvel    = P_GetQ16AngleDeltaForTic(pPlayer);
                    pPlayer->oq16ang      = pPlayer->q16ang;
                    pPlayer->oq16horiz    = pPlayer->q16horiz;
                    pPlayer->oq16horizoff = pPlayer->q16horizoff;
                }

                if (ud.recstat || pPlayer->gm & MODE_DEMO)
                {
                    thisPlayer.smoothcamera = true;
                    P_UpdateAngles(playerNum, thisPlayer.input);
                }

                if (pPlayer->one_eighty_count < 0)
                {
                    thisPlayer.smoothcamera = true;
                    pPlayer->one_eighty_count += 128;
                    pPlayer->q16ang += F16(128);
                }

                if (g_netServer || ud.multimode > 1)
                {
                    if (sprite[g_player[otherp].ps->i].extra > 0)
                    {
                        if (pSprite->yrepeat > 32 && sprite[g_player[otherp].ps->i].yrepeat < 32)
                        {
                            if (otherPlayerDist < 1400 && pPlayer->knee_incs == 0)
                            {
                                // Don't stomp teammates.
                                if (
                                    ((g_gametypeFlags[ud.coop] & GAMETYPE_TDM) && pPlayer->team != g_player[otherp].ps->team) ||
                                    (!(g_gametypeFlags[ud.coop] & GAMETYPE_PLAYERSFRIENDLY) && !(g_gametypeFlags[ud.coop] & GAMETYPE_TDM))
                                    )
                                {
                                    pPlayer->knee_incs = 1;
                                    pPlayer->weapon_pos = -1;
                                    pPlayer->actorsqu = g_player[otherp].ps->i;
                                }
                            }
                        }
                    }
                }

                if (pPlayer->actorsqu >= 0)
                {
                    thisPlayer.smoothcamera = true;
                    pPlayer->q16ang += fix16_from_int(
                    G_GetAngleDelta(fix16_to_int(pPlayer->q16ang), getangle(sprite[pPlayer->actorsqu].x - pPlayer->pos.x, sprite[pPlayer->actorsqu].y - pPlayer->pos.y))
                    >> 2);
                }

                if (ud.god)
                {
                    pSprite->extra = pPlayer->max_player_health;
                    pSprite->cstat = 257;
                    if (!WW2GI)
                        pPlayer->inv_amount[GET_JETPACK] = 1599;
                }

                if (pSprite->extra > 0)
                {
#ifndef EDUKE32_STANDALONE
                    if (!FURY)
                    {
                        actor[spriteNum].htowner = spriteNum;

                        if (ud.god == 0)
                            if (G_CheckForSpaceCeiling(pSprite->sectnum) || G_CheckForSpaceFloor(pSprite->sectnum))
                            {
                                LOG_F(WARNING, "%s: player killed by space sector!", EDUKE32_FUNCTION);
                                P_QuickKill(pPlayer);
                            }
                    }
#endif
                }
                else
                {
                    pPlayer->pos.x = pSprite->x;
                    pPlayer->pos.y = pSprite->y;
                    pPlayer->pos.z = pSprite->z-(20<<8);

                    pPlayer->newowner = -1;

                    if (pPlayer->wackedbyactor >= 0 && sprite[pPlayer->wackedbyactor].statnum < MAXSTATUS)
                    {
                        thisPlayer.smoothcamera = true;
                        pPlayer->q16ang += fix16_from_int(G_GetAngleDelta(fix16_to_int(pPlayer->q16ang),
                                                                      getangle(sprite[pPlayer->wackedbyactor].x - pPlayer->pos.x,
                                                                               sprite[pPlayer->wackedbyactor].y - pPlayer->pos.y))
                                                      >> 1);
                        pPlayer->q16ang &= 0x7FFFFFF;
                    }
                }

                pSprite->ang = fix16_to_int(pPlayer->q16ang);
            }
        }
        else
        {
            if (pPlayer->holoduke_on == -1)
                DELETE_SPRITE_AND_CONTINUE(spriteNum);

            actor[spriteNum].bpos = pSprite->xyz;
            pSprite->cstat = 0;

            if (pSprite->xrepeat < 42)
            {
                pSprite->xrepeat += 4;
                pSprite->cstat |= 2;
            }
            else pSprite->xrepeat = 42;

            if (pSprite->yrepeat < 36)
                pSprite->yrepeat += 4;
            else
            {
                pSprite->yrepeat = 36;
                if (sector[pSprite->sectnum].lotag != ST_2_UNDERWATER)
                    A_Fall(spriteNum);
                if (pSprite->zvel == 0 && sector[pSprite->sectnum].lotag == ST_1_ABOVE_WATER)
                    pSprite->z += ZOFFSET5;
            }

            if (pSprite->extra < 8)
            {
                pSprite->xvel = 128;
                pSprite->ang = fix16_to_int(pPlayer->q16ang);
                pSprite->extra++;
                A_SetSprite(spriteNum,CLIPMASK0);
            }
            else
            {
                pSprite->ang = 2047-fix16_to_int(pPlayer->q16ang);
                setsprite(spriteNum,&pSprite->xyz);
            }
        }

        pSprite->shade =
        logapproach(pSprite->shade, (sector[pSprite->sectnum].ceilingstat & 1) ? sector[pSprite->sectnum].ceilingshade
                                                                               : sector[pSprite->sectnum].floorshade);

next_sprite:
        spriteNum = nextSprite;
    }
}

ACTOR_STATIC void G_MoveFX(void)
{
    int spriteNum = headspritestat[STAT_FX];

    while (spriteNum >= 0)
    {
        auto const pSprite    = &sprite[spriteNum];
        int const  nextSprite = nextspritestat[spriteNum];

        switch (tileGetMapping(pSprite->picnum))
        {
        case RESPAWN__:
            if (pSprite->extra == 66)
            {
                /*int32_t j =*/ A_Spawn(spriteNum,SHT(spriteNum));
                //                    sprite[j].pal = sprite[i].pal;
                DELETE_SPRITE_AND_CONTINUE(spriteNum);
            }
            else if (pSprite->extra > (66-13))
                sprite[spriteNum].extra++;
            break;

        case MUSICANDSFX__:
        {
            int32_t const spriteHitag = (uint16_t)pSprite->hitag;
            auto const    pPlayer     = g_player[screenpeek].ps;

            if (T2(spriteNum) != ud.config.SoundToggle)
            {
                // If sound playback was toggled, restart.
                T2(spriteNum) = ud.config.SoundToggle;
                T1(spriteNum) = 0;
            }

            if (pSprite->lotag >= 1000 && pSprite->lotag < 2000)
            {
                int32_t playerDist = ldist(&sprite[pPlayer->i], pSprite);

#ifdef SPLITSCREEN_MOD_HACKS
                if (g_fakeMultiMode==2)
                {
                    // HACK for splitscreen mod
                    int32_t otherdist = ldist(&sprite[g_player[1].ps->i],pSprite);
                    playerDist = min(playerDist, otherdist);
                }
#endif

                if (playerDist < spriteHitag && T1(spriteNum) == 0)
                {
                    FX_SetReverb(pSprite->lotag - 1000);
                    T1(spriteNum) = 1;
                }
                else if (playerDist >= spriteHitag && T1(spriteNum) == 1)
                {
                    FX_SetReverb(0);
                    FX_SetReverbDelay(0);
                    T1(spriteNum) = 0;
                }
            }
            else if (pSprite->lotag < 999 && pSprite->lotag && S_SoundIsValid(pSprite->lotag) && (unsigned)sector[pSprite->sectnum].lotag < 9 &&  // ST_9_SLIDING_ST_DOOR
                         ud.config.AmbienceToggle && sector[SECT(spriteNum)].floorz != sector[SECT(spriteNum)].ceilingz)
            {
                if (g_sounds[pSprite->lotag]->flags & SF_MSFX)
                {
                    int playerDist = dist(&sprite[pPlayer->i], pSprite);

#ifdef SPLITSCREEN_MOD_HACKS
                    if (g_fakeMultiMode==2)
                    {
                        // HACK for splitscreen mod
                        int32_t otherdist = dist(&sprite[g_player[1].ps->i],pSprite);
                        playerDist = min(playerDist, otherdist);
                    }
#endif

                    if (playerDist < spriteHitag && T1(spriteNum) == 0 && FX_VoiceAvailable(g_sounds[pSprite->lotag]->priority-1))
                    {
                        // Start playing an ambience sound.

                        char om = g_sounds[pSprite->lotag]->flags;
                        if (g_numEnvSoundsPlaying == ud.config.NumVoices)
                        {
                            int32_t j;

                            for (SPRITES_OF(STAT_FX, j))
                                if (j != spriteNum && S_IsAmbientSFX(j) && actor[j].t_data[0] == 1 &&
                                        dist(&sprite[j], &sprite[pPlayer->i]) > playerDist)
                                {
                                    S_StopEnvSound(sprite[j].lotag,j);
                                    break;
                                }

                            if (j == -1)
                                goto next_sprite;
                        }

                        g_sounds[pSprite->lotag]->flags |= SF_LOOP;
                        A_PlaySound(pSprite->lotag,spriteNum);
                        g_sounds[pSprite->lotag]->flags = om;
                        T1(spriteNum) = 1;  // AMBIENT_SFX_PLAYING
                    }
                    else if (playerDist >= spriteHitag && T1(spriteNum) == 1)
                    {
                        // Stop playing ambience sound because we're out of its range.

                        // T1 will be reset in sounds.c: CLEAR_SOUND_T0
                        // T1 = 0;
                        S_StopEnvSound(pSprite->lotag,spriteNum);
                    }
                }

                if ((g_sounds[pSprite->lotag]->flags & (SF_GLOBAL|SF_DTAG)) == SF_GLOBAL)
                {
                    // Randomly playing global sounds (flyby of planes, screams, ...)

                    if (T5(spriteNum) > 0)
                        T5(spriteNum)--;
                    else
                    {
                        for (int TRAVERSE_CONNECT(playerNum))
                            if (playerNum == myconnectindex && g_player[playerNum].ps->cursectnum == pSprite->sectnum)
                            {
                                S_PlaySound(pSprite->lotag + (unsigned)g_globalRandom % (pSprite->hitag+1));
                                T5(spriteNum) = GAMETICSPERSEC*40 + g_globalRandom%(GAMETICSPERSEC*40);
                            }
                    }
                }
            }
            break;
        }
        }
next_sprite:
        spriteNum = nextSprite;
    }
}

ACTOR_STATIC void G_MoveFallers(void)
{
    int spriteNum = headspritestat[STAT_FALLER];

    while (spriteNum >= 0)
    {
        int const  nextSprite = nextspritestat[spriteNum];
        auto const pSprite    = &sprite[spriteNum];
        int const  sectNum    = pSprite->sectnum;

        if (T1(spriteNum) == 0)
        {
            const int16_t oextra = pSprite->extra;
            int j;

            pSprite->z -= ZOFFSET2;
            T2(spriteNum) = pSprite->ang;

            if ((j = A_IncurDamage(spriteNum)) >= 0)
            {
                if (j == FIREEXT || j == RPG || j == RADIUSEXPLOSION || j == SEENINE || j == OOZFILTER)
                {
                    if (pSprite->extra <= 0)
                    {
                        T1(spriteNum) = 1;

                        for (bssize_t SPRITES_OF(STAT_FALLER, j))
                        {
                            if (sprite[j].hitag == SHT(spriteNum))
                            {
                                actor[j].t_data[0] = 1;
                                sprite[j].cstat &= (65535-64);
                                if (sprite[j].picnum == CEILINGSTEAM || sprite[j].picnum == STEAM)
                                    sprite[j].cstat |= 32768;
                            }
                        }
                    }
                }
                else
                {
                    actor[spriteNum].htextra = 0;
                    pSprite->extra = oextra;
                }
            }
            pSprite->ang = T2(spriteNum);
            pSprite->z += ZOFFSET2;
        }
        else if (T1(spriteNum) == 1)
        {
            if ((int16_t)pSprite->lotag > 0)
            {
                pSprite->lotag-=3;
                if ((int16_t)pSprite->lotag <= 0)
                {
                    pSprite->xvel = (32+(krand()&63));
                    pSprite->zvel = -(1024+(krand()&1023));
                }
            }
            else
            {
                int32_t spriteGravity = g_spriteGravity;

                if (pSprite->xvel > 0)
                {
                    pSprite->xvel -= 8;
                    A_SetSprite(spriteNum,CLIPMASK0);
                }

                if (EDUKE32_PREDICT_FALSE(G_CheckForSpaceFloor(pSprite->sectnum)))
                    spriteGravity = 0;
                else if (EDUKE32_PREDICT_FALSE(G_CheckForSpaceCeiling(pSprite->sectnum)))
                    spriteGravity = g_spriteGravity / 6;

                if (pSprite->z < (sector[sectNum].floorz - AC_FZOFFSET(spriteNum)))
                {
                    pSprite->zvel += spriteGravity;
                    if (pSprite->zvel > ACTOR_MAXFALLINGZVEL)
                        pSprite->zvel = ACTOR_MAXFALLINGZVEL;
                    pSprite->z += pSprite->zvel;
                }

                if ((sector[sectNum].floorz-pSprite->z) < ZOFFSET2)
                {
#ifndef EDUKE32_STANDALONE
                    for (int x = 0, x_end = 1+(krand()&7); x < x_end; ++x)
                        RANDOMSCRAP(pSprite, spriteNum);
#endif
                    DELETE_SPRITE_AND_CONTINUE(spriteNum);
                }
            }
        }

next_sprite:
        spriteNum = nextSprite;
    }
}

ACTOR_STATIC void G_MoveStandables(void)
{
    int spriteNum = headspritestat[STAT_STANDABLE], j, switchPic;

    while (spriteNum >= 0)
    {
        int const  nextSprite = nextspritestat[spriteNum];
        auto const pData      = &actor[spriteNum].t_data[0];
        auto const pSprite    = &sprite[spriteNum];
        int const  sectNum    = pSprite->sectnum;

        if (sectNum < 0)
            DELETE_SPRITE_AND_CONTINUE(spriteNum);

#ifndef EDUKE32_STANDALONE
        if (!FURY && PN(spriteNum) >= CRANE && PN(spriteNum) <= CRANE+3)
        {
            int32_t nextj;

            //t[0] = state
            //t[1] = checking sector number

            if (pSprite->xvel) A_GetZLimits(spriteNum);

            if (pData[0] == 0)   //Waiting to check the sector
            {
                for (SPRITES_OF_SECT_SAFE(pData[1], j, nextj))
                {
                    switch (sprite[j].statnum)
                    {
                        case STAT_ACTOR:
                        case STAT_ZOMBIEACTOR:
                        case STAT_STANDABLE:
                        case STAT_PLAYER:
                        {
                            vec3_t vect = { g_origins[pData[4]+1].x, g_origins[pData[4]+1].y, sprite[j].z };

                            pSprite->ang = getangle(vect.x-pSprite->x, vect.y-pSprite->y);
                            setsprite(j, &vect);
                            pData[0]++;
                            goto next_sprite;
                        }
                    }
                }
            }

            else if (pData[0]==1)
            {
                if (pSprite->xvel < 184)
                {
                    pSprite->picnum = CRANE+1;
                    pSprite->xvel += 8;
                }
                A_SetSprite(spriteNum,CLIPMASK0);
                if (sectNum == pData[1])
                    pData[0]++;
            }
            else if (pData[0]==2 || pData[0]==7)
            {
                pSprite->z += (1024+512);

                if (pData[0]==2)
                {
                    if (sector[sectNum].floorz - pSprite->z < (64<<8))
                        if (pSprite->picnum > CRANE) pSprite->picnum--;

                    if (sector[sectNum].floorz - pSprite->z < 4096+1024)
                        pData[0]++;
                }

                if (pData[0]==7)
                {
                    if (sector[sectNum].floorz - pSprite->z < (64<<8))
                    {
                        if (pSprite->picnum > CRANE) pSprite->picnum--;
                        else
                        {
                            if (pSprite->owner==-2)
                            {
                                int32_t p = A_FindPlayer(pSprite, NULL);
                                A_PlaySound(DUKE_GRUNT,g_player[p].ps->i);
                                if (g_player[p].ps->on_crane == spriteNum)
                                    g_player[p].ps->on_crane = -1;
                            }

                            pData[0]++;
                            pSprite->owner = -1;
                        }
                    }
                }
            }
            else if (pData[0]==3)
            {
                pSprite->picnum++;
                if (pSprite->picnum == CRANE+2)
                {
                    int32_t p = G_GetPlayerInSector(pData[1]);

                    if (p >= 0 && g_player[p].ps->on_ground)
                    {
                        pSprite->owner = -2;
                        g_player[p].ps->on_crane = spriteNum;
                        A_PlaySound(DUKE_GRUNT,g_player[p].ps->i);
                        g_player[p].smoothcamera = true;
                        g_player[p].ps->q16ang = fix16_from_int(pSprite->ang+1024);
                    }
                    else
                    {
                        for (SPRITES_OF_SECT(pData[1], j))
                        {
                            switch (sprite[j].statnum)
                            {
                            case STAT_ACTOR:
                            case STAT_STANDABLE:
                                pSprite->owner = j;
                                break;
                            }
                        }
                    }

                    pData[0]++;//Grabbed the sprite
                    pData[2]=0;
                    goto next_sprite;
                }
            }
            else if (pData[0]==4) //Delay before going up
            {
                pData[2]++;
                if (pData[2] > 10)
                    pData[0]++;
            }
            else if (pData[0]==5 || pData[0] == 8)
            {
                if (pData[0]==8 && pSprite->picnum < (CRANE+2))
                    if ((sector[sectNum].floorz-pSprite->z) > 8192)
                        pSprite->picnum++;

                if (pSprite->z < g_origins[pData[4]+2].x)
                {
                    pData[0]++;
                    pSprite->xvel = 0;
                }
                else
                    pSprite->z -= (1024+512);
            }
            else if (pData[0]==6)
            {
                if (pSprite->xvel < 192)
                    pSprite->xvel += 8;
                pSprite->ang = getangle(g_origins[pData[4]].x - pSprite->x, g_origins[pData[4]].y - pSprite->y);
                A_SetSprite(spriteNum,CLIPMASK0);
                if (((pSprite->x-g_origins[pData[4]].x)*(pSprite->x-g_origins[pData[4]].x)+(pSprite->y-g_origins[pData[4]].y)*(pSprite->y-g_origins[pData[4]].y)) < (128*128))
                    pData[0]++;
            }

            else if (pData[0]==9)
                pData[0] = 0;

            {
                vec3_t vect = pSprite->xyz;
                vect.z -= (34<<8);
                setsprite(g_origins[pData[4]+2].y, &vect);
            }


            if (pSprite->owner != -1)
            {
                int32_t p = A_FindPlayer(pSprite, NULL);

                if (A_IncurDamage(spriteNum) >= 0)
                {
                    if (pSprite->owner == -2)
                        if (g_player[p].ps->on_crane == spriteNum)
                            g_player[p].ps->on_crane = -1;
                    pSprite->owner = -1;
                    pSprite->picnum = CRANE;
                    goto next_sprite;
                }

                if (pSprite->owner >= 0)
                {
                    setsprite(pSprite->owner,&pSprite->xyz);

                    actor[pSprite->owner].bpos = pSprite->xyz;

                    pSprite->zvel = 0;
                }
                else if (pSprite->owner == -2)
                {
                    auto const ps = g_player[p].ps;

                    ps->opos.x = ps->pos.x = pSprite->x-(sintable[(fix16_to_int(ps->q16ang)+512)&2047]>>6);
                    ps->opos.y = ps->pos.y = pSprite->y-(sintable[fix16_to_int(ps->q16ang)&2047]>>6);
                    ps->opos.z = ps->pos.z = pSprite->z+(2<<8);

                    setsprite(ps->i, &ps->pos);
                    ps->cursectnum = sprite[ps->i].sectnum;
                }
            }

            goto next_sprite;
        }
        else if (!FURY && PN(spriteNum) >= WATERFOUNTAIN && PN(spriteNum) <= WATERFOUNTAIN+3)
        {
            if (pData[0] > 0)
            {
                if (pData[0] < 20)
                {
                    pData[0]++;

                    pSprite->picnum++;

                    if (pSprite->picnum == (WATERFOUNTAIN+3))
                        pSprite->picnum = WATERFOUNTAIN+1;
                }
                else
                {
                    int32_t playerDist;

                    A_FindPlayer(pSprite,&playerDist);

                    if (playerDist > 512)
                    {
                        pData[0] = 0;
                        pSprite->picnum = WATERFOUNTAIN;
                    }
                    else pData[0] = 1;
                }
            }
            goto next_sprite;
        }
        else if (!FURY && AFLAMABLE(pSprite->picnum))
        {
            if (T1(spriteNum) == 1)
            {
                if ((++T2(spriteNum)&3) > 0) goto next_sprite;

                if (pSprite->picnum == TIRE && T2(spriteNum) == 32)
                {
                    pSprite->cstat = 0;
                    j = A_Spawn(spriteNum,BLOODPOOL);
                    sprite[j].shade = 127;
                }
                else
                {
                    if (pSprite->shade < 64) pSprite->shade++;
                    else DELETE_SPRITE_AND_CONTINUE(spriteNum);
                }

                j = pSprite->xrepeat-(krand()&7);
                if (j < 10)
                    DELETE_SPRITE_AND_CONTINUE(spriteNum);

                pSprite->xrepeat = j;

                j = pSprite->yrepeat-(krand()&7);
                if (j < 4)
                    DELETE_SPRITE_AND_CONTINUE(spriteNum);

                pSprite->yrepeat = j;
            }
            if (pSprite->picnum == BOX)
            {
                A_Fall(spriteNum);
                actor[spriteNum].ceilingz = sector[pSprite->sectnum].ceilingz;
            }
            goto next_sprite;
        }
        else if (!FURY && pSprite->picnum == TRIPBOMB)
        {
            // TIMER_CONTROL
            if (actor[spriteNum].t_data[6] == 1)
            {

                if (actor[spriteNum].t_data[7] >= 1)
                {
                    actor[spriteNum].t_data[7]--;
                }

                if (actor[spriteNum].t_data[7] <= 0)
                {
                    T3(spriteNum)=16;
                    actor[spriteNum].t_data[6]=3;
                    A_PlaySound(LASERTRIP_ARMING,spriteNum);
                }
                // we're on a timer....
            }
            if (T3(spriteNum) > 0 && actor[spriteNum].t_data[6] == 3)
            {
                T3(spriteNum)--;

                if (T3(spriteNum) == 8)
                {
                    for (j=0; j<5; j++)
                        RANDOMSCRAP(pSprite, spriteNum);

                    int const dmg = pSprite->extra;
                    A_RadiusDamage(spriteNum, g_tripbombRadius, dmg>>2, dmg>>1, dmg-(dmg>>2), dmg);

                    j = A_Spawn(spriteNum,EXPLOSION2);
                    A_PlaySound(LASERTRIP_EXPLODE,j);
                    sprite[j].ang = pSprite->ang;
                    sprite[j].xvel = 348;
                    A_SetSprite(j,CLIPMASK0);

                    for (SPRITES_OF(STAT_MISC, j))
                    {
                        if (sprite[j].picnum == LASERLINE && pSprite->hitag == sprite[j].hitag)
                            sprite[j].xrepeat = sprite[j].yrepeat = 0;
                    }

                    DELETE_SPRITE_AND_CONTINUE(spriteNum);
                }
                goto next_sprite;
            }
            else
            {
                int const oldExtra = pSprite->extra;
                int const oldAng = pSprite->ang;

                pSprite->extra = 1;
                if (A_IncurDamage(spriteNum) >= 0)
                {
                    actor[spriteNum].t_data[6] = 3;
                    T3(spriteNum) = 16;
                }
                pSprite->extra = oldExtra;
                pSprite->ang = oldAng;
            }

            switch (T1(spriteNum))
            {
            default:
            {
                int32_t playerDist;
                A_FindPlayer(pSprite, &playerDist);
                if (playerDist > 768 || T1(spriteNum) > 16) T1(spriteNum)++;
                break;
            }

            case 32:
            {
                int16_t hitSprite;
                int const oldAng = pSprite->ang;

                pSprite->ang = T6(spriteNum);

                T4(spriteNum) = pSprite->x;
                T5(spriteNum) = pSprite->y;

                pSprite->x += sintable[(T6(spriteNum)+512)&2047]>>9;
                pSprite->y += sintable[(T6(spriteNum))&2047]>>9;
                pSprite->z -= (3<<8);

                int16_t const oldSectNum = pSprite->sectnum;
                int16_t       curSectNum = pSprite->sectnum;

                updatesectorneighbor(pSprite->x, pSprite->y, &curSectNum, 1024, 2048);
                changespritesect(spriteNum, curSectNum);

                int32_t hitDist = A_CheckHitSprite(spriteNum, &hitSprite);

                actor[spriteNum].lastv.x = hitDist;
                pSprite->ang = oldAng;

                // we're on a trip wire
                if (actor[spriteNum].t_data[6] != 1)
                {
                    while (hitDist > 0)
                    {
                        j = A_Spawn(spriteNum, LASERLINE);

                        sprite[j].hitag = pSprite->hitag;
                        actor[j].t_data[1] = sprite[j].z;

                        if (hitDist < 1024)
                        {
                            sprite[j].xrepeat = hitDist>>5;
                            break;
                        }
                        hitDist -= 1024;

                        pSprite->x += sintable[(T6(spriteNum)+512)&2047]>>4;
                        pSprite->y += sintable[(T6(spriteNum))&2047]>>4;

                        updatesectorneighbor(pSprite->x, pSprite->y, &curSectNum, 1024, 2048);

                        if (curSectNum == -1)
                            break;

                        changespritesect(spriteNum, curSectNum);

                        // this is a hack to work around the LASERLINE sprite's art tile offset
                        changespritesect(j, curSectNum);
                    }
                }

                T1(spriteNum)++;

                pSprite->xy = { T4(spriteNum), T5(spriteNum) };
                pSprite->z += (3<<8);

                changespritesect(spriteNum, oldSectNum);
                T4(spriteNum) = T3(spriteNum) = 0;

                if (hitSprite >= 0 && actor[spriteNum].t_data[6] != 1)
                {
                    actor[spriteNum].t_data[6] = 3;
                    T3(spriteNum) = 13;
                    A_PlaySound(LASERTRIP_ARMING,spriteNum);
                }
                break;
            }

            case 33:
            {
                T2(spriteNum)++;

                T4(spriteNum) = pSprite->x;
                T5(spriteNum) = pSprite->y;

                pSprite->x += sintable[(T6(spriteNum)+512)&2047]>>9;
                pSprite->y += sintable[(T6(spriteNum))&2047]>>9;
                pSprite->z -= (3<<8);

                setsprite(spriteNum, &pSprite->xyz);

                int32_t const hitDist = A_CheckHitSprite(spriteNum, NULL);

                pSprite->xy = { T4(spriteNum), T5(spriteNum) };
                pSprite->z += (3<<8);
                setsprite(spriteNum, &pSprite->xyz);

                //                if( Actor[i].lastvx != x && lTripBombControl & TRIPBOMB_TRIPWIRE)
                if (actor[spriteNum].lastv.x != hitDist && actor[spriteNum].t_data[6] != 1)
                {
                    actor[spriteNum].t_data[6] = 3;
                    T3(spriteNum) = 13;
                    A_PlaySound(LASERTRIP_ARMING, spriteNum);
                }
                break;
            }
            }

            goto next_sprite;
        }
        else if (!FURY && pSprite->picnum >= CRACK1 && pSprite->picnum <= CRACK4)
        {
            if (pSprite->hitag)
            {
                pData[0] = pSprite->cstat;
                pData[1] = pSprite->ang;

                int const dmgTile = A_IncurDamage(spriteNum);

                if (dmgTile < 0)
                    goto crack_default;

                switch (tileGetMapping(dmgTile))
                {
                    case FIREEXT__:
                    case RPG__:
                    case RADIUSEXPLOSION__:
                    case SEENINE__:
                    case OOZFILTER__:
                        for (SPRITES_OF(STAT_STANDABLE, j))
                        {
                            if (pSprite->hitag == sprite[j].hitag &&
                                (sprite[j].picnum == OOZFILTER || sprite[j].picnum == SEENINE))
                                if (sprite[j].shade != -32)
                                    sprite[j].shade = -32;
                        }

                        goto DETONATE;

crack_default:
                    default:
                        pSprite->cstat = pData[0];
                        pSprite->ang   = pData[1];
                        pSprite->extra = 0;

                        goto next_sprite;
                }
            }
            goto next_sprite;
        }
        else if (!FURY && pSprite->picnum == FIREEXT)
        {
            if (A_IncurDamage(spriteNum) < 0)
                goto next_sprite;

            for (int k=0; k<16; k++)
            {
                j = A_InsertSprite(SECT(spriteNum), SX(spriteNum), SY(spriteNum), SZ(spriteNum) - (krand() % (48 << 8)),
                                   SCRAP3 + (krand() & 3), -8, 48, 48, krand() & 2047, (krand() & 63) + 64,
                                   -(krand() & 4095) - (sprite[spriteNum].zvel >> 2), spriteNum, 5);

                sprite[j].pal = 2;
            }

            j = A_Spawn(spriteNum,EXPLOSION2);
            A_PlaySound(PIPEBOMB_EXPLODE,j);
            A_PlaySound(GLASS_HEAVYBREAK,j);

            if ((int16_t)pSprite->hitag > 0)
            {
                for (SPRITES_OF(STAT_STANDABLE, j))
                {
                    // XXX: This block seems to be CODEDUP'd a lot of times.
                    if (pSprite->hitag == sprite[j].hitag && (sprite[j].picnum == OOZFILTER || sprite[j].picnum == SEENINE))
                        if (sprite[j].shade != -32)
                            sprite[j].shade = -32;
                }

                int const dmg = pSprite->extra;
                A_RadiusDamage(spriteNum, g_pipebombRadius,dmg>>2, dmg-(dmg>>1),dmg-(dmg>>2), dmg);
                j = A_Spawn(spriteNum,EXPLOSION2);
                A_PlaySound(PIPEBOMB_EXPLODE,j);

                goto DETONATE;
            }
            else
            {
                A_RadiusDamage(spriteNum,g_seenineRadius,10,15,20,25);
                DELETE_SPRITE_AND_CONTINUE(spriteNum);
            }
            goto next_sprite;
        }
        else
#endif
            if (pSprite->picnum == OOZFILTER || pSprite->picnum == SEENINE || pSprite->picnum == SEENINEDEAD || pSprite->picnum == SEENINEDEAD+1)
        {
            if (pSprite->shade != -32 && pSprite->shade != -33)
            {
                if (pSprite->xrepeat)
                    j = (A_IncurDamage(spriteNum) >= 0);
                else
                    j = 0;

                if (j || pSprite->shade == -31)
                {
                    if (j) pSprite->lotag = 0;

                    pData[3] = 1;

                    for (SPRITES_OF(STAT_STANDABLE, j))
                    {
                        if (pSprite->hitag == sprite[j].hitag && (sprite[j].picnum == SEENINE || sprite[j].picnum == OOZFILTER))
                            sprite[j].shade = -32;
                    }
                }
            }
            else
            {
                if (pSprite->shade == -32)
                {
                    if ((int16_t)pSprite->lotag > 0)
                    {
                        pSprite->lotag -= 3;
                        if ((int16_t)pSprite->lotag <= 0)
                            pSprite->lotag = -99;
                    }
                    else
                        pSprite->shade = -33;
                }
                else
                {
                    if (pSprite->xrepeat > 0)
                    {
                        T3(spriteNum)++;
                        if (T3(spriteNum) == 3)
                        {
                            if (pSprite->picnum == OOZFILTER)
                            {
                                T3(spriteNum) = 0;
                                goto DETONATE;
                            }

                            if (pSprite->picnum != (SEENINEDEAD+1))
                            {
                                T3(spriteNum) = 0;

                                if (pSprite->picnum == SEENINEDEAD)
                                    pSprite->picnum++;
                                else if (pSprite->picnum == SEENINE)
                                    pSprite->picnum = SEENINEDEAD;
                            }
                            else goto DETONATE;
                        }
                        goto next_sprite;
                    }

DETONATE:
                    g_earthquakeTime = 16;

                    for (SPRITES_OF(STAT_EFFECTOR, j))
                    {
                        if (pSprite->hitag == sprite[j].hitag)
                        {
                            if (sprite[j].lotag == SE_13_EXPLOSIVE)
                            {
                                if (actor[j].t_data[2] == 0)
                                    actor[j].t_data[2] = 1;
                            }
                            else if (sprite[j].lotag == SE_8_UP_OPEN_DOOR_LIGHTS)
                                actor[j].t_data[4] = 1;
                            else if (sprite[j].lotag == SE_18_INCREMENTAL_SECTOR_RISE_FALL)
                            {
                                if (actor[j].t_data[0] == 0)
                                    actor[j].t_data[0] = 1;
                            }
                            else if (sprite[j].lotag == SE_21_DROP_FLOOR)
                                actor[j].t_data[0] = 1;
                        }
                    }

                    pSprite->z -= ZOFFSET5;

#ifndef EDUKE32_STANDALONE
                    if (!FURY && pSprite->xrepeat)
                        for (int x=0; x<8; x++)
                            RANDOMSCRAP(pSprite, spriteNum);
#endif

                    if ((pData[3] == 1 && pSprite->xrepeat) || (int16_t)pSprite->lotag == -99)
                    {
                        int const newSprite = A_Spawn(spriteNum,EXPLOSION2);
                        int const dmg = pSprite->extra;

                        A_RadiusDamage(spriteNum,g_seenineRadius,dmg>>2, dmg-(dmg>>1),dmg-(dmg>>2), dmg);
                        A_PlaySound(PIPEBOMB_EXPLODE, newSprite);
                    }

                    DELETE_SPRITE_AND_CONTINUE(spriteNum);
                }
            }
            goto next_sprite;
        }
        else if (pSprite->picnum == MASTERSWITCH)
        {
            if (pSprite->yvel == 1)
            {
                if ((int16_t)--pSprite->hitag <= 0)
                {
                    G_OperateSectors(sectNum,spriteNum);

                    for (SPRITES_OF_SECT(sectNum, j))
                    {
                        if (sprite[j].statnum == STAT_EFFECTOR)
                        {
                            switch (sprite[j].lotag)
                            {
                            case SE_2_EARTHQUAKE:
                            case SE_21_DROP_FLOOR:
                            case SE_31_FLOOR_RISE_FALL:
                            case SE_32_CEILING_RISE_FALL:
                            case SE_36_PROJ_SHOOTER:
                                actor[j].t_data[0] = 1;
                                break;
                            case SE_3_RANDOM_LIGHTS_AFTER_SHOT_OUT:
                                actor[j].t_data[4] = 1;
                                break;
                            }
                        }
                        else if (sprite[j].statnum == STAT_STANDABLE)
                        {
                            switch (tileGetMapping(sprite[j].picnum))
                            {
                            case SEENINE__:
                            case OOZFILTER__:
                                sprite[j].shade = -31;
                                break;
                            }
                        }
                    }

                    if (pSprite->pal == 23)
                    {
                        pSprite->yvel = 0;
                        pSprite->hitag = pSprite->extra;
                        goto next_sprite;
                    }

                    DELETE_SPRITE_AND_CONTINUE(spriteNum);
                }
            }
            goto next_sprite;
        }
        else
        {
            switchPic = pSprite->picnum;

#ifndef EDUKE32_STANDALONE
            if (!FURY)
            {
                if (switchPic > SIDEBOLT1 && switchPic <= SIDEBOLT1 + 3)
                    switchPic = SIDEBOLT1;
                else if (switchPic > BOLT1 && switchPic <= BOLT1 + 3)
                    switchPic = BOLT1;
            }
#endif
            switch (tileGetMapping(switchPic))
            {
                case TOUCHPLATE__:
                    if (pData[1] == 1 && (int16_t)pSprite->hitag >= 0)  // Move the sector floor
                    {
                        int const floorZ = sector[sectNum].floorz;

                        if (pData[3] == 1)
                        {
                            if (floorZ >= pData[2])
                            {
                                sector[sectNum].floorz = floorZ;
                                pData[1]               = 0;
                            }
                            else
                            {
                                sector[sectNum].floorz += sector[sectNum].extra;
                                int const playerNum = G_GetPlayerInSector(sectNum);
                                if (playerNum >= 0)
                                    g_player[playerNum].ps->pos.z += sector[sectNum].extra;
                            }
                        }
                        else
                        {
                            if (floorZ <= pSprite->z)
                            {
                                sector[sectNum].floorz = pSprite->z;
                                pData[1]               = 0;
                            }
                            else
                            {
                                int32_t p;
                                sector[sectNum].floorz -= sector[sectNum].extra;
                                p = G_GetPlayerInSector(sectNum);
                                if (p >= 0)
                                    g_player[p].ps->pos.z -= sector[sectNum].extra;
                            }
                        }
                        goto next_sprite;
                    }

                    if (pData[5] == 1)
                        goto next_sprite;

                    {
                        int32_t p = G_GetPlayerInSector(sectNum);

                        if (p >= 0 && (g_player[p].ps->on_ground || pSprite->ang == 512))
                        {
                            if (pData[0] == 0 && !G_CheckActivatorMotion(pSprite->lotag))
                            {
                                pData[0] = 1;
                                pData[1] = 1;
                                pData[3] = !pData[3];
                                G_OperateMasterSwitches(pSprite->lotag);
                                G_OperateActivators(pSprite->lotag, p);
                                if ((int16_t)pSprite->hitag > 0)
                                {
                                    pSprite->hitag--;
                                    if (pSprite->hitag == 0)
                                        pData[5] = 1;
                                }
                            }
                        }
                        else
                            pData[0] = 0;
                    }

                    if (pData[1] == 1)
                    {
                        for (SPRITES_OF(STAT_STANDABLE, j))
                        {
                            if (j != spriteNum && sprite[j].picnum == TOUCHPLATE && sprite[j].lotag == pSprite->lotag)
                            {
                                actor[j].t_data[1] = 1;
                                actor[j].t_data[3] = pData[3];
                            }
                        }
                    }
                    goto next_sprite;

                case VIEWSCREEN__:
                case VIEWSCREEN2__:

                    if (pSprite->xrepeat == 0)
                        DELETE_SPRITE_AND_CONTINUE(spriteNum);

                    {
                        int32_t    playerDist;
                        // T2(spriteNum) contains index of viewscreen in g_activeViewscreenSprite
                        int const  curVscrIndex = T2(spriteNum);
                        int const  p  = A_FindPlayer(pSprite, &playerDist);
                        auto const ps = g_player[p].ps;

                        // EDuke32 extension: xvel of viewscreen determines active distance
                        int const activeDist = pSprite->xvel > 0 ? pSprite->xvel : VIEWSCREEN_ACTIVE_DISTANCE;

                        if (dist(&sprite[ps->i], pSprite) < activeDist)
                        {
                            // DOS behavior: yvel of 1 activates screen if player approaches
                            if (curVscrIndex == -1 && pSprite->yvel & 1)
                            {
                                int newVscrIndex = 0;
                                while (newVscrIndex < MAX_ACTIVE_VIEWSCREENS && g_activeVscrSprite[newVscrIndex] >= 0)
                                    newVscrIndex++;

                                T2(spriteNum) = newVscrIndex;
                                if (newVscrIndex < MAX_ACTIVE_VIEWSCREENS)
                                {
                                    g_activeVscrSprite[newVscrIndex] = spriteNum;

                                    // EDuke32 extension: for remote activation, check for connected camera and display its image
                                    if (sprite[spriteNum].hitag)
                                    {
                                        for (bssize_t SPRITES_OF(STAT_ACTOR, otherSprite))
                                        {
                                            if (PN(otherSprite) == CAMERA1 && sprite[spriteNum].hitag == SLT(otherSprite))
                                            {
                                                sprite[spriteNum].owner = otherSprite;
                                                break;
                                            }
                                        }
                                    }
                                }
                            }
                            // deactivate viewscreen in valid range if yvel is set to 0
                            else if (curVscrIndex != -1 && !(pSprite->yvel & 3))
                            {
                                T1(spriteNum) = 0;
                                T2(spriteNum) = -1;

                                if (curVscrIndex < MAX_ACTIVE_VIEWSCREENS)
                                {
                                    if (g_activeVscrTile[curVscrIndex] >= 0)
                                        walock[g_activeVscrTile[curVscrIndex]] = CACHE1D_UNLOCKED;

                                    g_activeVscrSprite[curVscrIndex] = -1;
                                    g_activeVscrTile[curVscrIndex] = -1;
                                }
                            }
                        }
                        else if (curVscrIndex != -1)
                        {
                            T1(spriteNum) = 0;
                            T2(spriteNum) = -1;
                            pSprite->yvel &= ~2; // yvel bit 2 is temp activation

                            if (curVscrIndex < MAX_ACTIVE_VIEWSCREENS)
                            {
                                if (g_activeVscrTile[curVscrIndex] >= 0)
                                    walock[g_activeVscrTile[curVscrIndex]] = CACHE1D_UNLOCKED;

                                g_activeVscrSprite[curVscrIndex] = -1;
                                g_activeVscrTile[curVscrIndex] = -1;
                            }
                        }
                    }

                    goto next_sprite;
            }
#ifndef EDUKE32_STANDALONE
            if (!FURY)
            switch (tileGetMapping(switchPic))
            {
                case TRASH__:

                    if (pSprite->xvel == 0)
                        pSprite->xvel = 1;
                    if (A_SetSprite(spriteNum, CLIPMASK0))
                    {
                        A_Fall(spriteNum);
                        if (krand() & 1)
                            pSprite->zvel -= 256;
                        if ((pSprite->xvel) < 48)
                            pSprite->xvel += (krand() & 3);
                    }
                    else
                        DELETE_SPRITE_AND_CONTINUE(spriteNum);
                    break;

                case SIDEBOLT1__:
                    //        case SIDEBOLT1+1:
                    //        case SIDEBOLT1+2:
                    //        case SIDEBOLT1+3:
                {
                    int32_t playerDist;
                    A_FindPlayer(pSprite, &playerDist);
                    if (playerDist > 20480)
                        goto next_sprite;

                CLEAR_THE_BOLT2:
                    if (pData[2])
                    {
                        pData[2]--;
                        goto next_sprite;
                    }
                    if ((pSprite->xrepeat | pSprite->yrepeat) == 0)
                    {
                        pSprite->xrepeat = pData[0];
                        pSprite->yrepeat = pData[1];
                    }
                    if ((krand() & 8) == 0)
                    {
                        pData[0]         = pSprite->xrepeat;
                        pData[1]         = pSprite->yrepeat;
                        pData[2]         = g_globalRandom & 4;
                        pSprite->xrepeat = pSprite->yrepeat = 0;
                        goto CLEAR_THE_BOLT2;
                    }
                    pSprite->picnum++;

#if 0
                    // NOTE: Um, this 'l' was assigned to last at the beginning of this function.
                    // SIDEBOLT1 never gets translucent as a consequence, unlike BOLT1.
                    if (randomRepeat & 1)
                        pSprite->cstat ^= 2;
#endif

                    if ((krand() & 1) && sector[sectNum].floorpicnum == HURTRAIL)
                        A_PlaySound(SHORT_CIRCUIT, spriteNum);

                    if (pSprite->picnum == SIDEBOLT1 + 4)
                        pSprite->picnum = SIDEBOLT1;

                    goto next_sprite;
                }

                case BOLT1__:
                    //        case BOLT1+1:
                    //        case BOLT1+2:
                    //        case BOLT1+3:
                {
                    int32_t playerDist;
                    A_FindPlayer(pSprite, &playerDist);
                    if (playerDist > 20480)
                        goto next_sprite;

                    if (pData[3] == 0)
                        pData[3] = sector[sectNum].floorshade;

                CLEAR_THE_BOLT:
                    if (pData[2])
                    {
                        pData[2]--;
                        sector[sectNum].floorshade   = 20;
                        sector[sectNum].ceilingshade = 20;
                        goto next_sprite;
                    }
                    if ((pSprite->xrepeat | pSprite->yrepeat) == 0)
                    {
                        pSprite->xrepeat = pData[0];
                        pSprite->yrepeat = pData[1];
                    }
                    else if ((krand() & 8) == 0)
                    {
                        pData[0]         = pSprite->xrepeat;
                        pData[1]         = pSprite->yrepeat;
                        pData[2]         = g_globalRandom & 4;
                        pSprite->xrepeat = pSprite->yrepeat = 0;
                        goto CLEAR_THE_BOLT;
                    }
                    pSprite->picnum++;

                    int const randomRepeat = g_globalRandom & 7;
                    pSprite->xrepeat = randomRepeat + 8;

                    if (randomRepeat & 1)
                        pSprite->cstat ^= 2;

                    if (pSprite->picnum == (BOLT1 + 1)
                        && (krand() & 7) == 0 && sector[sectNum].floorpicnum == HURTRAIL)
                        A_PlaySound(SHORT_CIRCUIT, spriteNum);

                    if (pSprite->picnum == BOLT1 + 4)
                        pSprite->picnum = BOLT1;

                    if (pSprite->picnum & 1)
                    {
                        sector[sectNum].floorshade   = 0;
                        sector[sectNum].ceilingshade = 0;
                    }
                    else
                    {
                        sector[sectNum].floorshade   = 20;
                        sector[sectNum].ceilingshade = 20;
                    }
                    goto next_sprite;
                }

                case WATERDRIP__:

                    if (pData[1])
                    {
                        if (--pData[1] == 0)
                            pSprite->cstat &= 32767;
                    }
                    else
                    {
                        A_Fall(spriteNum);
                        A_SetSprite(spriteNum, CLIPMASK0);
                        if (pSprite->xvel > 0)
                            pSprite->xvel -= 2;

                        if (pSprite->zvel == 0)
                        {
                            pSprite->cstat |= 32768;

                            if (pSprite->pal != 2 && pSprite->hitag == 0)
                                A_PlaySound(SOMETHING_DRIPPING, spriteNum);

                            if (sprite[pSprite->owner].picnum != WATERDRIP)
                            {
                                DELETE_SPRITE_AND_CONTINUE(spriteNum);
                            }
                            else
                            {
                                actor[spriteNum].bpos.z = pSprite->z = pData[0];
                                pData[1]                             = 48 + (krand() & 31);
                            }
                        }
                    }


                    goto next_sprite;

                case DOORSHOCK__:
                    pSprite->yrepeat = (klabs(sector[sectNum].ceilingz - sector[sectNum].floorz) >> 9) + 4;
                    pSprite->xrepeat = 16;
                    pSprite->z       = sector[sectNum].floorz;
                    goto next_sprite;

                case CANWITHSOMETHING__:
                case CANWITHSOMETHING2__:
                case CANWITHSOMETHING3__:
                case CANWITHSOMETHING4__:
                    A_Fall(spriteNum);
                    if (A_IncurDamage(spriteNum) >= 0)
                    {
                        A_PlaySound(VENT_BUST, spriteNum);

                        for (j = 9; j >= 0; j--) RANDOMSCRAP(pSprite, spriteNum);

                        if (pSprite->lotag)
                            A_Spawn(spriteNum, pSprite->lotag);

                        DELETE_SPRITE_AND_CONTINUE(spriteNum);
                    }
                    goto next_sprite;

                case FLOORFLAME__:
                case FIREBARREL__:
                case FIREVASE__:
                case EXPLODINGBARREL__:
                case WOODENHORSE__:
                case HORSEONSIDE__:
                case NUKEBARREL__:
                case NUKEBARRELDENTED__:
                case NUKEBARRELLEAKED__:
                case TOILETWATER__:
                case RUBBERCAN__:
                case STEAM__:
                case CEILINGSTEAM__:
                case WATERBUBBLEMAKER__:
                    if (!G_TileHasActor(sprite[spriteNum].picnum))
                        goto next_sprite;
                    {
                        int32_t playerDist;
                        int const playerNum = A_FindPlayer(pSprite, &playerDist);
                        A_Execute(spriteNum, playerNum, playerDist);
                    }
                    goto next_sprite;
            }
#endif
        }

    next_sprite:
        spriteNum = nextSprite;
    }
}

ACTOR_STATIC void A_DoProjectileBounce(int const spriteNum)
{
    auto const pSprite = &sprite[spriteNum];
    int32_t const hitSectnum = pSprite->sectnum;
    int const firstWall  = sector[hitSectnum].wallptr;
    int const secondWall = wall[firstWall].point2;
    int const wallAngle  = getangle(wall[secondWall].x - wall[firstWall].x, wall[secondWall].y - wall[firstWall].y);
    vec3_t    vect       = { mulscale10(pSprite->xvel, sintable[(pSprite->ang + 512) & 2047]),
                                mulscale10(pSprite->xvel, sintable[pSprite->ang & 2047]), pSprite->zvel };

    int k = (pSprite->z<(actor[spriteNum].floorz + actor[spriteNum].ceilingz)>> 1) ? sector[hitSectnum].ceilingheinum
                                                                   : sector[hitSectnum].floorheinum;

    vec3_t const da = { mulscale14(k, sintable[(wallAngle)&2047]),
                        mulscale14(k, sintable[(wallAngle + 1536) & 2047]), 4096 };

    k     = vect.x * da.x + vect.y * da.y + vect.z * da.z;
    int l = da.x * da.x + da.y * da.y + da.z * da.z;

    if ((klabs(k) >> 14) < l)
    {
        k = divscale17(k, l);
        vect.x -= mulscale16(da.x, k);
        vect.y -= mulscale16(da.y, k);
        vect.z -= mulscale16(da.z, k);
    }

    pSprite->zvel = vect.z;
    pSprite->xvel = ksqrt(dmulscale8(vect.x, vect.x, vect.y, vect.y));
    pSprite->ang = getangle(vect.x, vect.y);
}

#ifndef EDUKE32_STANDALONE
ACTOR_STATIC void P_HandleBeingSpitOn(DukePlayer_t * const ps)
{
    ps->q16horiz += F16(32);
    ps->return_to_center = 8;

    if (ps->loogcnt)
        return;

    if (!A_CheckSoundPlaying(ps->i, DUKE_LONGTERM_PAIN))
        A_PlaySound(DUKE_LONGTERM_PAIN,ps->i);

    int j = 3+(krand()&3);
    ps->numloogs = j;
    ps->loogcnt = 24*4;
    for (int x=0; x < j; x++)
        ps->loogie[x] = { (int16_t)(krand()%320), (int16_t)(krand()%200) };
}
#endif

static FORCE_INLINE vec2_t Proj_GetOffset(int const spriteNum)
{
    return { sprite[spriteNum].xrepeat * (sintable[(sprite[spriteNum].ang + 512) & 2047] >> 11),
             sprite[spriteNum].xrepeat * (sintable[sprite[spriteNum].ang & 2047] >> 11) };
}

static void A_DoProjectileEffects(int spriteNum, bool radiusDamage = true)
{
    auto const pProj = &SpriteProjectile[spriteNum];

    if (pProj->spawns >= 0)
    {
        int const newSpr = A_Spawn(spriteNum,pProj->spawns);

        if (pProj->sxrepeat > 4)
            sprite[newSpr].xrepeat=pProj->sxrepeat;
        if (pProj->syrepeat > 4)
            sprite[newSpr].yrepeat=pProj->syrepeat;

        sprite[newSpr].xy -= Proj_GetOffset(newSpr);
    }

    if (pProj->isound >= 0)
        A_PlaySound(pProj->isound,spriteNum);

    if (!radiusDamage)
        return;

    auto const pSprite = &sprite[spriteNum];
    pSprite->extra = Proj_GetDamage(pProj);
    int const dmg = pSprite->extra;
    A_RadiusDamage(spriteNum, pProj->hitradius, dmg >> 2, dmg >> 1, dmg - (dmg >> 2), dmg);
}

static void G_WeaponHitCeilingOrFloor(int32_t i, spritetype *s, int *j)
{
    if (actor[i].flags & SFLAG_DIDNOSE7WATER)
    {
        actor[i].flags &= ~SFLAG_DIDNOSE7WATER;
        return;
    }

    if (s->z < actor[i].ceilingz)
    {
        *j = 16384|s->sectnum;
        s->zvel = -1;
    }
    else if (s->z > actor[i].floorz + ZOFFSET2*(sector[s->sectnum].lotag == ST_1_ABOVE_WATER))
    {
        *j = 16384|s->sectnum;

        if (sector[s->sectnum].lotag != ST_1_ABOVE_WATER)
            s->zvel = 1;
    }
}

static void Proj_BounceOffWall(spritetype *s, int j)
{
    int k = getangle(
        wall[wall[j].point2].x-wall[j].x,
        wall[wall[j].point2].y-wall[j].y);
    s->ang = ((k<<1) - s->ang)&2047;
}

#define PROJ_DECAYVELOCITY(s) s->xvel >>= 1, s->zvel >>= 1

// Maybe damage a ceiling or floor as the consequence of projectile impact.
// Returns 1 if sprite <s> should be killed.
// NOTE: Compare with Proj_MaybeDamageCF2() in sector.c
static int Proj_MaybeDamageCF(int spriteNum)
{
    auto const s = (uspriteptr_t)&sprite[spriteNum];

    if (s->zvel < 0)
    {
        if ((sector[s->sectnum].ceilingstat&1) && sector[s->sectnum].ceilingpal == 0)
            return 1;

        Sect_DamageCeiling(spriteNum, s->sectnum);
    }
    else if (s->zvel > 0)
    {
        if ((sector[s->sectnum].floorstat&1) && sector[s->sectnum].floorpal == 0)
        {
            // Keep original Duke3D behavior: pass projectiles through
            // parallaxed ceilings, but NOT through such floors.
            return 0;
        }

        Sect_DamageFloor(spriteNum, s->sectnum);
    }

    return 0;
}

ACTOR_STATIC void Proj_MoveCustom(int const spriteNum)
{
    int projectileMoved = SpriteProjectile[spriteNum].workslike & PROJECTILE_MOVED;
    SpriteProjectile[spriteNum].workslike |= PROJECTILE_MOVED;

    auto const pProj   = &SpriteProjectile[spriteNum];
    auto const pSprite = &sprite[spriteNum];
    int        otherSprite = 0;

    switch (pProj->workslike & PROJECTILE_TYPE_MASK)
    {
        case PROJECTILE_HITSCAN:
        {
            if (!G_TileHasActor(sprite[spriteNum].picnum))
                return;
            int32_t   playerDist;
            int const playerNum = A_FindPlayer(pSprite, &playerDist);
            A_Execute(spriteNum, playerNum, playerDist);
            return;
        }

        case PROJECTILE_KNEE:
        case PROJECTILE_BLOOD: A_DeleteSprite(spriteNum); return;

        default:
        case PROJECTILE_RPG:
        {
            VM_UpdateAnim(spriteNum, &actor[spriteNum].t_data[0]);

            if (pProj->flashcolor)
                G_AddGameLight(spriteNum, pSprite->sectnum, { 0, 0, ((pSprite->yrepeat * tilesiz[pSprite->picnum].y) << 1)}, 2048,
 0, 100,
                               pProj->flashcolor, PR_LIGHT_PRIO_LOW_GAME);

            if (pProj->workslike & (PROJECTILE_BOUNCESOFFWALLS|PROJECTILE_BOUNCESOFFSPRITES) && pSprite->yvel < 1)
            {
                if ((pProj->workslike & PROJECTILE_EXPLODEONTIMER) == 0)
                {
                    A_DoProjectileEffects(spriteNum);
                    A_DeleteSprite(spriteNum);
                    return;
                }
                else if (pProj->workslike & PROJECTILE_LOSESVELOCITY)
                    PROJ_DECAYVELOCITY(pSprite);
            }

            if (pProj->workslike & PROJECTILE_COOLEXPLOSION1 && ++pSprite->shade >= 40)
            {
                A_DeleteSprite(spriteNum);
                return;
            }

            pSprite->zvel -= pProj->drop;

            if (pProj->workslike & PROJECTILE_SPIT && pSprite->zvel < ACTOR_MAXFALLINGZVEL)
                pSprite->zvel += g_spriteGravity - 112;

            A_GetZLimits(spriteNum);

            if (pProj->trail >= 0)
            {
                for (bssize_t cnt = 0; cnt <= pProj->tnum; cnt++)
                {
                    otherSprite = A_Spawn(spriteNum, pProj->trail);

                    sprite[otherSprite].z += (pProj->toffset << 8);

                    if (pProj->txrepeat >= 0)
                        sprite[otherSprite].xrepeat = pProj->txrepeat;

                    if (pProj->tyrepeat >= 0)
                        sprite[otherSprite].yrepeat = pProj->tyrepeat;
                }
            }

            int projMoveCnt = pProj->movecnt;
            int projVel     = pSprite->xvel;
            int projZvel    = pSprite->zvel;

            if (sector[pSprite->sectnum].lotag == ST_2_UNDERWATER)
            {
                projVel >>= 1;
                projZvel >>= 1;
            }

            int const origXvel = projVel;

            if (!projectileMoved)
                projVel += sprite[pSprite->owner].xvel;

            do
            {
                otherSprite = A_MoveSprite(spriteNum, { (projVel * (sintable[(pSprite->ang + 512) & 2047])) >> 14 >> (int)!projectileMoved,
                                                        (projVel * (sintable[pSprite->ang & 2047])) >> 14 >> (int)!projectileMoved, projZvel >> (int)!projectileMoved },
                                                        (A_CheckSpriteFlags(spriteNum, SFLAG_NOCLIP) ? 0 : CLIPMASK1));
                if (!projectileMoved && ldist(pSprite, &sprite[pSprite->owner]) > A_GetClipdist(spriteNum))
                    projectileMoved++, projVel = origXvel;
            }
            while (!otherSprite && --projMoveCnt > 0);

            if (!(pProj->workslike & PROJECTILE_BOUNCESOFFWALLS) &&  // NOT_BOUNCESOFFWALLS_YVEL
                (unsigned)pSprite->yvel < MAXSPRITES
                && sprite[pSprite->yvel].sectnum != MAXSECTORS)
                if (FindDistance2D(pSprite->x - sprite[pSprite->yvel].x, pSprite->y - sprite[pSprite->yvel].y) < 256)
                    otherSprite = 49152 | pSprite->yvel;

            actor[spriteNum].movflag = otherSprite;

            if (pSprite->sectnum < 0)
            {
                A_DeleteSprite(spriteNum);
                return;
            }

            if (pProj->workslike & PROJECTILE_TIMED && pProj->range > 0)
            {
                if (++actor[spriteNum].t_data[8] > pProj->range)
                {
                    if (pProj->workslike & PROJECTILE_EXPLODEONTIMER)
                        A_DoProjectileEffects(spriteNum);

                    A_DeleteSprite(spriteNum);
                    return;
                }
            }

            if ((otherSprite & 49152) != 49152 && !(pProj->workslike & PROJECTILE_BOUNCESOFFWALLS))
                G_WeaponHitCeilingOrFloor(spriteNum, pSprite, &otherSprite);

            if (pProj->workslike & PROJECTILE_WATERBUBBLES && sector[pSprite->sectnum].lotag == ST_2_UNDERWATER && rnd(140))
                A_Spawn(spriteNum, WATERBUBBLE);

            if (otherSprite != 0)
            {
                if (pProj->workslike & PROJECTILE_COOLEXPLOSION1)
                {
                    pSprite->xvel = 0;
                    pSprite->zvel = 0;
                }

                switch (otherSprite & 49152)
                {
                    case 49152:
                        otherSprite &= (MAXSPRITES - 1);

                        if (pProj->workslike & PROJECTILE_BOUNCESOFFSPRITES && pSprite->yvel > 0)
                        {
                            pSprite->yvel--;

                            int const projAngle = getangle(sprite[otherSprite].x - pSprite->x, sprite[otherSprite].y - pSprite->y)
                                                  + ((sprite[otherSprite].cstat & 16) ? 0 : 512);
                            pSprite->ang = ((projAngle << 1) - pSprite->ang) & 2047;

                            if (pProj->bsound >= 0)
                                A_PlaySound(pProj->bsound, spriteNum);

                            if (pProj->workslike & PROJECTILE_LOSESVELOCITY)
                                PROJ_DECAYVELOCITY(pSprite);

                            if (!(pProj->workslike & PROJECTILE_FORCEIMPACT))
                                return;
                        }

                        A_DamageObject(otherSprite, spriteNum);

#ifndef EDUKE32_STANDALONE
                        if (sprite[otherSprite].picnum == APLAYER)
                        {
                            int playerNum = P_Get(otherSprite);

                            if (!FURY)
                            {
                                A_PlaySound(PISTOL_BODYHIT, otherSprite);

                                if (pProj->workslike & PROJECTILE_SPIT)
                                    P_HandleBeingSpitOn(g_player[playerNum].ps);
                            }
                        }
#endif

                        if (pProj->workslike & PROJECTILE_RPG_IMPACT)
                        {
                            actor[otherSprite].htowner  = pSprite->owner;
                            actor[otherSprite].htpicnum = pSprite->picnum;

                            if (pProj->workslike & PROJECTILE_RPG_IMPACT_DAMAGE)
                                actor[otherSprite].htextra += pProj->extra;

                            A_DoProjectileEffects(spriteNum, false);

                            if (!(pProj->workslike & PROJECTILE_FORCEIMPACT))
                            {
                                A_DeleteSprite(spriteNum);
                                return;
                            }
                        }

                        if (pProj->workslike & PROJECTILE_FORCEIMPACT)
                            return;
                        break;

                    case 32768:
                        otherSprite &= (MAXWALLS - 1);

                        if (pProj->workslike & PROJECTILE_BOUNCESOFFMIRRORS
                            && (wall[otherSprite].overpicnum == MIRROR || wall[otherSprite].picnum == MIRROR))
                        {
                            Proj_BounceOffWall(pSprite, otherSprite);
                            pSprite->owner = spriteNum;
#ifndef EDUKE32_STANDALONE
                            if (!FURY)
                                A_Spawn(spriteNum, TRANSPORTERSTAR);
#endif
                            return;
                        }
                        else
                        {
                            sprite[spriteNum].xy -= Proj_GetOffset(spriteNum);

                            A_DamageWall(spriteNum, otherSprite, pSprite->xyz, pSprite->picnum);

                            if (pProj->workslike & PROJECTILE_BOUNCESOFFWALLS && pSprite->yvel > 0)
                            {
                                if (wall[otherSprite].overpicnum != MIRROR && wall[otherSprite].picnum != MIRROR)
                                    pSprite->yvel--;

                                Proj_BounceOffWall(pSprite, otherSprite);

                                if (pProj->bsound >= 0)
                                    A_PlaySound(pProj->bsound, spriteNum);

                                if (pProj->workslike & PROJECTILE_LOSESVELOCITY)
                                    PROJ_DECAYVELOCITY(pSprite);

                                return;
                            }
                        }
                        break;

                    case 16384:
                        sprite[spriteNum].xy -= Proj_GetOffset(spriteNum);

                        if (Proj_MaybeDamageCF(spriteNum))
                        {
                            A_DeleteSprite(spriteNum);
                            return;
                        }

                        if (pProj->workslike & PROJECTILE_BOUNCESOFFWALLS && pSprite->yvel > 0)
                        {
                            A_DoProjectileBounce(spriteNum);
                            A_SetSprite(spriteNum, CLIPMASK1);

                            pSprite->yvel--;

                            if (pProj->bsound >= 0)
                                A_PlaySound(pProj->bsound, spriteNum);

                            if (pProj->workslike & PROJECTILE_LOSESVELOCITY)
                                PROJ_DECAYVELOCITY(pSprite);

                            return;
                        }
                        break;
                }

                A_DoProjectileEffects(spriteNum);
                A_DeleteSprite(spriteNum);
                return;
            }
            return;
        }
    }
}

#ifndef EDUKE32_STANDALONE
struct SpriteTracerData
{
    int32_t x, y, z;
    int32_t xVel, yVel, zVel;
    SpriteTracerData() : x(0), y(0), z(0), xVel(0), yVel(0), zVel(0) { }
};

std::map<int, SpriteTracerData> tracerData;
#endif

ACTOR_STATIC void G_MoveWeapons(void)
{
    int spriteNum = headspritestat[STAT_PROJECTILE];

    while (spriteNum >= 0)
    {
        int const  nextSprite = nextspritestat[spriteNum];
        auto const pSprite    = &sprite[spriteNum];

        if (pSprite->sectnum < 0)
            DELETE_SPRITE_AND_CONTINUE(spriteNum);

        /* Custom projectiles */
        if (A_CheckSpriteFlags(spriteNum, SFLAG_PROJECTILE))
        {
            Proj_MoveCustom(spriteNum);
            goto next_sprite;
        }

        // hard coded projectiles
        switch (tileGetMapping(pSprite->picnum))
        {
            case SHOTSPARK1__:
            {
                if (!G_TileHasActor(sprite[spriteNum].picnum))
                    goto next_sprite;
                int32_t   playerDist;
                int const playerNum = A_FindPlayer(pSprite, &playerDist);
                A_Execute(spriteNum, playerNum, playerDist);
                goto next_sprite;
            }

            case RADIUSEXPLOSION__:
            case KNEE__: DELETE_SPRITE_AND_CONTINUE(spriteNum);
        }
#ifndef EDUKE32_STANDALONE
        if (!FURY)
        switch (tileGetMapping(pSprite->picnum))
        {
            case FREEZEBLAST__:
                if (pSprite->yvel < 1 || pSprite->extra < 2 || (pSprite->xvel | pSprite->zvel) == 0)
                {
                    int const newSprite       = A_Spawn(spriteNum, TRANSPORTERSTAR);
                    sprite[newSprite].pal     = 1;
                    sprite[newSprite].xrepeat = 32;
                    sprite[newSprite].yrepeat = 32;
                    DELETE_SPRITE_AND_CONTINUE(spriteNum);
                }
                fallthrough__;
            case FIREBALL__:
                if (!WORLDTOUR && pSprite->picnum == FIREBALL)
                    break;
                fallthrough__;
            case SHRINKSPARK__:
            case RPG__:
            case FIRELASER__:
            case SPIT__:
            case COOLEXPLOSION1__:
            {
                int const projectileMoved = SpriteProjectile[spriteNum].workslike & PROJECTILE_MOVED;
                SpriteProjectile[spriteNum].workslike |= PROJECTILE_MOVED;

                if (pSprite->picnum == COOLEXPLOSION1)
                    if (!S_CheckSoundPlaying(WIERDSHOT_FLY))
                        A_PlaySound(WIERDSHOT_FLY, spriteNum);

                int spriteXvel = pSprite->xvel;
                int spriteZvel = pSprite->zvel;

                if (pSprite->picnum == RPG && sector[pSprite->sectnum].lotag == ST_2_UNDERWATER)
                {
                    spriteXvel >>= 1;
                    spriteZvel >>= 1;
                }

                if (!projectileMoved)
                    spriteXvel += sprite[pSprite->owner].xvel;

                A_GetZLimits(spriteNum);

                int const fireball = (WORLDTOUR && pSprite->picnum == FIREBALL && sprite[pSprite->owner].picnum != FIREBALL);

                if (pSprite->picnum == RPG && actor[spriteNum].htpicnum != BOSS2 && pSprite->xrepeat >= 10
                    && sector[pSprite->sectnum].lotag != ST_2_UNDERWATER
                    && g_scriptVersion >= 13)
                {
                    int const newSprite = A_Spawn(spriteNum, SMALLSMOKE);
                    sprite[newSprite].z += (1 << 8);
                }
                if (pSprite->picnum == FIREBALL)
                {
                    if (sector[pSprite->sectnum].lotag == ST_2_UNDERWATER) { DELETE_SPRITE_AND_CONTINUE(spriteNum); }
                    if (fireball)
                    {
                        if (actor[spriteNum].t_data[0] >= 1 && actor[spriteNum].t_data[0] < 6)
                        {
                            float t = 1.f - 0.2 * actor[spriteNum].t_data[0];
                            int j = A_Spawn(spriteNum, FIREBALL);
                            spritetype* sj = &sprite[j];
                            sj->xvel = pSprite->xvel;
                            sj->yvel = pSprite->yvel;
                            sj->zvel = pSprite->zvel;
                            if (actor[spriteNum].t_data[0] > 1)
                            {
                                SpriteTracerData t = tracerData[actor[spriteNum].t_data[1]];
                                sj->x = t.x;
                                sj->y = t.y;
                                sj->z = t.z;
                                sj->xvel = t.xVel;
                                sj->yvel = t.yVel;
                                sj->zvel = t.zVel;
                            }
                            sj->xrepeat = sj->yrepeat = t * pSprite->xrepeat;
                            sj->cstat = pSprite->cstat;
                            sj->extra = 0;
                            actor[spriteNum].t_data[1] = j;
                            SpriteTracerData tt;
                            tt.x = sj->x;
                            tt.y = sj->y;
                            tt.z = sj->z;
                            tt.xVel = sj->xvel;
                            tt.yVel = sj->yvel;
                            tt.zVel = sj->zvel;
                            tracerData[actor[spriteNum].t_data[1]] = tt;
                            changespritestat(j, 4);
                        }
                        actor[spriteNum].t_data[0]++;
                    }
                    if (pSprite->zvel < 15000)
                        pSprite->zvel += 200;
                }

                int moveSprite = A_MoveSprite(spriteNum,
                                              { (spriteXvel * (sintable[(pSprite->ang + 512) & 2047])) >> 14 >> (int)!projectileMoved,
                                                (spriteXvel * (sintable[pSprite->ang & 2047])) >> 14 >> (int)!projectileMoved, spriteZvel >> (int)!projectileMoved },
                                                (A_CheckSpriteFlags(spriteNum, SFLAG_NOCLIP) ? 0 : CLIPMASK1));

                if (pSprite->picnum == RPG && (unsigned) pSprite->yvel < MAXSPRITES)  // RPG_YVEL
                    if (FindDistance2D(pSprite->x - sprite[pSprite->yvel].x, pSprite->y - sprite[pSprite->yvel].y) < 256)
                        moveSprite = 49152 | pSprite->yvel;

                actor[spriteNum].movflag = moveSprite;

                if (pSprite->sectnum < 0)
                    DELETE_SPRITE_AND_CONTINUE(spriteNum);

                if ((moveSprite & 49152) != 49152 && pSprite->picnum != FREEZEBLAST)
                    G_WeaponHitCeilingOrFloor(spriteNum, pSprite, &moveSprite);

                if (pSprite->picnum == FIRELASER)
                {
                    for (int k = -3; k < 2; k++)
                    {
                        vec3_t const offset = { (k * sintable[(pSprite->ang + 512) & 2047]) >> 9,
                                                (k * sintable[pSprite->ang & 2047]) >> 9,
                                                (k * ksgn(pSprite->zvel)) * klabs(pSprite->zvel / 24) };

                        int const newSprite
                            = A_InsertSprite(pSprite->sectnum, pSprite->x + offset.x,
                                pSprite->y + offset.y,
                                pSprite->z + offset.z, FIRELASER, -40 + ksgn(k) * (klabs(k) << 2),
                                pSprite->xrepeat, pSprite->yrepeat, 0, 0, 0, pSprite->owner, STAT_MISC);

                        actor[newSprite].bpos = actor[spriteNum].bpos + offset;
                        sprite[newSprite].cstat = 128;
                        sprite[newSprite].pal   = pSprite->pal;
                    }
                }
                else if (pSprite->picnum == SPIT)
                    if (pSprite->zvel < ACTOR_MAXFALLINGZVEL)
                        pSprite->zvel += g_spriteGravity - 112;

                if (moveSprite != 0)
                {
                    if (pSprite->picnum == COOLEXPLOSION1)
                    {
                        if ((moveSprite & 49152) == 49152 && sprite[moveSprite & (MAXSPRITES - 1)].picnum != APLAYER)
                            goto COOLEXPLOSION;
                        pSprite->xvel = 0;
                        pSprite->zvel = 0;
                    }

                    switch (moveSprite & 49152)
                    {
                        case 49152:
                            moveSprite &= (MAXSPRITES - 1);

                            if (pSprite->picnum == FREEZEBLAST && sprite[moveSprite].pal == 1)
                                if (A_CheckEnemySprite(&sprite[moveSprite]) || sprite[moveSprite].picnum == APLAYER)
                                {
                                    int const newSprite       = A_Spawn(spriteNum, TRANSPORTERSTAR);
                                    sprite[newSprite].pal     = 1;
                                    sprite[newSprite].xrepeat = 32;
                                    sprite[newSprite].yrepeat = 32;

                                    DELETE_SPRITE_AND_CONTINUE(spriteNum);
                                }

                            if (!WORLDTOUR || pSprite->picnum != FIREBALL || fireball)
                                A_DamageObject(moveSprite, spriteNum);

                            if (sprite[moveSprite].picnum == APLAYER)
                            {
                                int const playerNum = P_Get(moveSprite);
                                A_PlaySound(PISTOL_BODYHIT, moveSprite);

                                if (pSprite->picnum == SPIT)
                                    P_HandleBeingSpitOn(g_player[playerNum].ps);
                            }
                            break;

                        case 32768:
                            moveSprite &= (MAXWALLS - 1);

                            if (pSprite->picnum != RPG && pSprite->picnum != FREEZEBLAST && pSprite->picnum != SPIT
                                && (!WORLDTOUR || pSprite->picnum != FIREBALL)
                                && (wall[moveSprite].overpicnum == MIRROR || wall[moveSprite].picnum == MIRROR))
                            {
                                Proj_BounceOffWall(pSprite, moveSprite);
                                pSprite->owner = spriteNum;
                                A_Spawn(spriteNum, TRANSPORTERSTAR);
                                goto next_sprite;
                            }
                            else
                            {
                                A_DamageWall(spriteNum, moveSprite, pSprite->xyz, pSprite->picnum);

                                if (pSprite->picnum == FREEZEBLAST)
                                {
                                    if (wall[moveSprite].overpicnum != MIRROR && wall[moveSprite].picnum != MIRROR)
                                    {
                                        pSprite->extra >>= 1;
                                        pSprite->yvel--;
                                    }

                                    Proj_BounceOffWall(pSprite, moveSprite);
                                    goto next_sprite;
                                }
                            }
                            break;

                        case 16384:
                            pSprite->xy -= Proj_GetOffset(spriteNum);

                            if (Proj_MaybeDamageCF(spriteNum))
                                DELETE_SPRITE_AND_CONTINUE(spriteNum);

                            if (pSprite->picnum == FREEZEBLAST)
                            {
                                A_DoProjectileBounce(spriteNum);
                                A_SetSprite(spriteNum, CLIPMASK1);

                                pSprite->extra >>= 1;
                                pSprite->yvel--;

                                if (pSprite->xrepeat > 8)
                                {
                                    pSprite->xrepeat -= 2;

                                    if (pSprite->yrepeat > 8)
                                        pSprite->yrepeat -= 2;
                                }

                                goto next_sprite;
                            }

                            if (pSprite->zvel >= 0 && fireball)
                            {
                                int lp = A_Spawn(spriteNum, LAVAPOOL);
                                sprite[lp].owner = sprite[spriteNum].owner;
                                sprite[lp].yvel = sprite[spriteNum].yvel;
                                actor[lp].htowner = sprite[spriteNum].owner;
                                DELETE_SPRITE_AND_CONTINUE(spriteNum);
                            }
                            break;
                        default: break;
                    }

                    switch (tileGetMapping(pSprite->picnum))
                    {
                        case SPIT__:
                        case COOLEXPLOSION1__:
                        case FREEZEBLAST__:
                        case FIRELASER__:
                            break;

                        case RPG__:
                        {
                            int const newSprite = A_Spawn(spriteNum, EXPLOSION2);
                            A_PlaySound(RPG_EXPLODE, newSprite);

                            if (pSprite->xrepeat < 10)
                            {
                                sprite[newSprite].xrepeat = 6;
                                sprite[newSprite].yrepeat = 6;
                            }

                            sprite[newSprite].xy -= Proj_GetOffset(newSprite);

                            if (pSprite->xrepeat >= 10 && (moveSprite & 49152) == 16384)
                            {
                                if (pSprite->zvel > 0)
                                {
                                    auto newSprite2 = A_Spawn(spriteNum, EXPLOSION2BOT);
                                    sprite[newSprite2].xy = sprite[newSprite].xy;
                                }
                                else
                                {
                                    sprite[newSprite].cstat |= 8;
                                    sprite[newSprite].z += (48 << 8);
                                }
                            }

                            if (pSprite->xrepeat >= 10)
                            {
                                int const x = pSprite->extra;
                                A_RadiusDamage(spriteNum, g_rpgRadius, x >> 2, x >> 1, x - (x >> 2), x);
                            }
                            else
                            {
                                int const x = pSprite->extra + (g_globalRandom & 3);
                                A_RadiusDamage(spriteNum, (g_rpgRadius >> 1), x >> 2, x >> 1, x - (x >> 2), x);
                            }
                            break;
                        }

                        case SHRINKSPARK__:
                            A_Spawn(spriteNum, SHRINKEREXPLOSION);
                            A_PlaySound(SHRINKER_HIT, spriteNum);
                            A_RadiusDamage(spriteNum, g_shrinkerRadius, 0, 0, 0, 0);
                            break;

                        case FIREBALL__:
                            if (WORLDTOUR)
                                break;
                            fallthrough__;
                        default:
                        {
                            int const newSprite       = A_Spawn(spriteNum, EXPLOSION2);
                            sprite[newSprite].xrepeat = sprite[newSprite].yrepeat = pSprite->xrepeat >> 1;
                            if ((moveSprite & 49152) == 16384)
                            {
                                if (pSprite->zvel < 0)
                                {
                                    sprite[newSprite].cstat |= 8;
                                    sprite[newSprite].z += (72 << 8);
                                }
                            }
                            break;
                        }
                    }

                    if (fireball)
                    {
                        int ex = A_Spawn(spriteNum, EXPLOSION2);
                        sprite[ex].xrepeat = sprite[ex].yrepeat = pSprite->xrepeat >> 1;
                    }

                    if (pSprite->picnum != COOLEXPLOSION1)
                        DELETE_SPRITE_AND_CONTINUE(spriteNum);
                }

                if (pSprite->picnum == COOLEXPLOSION1)
                {
                COOLEXPLOSION:
                    pSprite->shade++;
                    if (pSprite->shade >= 40)
                        DELETE_SPRITE_AND_CONTINUE(spriteNum);
                }
                else if (pSprite->picnum == RPG && sector[pSprite->sectnum].lotag == ST_2_UNDERWATER && pSprite->xrepeat >= 10 && rnd(140))
                    A_Spawn(spriteNum, WATERBUBBLE);

                goto next_sprite;
            }
        }
#endif
    next_sprite:
        spriteNum = nextSprite;
    }
}


static int P_Submerge(int const playerNum, DukePlayer_t * const pPlayer, int const sectNum, int const otherSect)
{
    if (pPlayer->on_ground && pPlayer->pos.z >= sector[sectNum].floorz
        && (TEST_SYNC_KEY(g_player[playerNum].input.bits, SK_CROUCH) || pPlayer->vel.z > 2048))
    //        if( onfloorz && sectlotag == 1 && ps->pos.z > (sector[sect].floorz-(6<<8)) )
    {
        if (screenpeek == playerNum)
        {
            FX_StopAllSounds();
            S_ClearSoundLocks();
        }

#ifndef EDUKE32_STANDALONE
        if (!FURY && sprite[pPlayer->i].extra > 0)
            A_PlaySound(DUKE_UNDERWATER, pPlayer->i);
#endif

        pPlayer->opos.z = pPlayer->pos.z = sector[otherSect].ceilingz;

        if (TEST_SYNC_KEY(g_player[playerNum].input.bits, SK_CROUCH))
            pPlayer->vel.z += 512;

        return 1;
    }

    return 0;
}

static int P_Emerge(int const playerNum, DukePlayer_t * const pPlayer, int const sectNum, int const otherSect)
{
    // r1449-:
    if (pPlayer->pos.z < (sector[sectNum].ceilingz+1080) && pPlayer->vel.z <= 0)
        // r1450+, breaks submergible slime in bobsp2:
//        if (onfloorz && sectlotag == 2 && ps->pos.z <= sector[sect].ceilingz /*&& ps->vel.z == 0*/)
    {
//        if( sprite[j].extra <= 0) break;
        if (screenpeek == playerNum)
        {
            FX_StopAllSounds();
            S_ClearSoundLocks();
        }

#ifndef EDUKE32_STANDALONE
        if (!FURY)
            A_PlaySound(DUKE_GASP, pPlayer->i);
#endif

        pPlayer->opos.z = pPlayer->pos.z = sector[otherSect].floorz;
        pPlayer->vel.z = 0;
//        ps->vel.z += 1024;

        pPlayer->jumping_toggle = 1;
        pPlayer->jumping_counter = 0;

        return 1;
    }

    return 0;
}

static void P_FinishWaterChange(int const playerNum, DukePlayer_t * const pPlayer, int const sectLotag, int const spriteOwner, int const newSector)
{
    pPlayer->bobpos.x = pPlayer->opos.x = pPlayer->pos.x;
    pPlayer->bobpos.y = pPlayer->opos.y = pPlayer->pos.y;

    if (spriteOwner < 0 || sprite[spriteOwner].owner != spriteOwner)
        pPlayer->transporter_hold = -2;

    pPlayer->cursectnum = newSector;
    changespritesect(playerNum, newSector);

    vec3_t vect = pPlayer->pos;
    vect.z += pPlayer->spritezoffset;
    setsprite(pPlayer->i, &vect);

    P_UpdateScreenPal(pPlayer);

    if ((krand()&255) < 32)
        A_Spawn(playerNum, WATERSPLASH2);

    if (sectLotag == ST_1_ABOVE_WATER)
    {
        for (bssize_t l = 0; l < 9; l++)
            sprite[A_Spawn(pPlayer->i, WATERBUBBLE)].z += krand() & 16383;
    }
}

// Check prevention of teleportation *when alive*. For example, commanders and
// octabrains would be transported by SE7 (both water and normal) only if dead.
static int A_CheckNonTeleporting(int const spriteNum)
{
    int const tileNum = sprite[spriteNum].picnum;
    return !!(A_CheckSpriteFlags(spriteNum, SFLAG_NOTELEPORT) || tileNum == SHARK || tileNum == COMMANDER || tileNum == OCTABRAIN
              || (tileNum >= GREENSLIME && tileNum <= GREENSLIME + 7));
}

ACTOR_STATIC void G_MoveTransports(void)
{
    int spriteNum = headspritestat[STAT_TRANSPORT];

    while (spriteNum >= 0)
    {
        int const nextSprite = nextspritestat[spriteNum];

        if (OW(spriteNum) == spriteNum)
        {
            spriteNum = nextSprite;
            continue;
        }

        int const sectNum    = SECT(spriteNum);
        int const sectLotag  = sector[sectNum].lotag;
        int const onFloor    = T5(spriteNum);  // ONFLOORZ

        if (T1(spriteNum) > 0)
            T1(spriteNum)--;

        int sectSprite = headspritesect[sectNum];
        while (sectSprite >= 0)
        {
            int const nextSectSprite = nextspritesect[sectSprite];

            switch (sprite[sectSprite].statnum)
            {
                case STAT_PLAYER:
                    if (sprite[sectSprite].owner != -1)
                    {
                        int const  playerNum  = P_Get(sectSprite);
                        auto &     thisPlayer = g_player[playerNum];
                        auto const pPlayer    = thisPlayer.ps;

                        pPlayer->on_warping_sector = 1;

                        if (pPlayer->transporter_hold == 0 && pPlayer->jumping_counter == 0)
                        {
                            if (pPlayer->on_ground && sectLotag == 0 && onFloor && pPlayer->jetpack_on == 0)
                            {
#ifndef EDUKE32_STANDALONE
                                if (!FURY && sprite[spriteNum].pal == 0)
                                {
                                    A_Spawn(spriteNum, TRANSPORTERBEAM);
                                    A_PlaySound(TELEPORTER, spriteNum);
                                }
#endif
                                for (int TRAVERSE_CONNECT(otherPlayer))
                                {
                                    if (g_player[otherPlayer].ps->cursectnum == sprite[OW(spriteNum)].sectnum)
                                    {
                                        g_player[otherPlayer].ps->frag_ps         = playerNum;
                                        sprite[g_player[otherPlayer].ps->i].extra = 0;
                                    }
                                }

                                thisPlayer.smoothcamera = true;
                                pPlayer->q16ang = fix16_from_int(sprite[OW(spriteNum)].ang);

                                if (sprite[OW(spriteNum)].owner != OW(spriteNum))
                                {
                                    T1(spriteNum)                  = 13;
                                    actor[OW(spriteNum)].t_data[0] = 13;
                                    pPlayer->transporter_hold      = 13;
                                }

                                pPlayer->pos    = sprite[OW(spriteNum)].xyz;
                                pPlayer->pos.z -= pPlayer->spritezoffset;
                                pPlayer->opos   = pPlayer->pos;
                                pPlayer->bobpos = pPlayer->pos.xy;

                                changespritesect(sectSprite, sprite[OW(spriteNum)].sectnum);
                                pPlayer->cursectnum = sprite[sectSprite].sectnum;

#ifndef EDUKE32_STANDALONE
                                if (!FURY && sprite[spriteNum].pal == 0)
                                {
                                    int const newSprite = A_Spawn(OW(spriteNum), TRANSPORTERBEAM);
                                    A_PlaySound(TELEPORTER, newSprite);
                                }
#endif
                                break;
                            }

                            if (onFloor == 0 && klabs(SZ(spriteNum) - pPlayer->pos.z) < 6144)
                                if (!pPlayer->jetpack_on || TEST_SYNC_KEY(thisPlayer.input.bits, SK_JUMP)
                                    || TEST_SYNC_KEY(thisPlayer.input.bits, SK_CROUCH))
                                {
                                    auto posdiff = pPlayer->opos - pPlayer->pos;

                                    pPlayer->pos.x += sprite[OW(spriteNum)].x - SX(spriteNum);
                                    pPlayer->pos.y += sprite[OW(spriteNum)].y - SY(spriteNum);
                                    pPlayer->pos.z = (pPlayer->jetpack_on && (TEST_SYNC_KEY(thisPlayer.input.bits, SK_JUMP)
                                                                              || pPlayer->jetpack_on < 11))
                                                     ? sprite[OW(spriteNum)].z - 6144
                                                     : sprite[OW(spriteNum)].z + 6144;

                                    actor[pPlayer->i].bpos = pPlayer->pos;
                                    pPlayer->opos          = pPlayer->pos + posdiff;
                                    pPlayer->bobpos        = pPlayer->pos.xy;

                                    changespritesect(sectSprite, sprite[OW(spriteNum)].sectnum);
                                    pPlayer->cursectnum = sprite[OW(spriteNum)].sectnum;

                                    break;
                                }

                            int doWater = 0;

                            if (onFloor)
                            {
                                if (sectLotag == ST_1_ABOVE_WATER)
                                    doWater = P_Submerge(playerNum, pPlayer, sectNum, sprite[OW(spriteNum)].sectnum);
                                else if (sectLotag == ST_2_UNDERWATER)
                                    doWater = P_Emerge(playerNum, pPlayer, sectNum, sprite[OW(spriteNum)].sectnum);

                                if (doWater == 1)
                                {
                                    pPlayer->pos.x += sprite[OW(spriteNum)].x - SX(spriteNum);
                                    pPlayer->pos.y += sprite[OW(spriteNum)].y - SY(spriteNum);

                                    P_FinishWaterChange(sectSprite, pPlayer, sectLotag, OW(spriteNum), sprite[OW(spriteNum)].sectnum);
                                }
                            }
                        }
                        else if (!(sectLotag == ST_1_ABOVE_WATER && pPlayer->on_ground == 1))
                            break;
                    }
                    break;


                ////////// Non-player teleportation //////////

                case STAT_PROJECTILE:
                // SE7_PROJECTILE, PROJECTILE_CHSECT.
                // comment out to make RPGs pass through water: (r1450 breaks this)
                //                if (sectlotag != 0) goto JBOLT;
                case STAT_ACTOR:
                    if (sprite[sectSprite].extra > 0 && A_CheckNonTeleporting(sectSprite))
                        goto JBOLT;
                    fallthrough__;
                case STAT_MISC:
                case STAT_FALLER:
                case STAT_DUMMYPLAYER:
                {
                    if (((int32_t) totalclock & UINT8_MAX) != actor[sectSprite].lasttransport)
                    {
                        int const zvel    = sprite[sectSprite].zvel;
                        int const absZvel = klabs(zvel);
                        int       doWarp  = 0;

                        if (absZvel != 0)
                        {
                            if (sectLotag == ST_2_UNDERWATER && sprite[sectSprite].z < (sector[sectNum].ceilingz + absZvel) && zvel < 0)
                                doWarp = 1;
                            if (sectLotag == ST_1_ABOVE_WATER && sprite[sectSprite].z > (sector[sectNum].floorz - absZvel) && zvel > 0)
                                doWarp = 1;
                        }

                        if (sectLotag == 0 && (onFloor || klabs(sprite[sectSprite].z - SZ(spriteNum)) < 4096))
                        {
                            if (sprite[OW(spriteNum)].owner != OW(spriteNum) && onFloor && T1(spriteNum) > 0
                                && sprite[sectSprite].statnum != STAT_MISC)
                            {
                                T1(spriteNum)++;
                                goto next_sprite;
                            }
                            doWarp = 1;
                        }

                        if (doWarp)
                        {
                            if (A_CheckSpriteFlags(sectSprite, SFLAG_DECAL))
                                goto JBOLT;

#ifndef EDUKE32_STANDALONE
                            if (!FURY)
                            switch (tileGetMapping(sprite[sectSprite].picnum))
                            {
                                case TRANSPORTERSTAR__:
                                case TRANSPORTERBEAM__:
                                case TRIPBOMB__:
                                case BULLETHOLE__:
                                case WATERSPLASH2__:
                                case BURNING__:
                                case BURNING2__:
                                case FIRE__:
                                case FIRE2__:
                                case TOILETWATER__:
                                case LASERLINE__: goto JBOLT;
                            }
#endif
                            switch (tileGetMapping(sprite[sectSprite].picnum))
                            {
                                case PLAYERONWATER__:
                                    if (sectLotag == ST_2_UNDERWATER)
                                    {
                                        sprite[sectSprite].cstat &= 32768;
                                        break;
                                    }
                                    fallthrough__;
                                default:
                                    if (sprite[sectSprite].statnum == STAT_MISC && !(sectLotag == ST_1_ABOVE_WATER || sectLotag == ST_2_UNDERWATER))
                                        break;
                                    fallthrough__;
                                case WATERBUBBLE__:
                                    //                            if( rnd(192) && sprite[j].picnum == WATERBUBBLE)
                                    //                                break;

                                    if (sectLotag > 0)
                                    {
                                        // Water SE7 teleportation.
                                        int const osect = sprite[OW(spriteNum)].sectnum;

                                        Bassert(sectLotag == ST_1_ABOVE_WATER || sectLotag == ST_2_UNDERWATER);
#ifndef EDUKE32_STANDALONE
                                        if (!FURY)
                                        {
                                            int const newSprite = A_Spawn(sectSprite, WATERSPLASH2);

                                            if (sectLotag == ST_1_ABOVE_WATER && sprite[sectSprite].statnum == STAT_PROJECTILE)
                                            {
                                                sprite[newSprite].xvel = sprite[sectSprite].xvel >> 1;
                                                sprite[newSprite].ang  = sprite[sectSprite].ang;
                                                A_SetSprite(newSprite, CLIPMASK0);
                                            }
                                        }
#endif
                                        actor[sectSprite].lasttransport = ((int32_t) totalclock & UINT8_MAX);

                                        sprite[sectSprite].x += sprite[OW(spriteNum)].x - SX(spriteNum);
                                        sprite[sectSprite].y += sprite[OW(spriteNum)].y - SY(spriteNum);
                                        sprite[sectSprite].z = (sectLotag == ST_1_ABOVE_WATER) ? sector[osect].ceilingz : sector[osect].floorz;

                                        actor[sectSprite].bpos = sprite[sectSprite].xyz;

                                        changespritesect(sectSprite, sprite[OW(spriteNum)].sectnum);
                                    }
                                    else if (Bassert(sectLotag == 0), 1)
                                    {
                                        // Non-water SE7 teleportation.

                                        if (onFloor)
                                        {
                                            if (sprite[sectSprite].statnum == STAT_PROJECTILE
                                                || (G_GetPlayerInSector(sectNum) == -1
                                                    && G_GetPlayerInSector(sprite[OW(spriteNum)].sectnum) == -1))
                                            {
                                                sprite[sectSprite].x += (sprite[OW(spriteNum)].x - SX(spriteNum));
                                                sprite[sectSprite].y += (sprite[OW(spriteNum)].y - SY(spriteNum));
                                                sprite[sectSprite].z -= SZ(spriteNum) - sector[sprite[OW(spriteNum)].sectnum].floorz;

                                                sprite[sectSprite].ang = sprite[OW(spriteNum)].ang;
                                                actor[sectSprite].bpos = sprite[sectSprite].xyz;
#ifndef EDUKE32_STANDALONE
                                                if (!FURY && sprite[spriteNum].pal == 0)
                                                {
                                                    int newSprite = A_Spawn(spriteNum, TRANSPORTERBEAM);
                                                    A_PlaySound(TELEPORTER, newSprite);

                                                    newSprite = A_Spawn(OW(spriteNum), TRANSPORTERBEAM);
                                                    A_PlaySound(TELEPORTER, newSprite);
                                                }
#endif
                                                if (sprite[OW(spriteNum)].owner != OW(spriteNum))
                                                {
                                                    T1(spriteNum)                  = 13;
                                                    actor[OW(spriteNum)].t_data[0] = 13;
                                                }

                                                changespritesect(sectSprite, sprite[OW(spriteNum)].sectnum);
                                            }
                                        }
                                        else
                                        {
                                            sprite[sectSprite].x += (sprite[OW(spriteNum)].x - SX(spriteNum));
                                            sprite[sectSprite].y += (sprite[OW(spriteNum)].y - SY(spriteNum));
                                            sprite[sectSprite].z = sprite[OW(spriteNum)].z + 4096;

                                            actor[sectSprite].bpos = sprite[sectSprite].xyz;

                                            changespritesect(sectSprite, sprite[OW(spriteNum)].sectnum);
                                        }
                                    }

                                    break;
                            }  // switch (DYNAMICTILEMAP(sprite[j].picnum))
                        }      // if (doWarp)
                    }          // if (totalclock > actor[j].lasttransport)

                    break;
                }  // five cases

            }  // switch (sprite[j].statnum)
        JBOLT:
            sectSprite = nextSectSprite;
        }
    next_sprite:
        spriteNum = nextSprite;
    }
}

static int A_FindLocator(int const tag, int const sectNum)
{
    for (bssize_t SPRITES_OF(STAT_LOCATOR, spriteNum))
    {
        if ((sectNum == -1 || sectNum == SECT(spriteNum)) && tag == SLT(spriteNum))
            return spriteNum;
    }

    return -1;
}

static int A_FindLocatorWithHiLoTags(int const hitag, int const tag, int const sectNum)
{
    for (bssize_t SPRITES_OF(STAT_LOCATOR, spriteNum))
    {
        if ((sectNum == -1 || sectNum == SECT(spriteNum)) && tag == SLT(spriteNum) && hitag == SHT(spriteNum))
            return spriteNum;
    }

    return -1;
}

ACTOR_STATIC void G_MoveActors(void)
{
    int spriteNum = headspritestat[STAT_ACTOR];

    while (spriteNum >= 0)
    {
        int const  nextSprite = nextspritestat[spriteNum];
        auto const pSprite    = &sprite[spriteNum];
        int const  sectNum    = pSprite->sectnum;
        auto const pData      = actor[spriteNum].t_data;

        int switchPic;

        if (pSprite->xrepeat == 0 || sectNum < 0 || sectNum >= MAXSECTORS)
            DELETE_SPRITE_AND_CONTINUE(spriteNum);

        switchPic = pSprite->picnum;

#ifndef EDUKE32_STANDALONE
        if (!FURY && pSprite->picnum > GREENSLIME && pSprite->picnum <= GREENSLIME+7)
            switchPic = GREENSLIME;
#endif

        switch (tileGetMapping(switchPic))
        {
        case OOZ__:
        case OOZ2__:
        {
            A_GetZLimits(spriteNum);

            int const yrepeat = clamp((actor[spriteNum].floorz - actor[spriteNum].ceilingz) >> 9, 8, 255);
            int const xrepeat = clamp(25 - (yrepeat >> 1), 8, 48);

            pSprite->yrepeat = yrepeat;
            pSprite->xrepeat = xrepeat;
            pSprite->z       = actor[spriteNum].floorz;

            goto next_sprite;
        }
        case CAMERA1__:
            if (pData[0] == 0)
            {
                pData[1]+=8;
                if (g_damageCameras)
                {
                    if (A_IncurDamage(spriteNum) >= 0)
                    {
                        pData[0]       = 1;  // static
                        pSprite->cstat = 32768;

#ifndef EDUKE32_STANDALONE
                        if (!FURY)
                        {
                            for (bssize_t x = 0; x < 5; x++)
                                RANDOMSCRAP(pSprite, spriteNum);
                        }
#endif
                        goto next_sprite;
                    }
                }

                if (pSprite->hitag > 0)
                {
                    if (pData[1] < pSprite->hitag)             pSprite->ang += 8;
                    else if (pData[1] < pSprite->hitag * 3)    pSprite->ang -= 8;
                    else if (pData[1] < (pSprite->hitag << 2)) pSprite->ang += 8;
                    else
                    {
                        pData[1] = 0;
                        pSprite->ang += 8;
                    }
                }
            }
            goto next_sprite;
        }
#ifndef EDUKE32_STANDALONE
        switch (tileGetMapping(switchPic))
        {
        case FLAMETHROWERFLAME__:
        {
            if (!WORLDTOUR)
                goto next_sprite;

            if (G_TileHasActor(sprite[spriteNum].picnum))
            {
                int32_t playerDist;
                int const playerNum = A_FindPlayer(pSprite, &playerDist);
                A_Execute(spriteNum, playerNum, playerDist);
            }

            actor[spriteNum].t_data[0]++;
            if (sector[pSprite->sectnum].lotag == ST_2_UNDERWATER)
            {
                int const newSprite = A_Spawn(spriteNum, EXPLOSION2);
                sprite[newSprite].shade = 127;
                DELETE_SPRITE_AND_CONTINUE(spriteNum);
            }
            int spriteXvel = pSprite->xvel;
            int spriteZvel = pSprite->zvel;

            A_GetZLimits(spriteNum);

            if (pSprite->xrepeat < 80)
            {
                pSprite->xrepeat += actor[spriteNum].t_data[0] / 6;
                pSprite->yrepeat += actor[spriteNum].t_data[0] / 6;
            }
            pSprite->clipdist += actor[spriteNum].t_data[0] / 6;
            if (actor[spriteNum].t_data[0] < 2)
                actor[spriteNum].t_data[3] = krand() % 10;
            if (actor[spriteNum].t_data[0] > 30)
            {
                int const newSprite = A_Spawn(spriteNum, EXPLOSION2);
                sprite[newSprite].shade = 127;
                DELETE_SPRITE_AND_CONTINUE(spriteNum);
            }

            int moveSprite = A_MoveSprite(spriteNum, { (spriteXvel * (sintable[(pSprite->ang + 512) & 2047])) >> 14,
                                                       (spriteXvel * (sintable[pSprite->ang & 2047])) >> 14, spriteZvel }, CLIPMASK1);

            actor[spriteNum].movflag = moveSprite;

            if (pSprite->sectnum < 0)
                DELETE_SPRITE_AND_CONTINUE(spriteNum);

            if ((moveSprite & 49152) != 49152 && pSprite->picnum != FREEZEBLAST)
                G_WeaponHitCeilingOrFloor(spriteNum, pSprite, &moveSprite);

            if (moveSprite != 0)
            {
                switch (moveSprite & 49152)
                {
                case 49152:
                    moveSprite &= (MAXSPRITES - 1);

                    A_DamageObject(moveSprite, spriteNum);

                    if (sprite[moveSprite].picnum == APLAYER)
                    {
                        A_PlaySound(PISTOL_BODYHIT, moveSprite);
                    }
                    break;

                case 32768:
                    moveSprite &= (MAXWALLS - 1);

                    pSprite->xy -= Proj_GetOffset(spriteNum);
                    A_DamageWall(spriteNum, moveSprite, pSprite->xyz, pSprite->picnum);

                    break;

                case 16384:
                    if (pSprite->zvel < 0)
                        Sect_DamageCeiling(spriteNum, pSprite->sectnum);
                    else if (pSprite->zvel > 0)
                        Sect_DamageFloor(spriteNum, pSprite->sectnum);
                    break;
                default: break;
                }
            }

            if (pSprite->xrepeat >= 10)
            {
                int const x = pSprite->extra;
                A_RadiusDamage(spriteNum, g_rpgRadius, x >> 2, x >> 1, x - (x >> 2), x);
            }
            else
            {
                int const x = pSprite->extra + (g_globalRandom & 3);
                A_RadiusDamage(spriteNum, (g_rpgRadius >> 1), x >> 2, x >> 1, x - (x >> 2), x);
            }

            goto next_sprite;
        }
        case DUCK__:
        case TARGET__:
            if (pSprite->cstat&32)
            {
                pData[0]++;
                if (pData[0] > 60)
                {
                    pData[0] = 0;
                    pSprite->cstat = 128+257+16;
                    pSprite->extra = 1;
                }
            }
            else
            {
                if (A_IncurDamage(spriteNum) >= 0)
                {
                    int doEffects = 1;

                    pSprite->cstat = 32+128;

                    for (bssize_t SPRITES_OF(STAT_ACTOR, actorNum))
                    {
                        if ((sprite[actorNum].lotag == pSprite->lotag && sprite[actorNum].picnum == pSprite->picnum)
                            && ((sprite[actorNum].hitag != 0) ^ ((sprite[actorNum].cstat & 32) != 0)))
                        {
                            doEffects = 0;
                            break;
                        }
                    }

                    if (doEffects == 1)
                    {
                        G_OperateActivators(pSprite->lotag, -1);
                        G_OperateForceFields(spriteNum, pSprite->lotag);
                        G_OperateMasterSwitches(pSprite->lotag);
                    }
                }
            }
            goto next_sprite;

        case RESPAWNMARKERRED__:
        case RESPAWNMARKERYELLOW__:
        case RESPAWNMARKERGREEN__:
            if (++T1(spriteNum) > g_itemRespawnTime)
                DELETE_SPRITE_AND_CONTINUE(spriteNum);

            if (T1(spriteNum) >= (g_itemRespawnTime>>1) && T1(spriteNum) < ((g_itemRespawnTime>>1)+(g_itemRespawnTime>>2)))
                PN(spriteNum) = RESPAWNMARKERYELLOW;
            else if (T1(spriteNum) > ((g_itemRespawnTime>>1)+(g_itemRespawnTime>>2)))
                PN(spriteNum) = RESPAWNMARKERGREEN;

            A_Fall(spriteNum);
            break;

        case HELECOPT__:
        case DUKECAR__:
            pSprite->z += pSprite->zvel;
            pData[0]++;

            if (pData[0] == 4)
                A_PlaySound(WAR_AMBIENCE2,spriteNum);

            if (pData[0] > (GAMETICSPERSEC*8))
            {
                g_earthquakeTime = 16;
                S_PlaySound(RPG_EXPLODE);

                for (bssize_t j  = 0; j < 32; j++)
                    RANDOMSCRAP(pSprite, spriteNum);

                DELETE_SPRITE_AND_CONTINUE(spriteNum);
            }
            else if ((pData[0]&3) == 0)
                A_Spawn(spriteNum,EXPLOSION2);

            A_SetSprite(spriteNum,CLIPMASK0);
            break;

        case RAT__:
            A_Fall(spriteNum);
            if (A_SetSprite(spriteNum, CLIPMASK0))
            {
                if ((krand()&255) < 3) A_PlaySound(RATTY,spriteNum);
                pSprite->ang += (krand()&31)-15+(sintable[(pData[0]<<8)&2047]>>11);
            }
            else
            {
                T1(spriteNum)++;
                if (T1(spriteNum) > 1)
                {
                    DELETE_SPRITE_AND_CONTINUE(spriteNum);
                }
                else pSprite->ang = (krand()&2047);
            }
            if (pSprite->xvel < 128)
                pSprite->xvel+=2;
            pSprite->ang += (krand()&3)-6;
            break;

        case QUEBALL__:
        case STRIPEBALL__:
            if (pSprite->xvel)
            {
                for (bssize_t SPRITES_OF(STAT_DEFAULT, hitObject))
                    if (sprite[hitObject].picnum == POCKET && ldist(&sprite[hitObject],pSprite) < 52)
                        DELETE_SPRITE_AND_CONTINUE(spriteNum);

                int hitObject = clipmove(&pSprite->xyz, &pSprite->sectnum,
                                         (((pSprite->xvel * (sintable[(pSprite->ang + 512) & 2047])) >> 14) * TICSPERFRAME) << 11,
                                         (((pSprite->xvel * (sintable[pSprite->ang & 2047])) >> 14) * TICSPERFRAME) << 11, 24L, ZOFFSET6,
                                         ZOFFSET6, CLIPMASK1);

                if (hitObject & 49152)
                {
                    if ((hitObject & 49152) == 32768)
                    {
                        hitObject &= (MAXWALLS - 1);
                        Proj_BounceOffWall(pSprite, hitObject);
                    }
                    else if ((hitObject & 49152) == 49152)
                    {
                        hitObject &= (MAXSPRITES - 1);
                        A_DamageObject(spriteNum, hitObject);
                    }
                }

                if (--pSprite->xvel < 0)
                    pSprite->xvel = 0;

                if (pSprite->picnum == STRIPEBALL)
                {
                    pSprite->cstat = 257;
                    pSprite->cstat |= (4 & pSprite->xvel) | (8 & pSprite->xvel);
                }
            }
            else
            {
                int32_t    playerDist;
                int const  playerNum = A_FindPlayer(pSprite, &playerDist);
                auto const pPlayer   = g_player[playerNum].ps;

                // I'm 50/50 on this being either a typo or a stupid hack
                if (playerDist < 1596)
                {
                    int const angDiff = G_GetAngleDelta(fix16_to_int(pPlayer->q16ang),getangle(pSprite->x-pPlayer->pos.x,pSprite->y-pPlayer->pos.y));

                    if (angDiff > -64 && angDiff < 64 && TEST_SYNC_KEY(g_player[playerNum].input.bits, SK_OPEN)
                        && pPlayer->toggle_key_flag == 1)
                    {
                        int ballSprite;

                        for (SPRITES_OF(STAT_ACTOR, ballSprite))
                        {
                            if (sprite[ballSprite].picnum == QUEBALL || sprite[ballSprite].picnum == STRIPEBALL)
                            {
                                int const angDiff2 = G_GetAngleDelta(
                                    fix16_to_int(pPlayer->q16ang), getangle(sprite[ballSprite].x - pPlayer->pos.x, sprite[ballSprite].y - pPlayer->pos.y));

                                if (angDiff2 > -64 && angDiff2 < 64)
                                {
                                    int32_t ballDist;
                                    A_FindPlayer(&sprite[ballSprite], &ballDist);

                                    if (playerDist > ballDist)
                                        break;
                                }
                            }
                        }

                        if (ballSprite == -1)
                        {
                            pSprite->xvel = (pSprite->pal == 12) ? 164 : 140;
                            pSprite->ang  = fix16_to_int(pPlayer->q16ang);

                            pPlayer->toggle_key_flag = 2;
                        }
                    }
                }

                if (playerDist < 512 && pSprite->sectnum == pPlayer->cursectnum)
                {
                    pSprite->ang = getangle(pSprite->x-pPlayer->pos.x,pSprite->y-pPlayer->pos.y);
                    pSprite->xvel = 48;
                }
            }

            break;

        case FORCESPHERE__:
            if (pSprite->yvel == 0)
            {
                pSprite->yvel = 1;

                for (bssize_t l = 512; l < (2048 - 512); l += 128)
                {
                    for (bssize_t j = 0; j < 2048; j += 128)
                    {
                        int const newSprite        = A_Spawn(spriteNum, FORCESPHERE);
                        sprite[newSprite].cstat    = 257 + 128;
                        sprite[newSprite].clipdist = 64;
                        sprite[newSprite].ang      = j;
                        sprite[newSprite].zvel     = sintable[l & 2047] >> 5;
                        sprite[newSprite].xvel     = sintable[(l + 512) & 2047] >> 9;
                        sprite[newSprite].owner    = spriteNum;
                    }
                }
            }

            if (pData[3] > 0)
            {
                if (pSprite->zvel < ACTOR_MAXFALLINGZVEL)
                    pSprite->zvel += 192;

                pSprite->z += pSprite->zvel;

                if (pSprite->z > sector[sectNum].floorz)
                    pSprite->z = sector[sectNum].floorz;

                if (--pData[3] == 0)
                    DELETE_SPRITE_AND_CONTINUE(spriteNum);
            }
            else if (pData[2] > 10)
            {
                for (bssize_t SPRITES_OF(STAT_MISC, miscSprite))
                {
                    if (sprite[miscSprite].owner == spriteNum && sprite[miscSprite].picnum == FORCESPHERE)
                        actor[miscSprite].t_data[1] = 1 + (krand() & 63);
                }

                pData[3] = 64;
            }

            goto next_sprite;

        case RECON__:
        {
            int playerNum;
            DukePlayer_t *pPlayer;

            A_GetZLimits(spriteNum);

            pSprite->shade += (sector[pSprite->sectnum].ceilingstat & 1) ? (sector[pSprite->sectnum].ceilingshade - pSprite->shade) >> 1
                                                                         : (sector[pSprite->sectnum].floorshade - pSprite->shade) >> 1;

            if (pSprite->z < sector[sectNum].ceilingz + ZOFFSET5)
                pSprite->z = sector[sectNum].ceilingz + ZOFFSET5;

            if (!g_netServer && ud.multimode < 2)
            {
                if (g_noEnemies == 1)
                {
                    pSprite->cstat = 32768;
                    goto next_sprite;
                }
                else if (g_noEnemies == 2) pSprite->cstat = 257;
            }
            if (A_IncurDamage(spriteNum) >= 0)
            {
                if (pSprite->extra < 0 && pData[0] != -1)
                {
                    pData[0] = -1;
                    pSprite->extra = 0;
                }

                A_PlaySound(RECO_PAIN,spriteNum);
                RANDOMSCRAP(pSprite, spriteNum);
            }

            if (pData[0] == -1)
            {
                pSprite->z += 1024;
                pData[2]++;

                if ((pData[2]&3) == 0)
                    A_Spawn(spriteNum,EXPLOSION2);

                A_GetZLimits(spriteNum);
                pSprite->ang += 96;
                pSprite->xvel = 128;

                if (!A_SetSprite(spriteNum, CLIPMASK0) || pSprite->z > actor[spriteNum].floorz)
                {
                    for (bssize_t l = 0; l < 16; l++)
                        RANDOMSCRAP(pSprite, spriteNum);

                    int const newSprite = A_Spawn(spriteNum, EXPLOSION2);
                    A_PlaySound(LASERTRIP_EXPLODE, newSprite);
                    A_Spawn(spriteNum, PIGCOP);
                    P_AddKills(g_player[myconnectindex].ps, 1);
                    DELETE_SPRITE_AND_CONTINUE(spriteNum);
                }

                goto next_sprite;
            }
            else
            {
                if (pSprite->z > actor[spriteNum].floorz-(48<<8))
                    pSprite->z = actor[spriteNum].floorz-(48<<8);
            }

            int32_t playerDist;
            playerNum = A_FindPlayer(pSprite, &playerDist);
            pPlayer   = g_player[playerNum].ps;

            int const spriteOwner = pSprite->owner;

            // 3 = findplayerz, 4 = shoot

            if (pData[0] >= 4)
            {
                if ((++pData[2] & 15) == 0)
                {
                    int const saveAng = pSprite->ang;
                    pSprite->ang      = actor[spriteNum].tempang;
                    A_PlaySound(RECO_ATTACK, spriteNum);
                    A_Shoot(spriteNum, FIRELASER);
                    pSprite->ang      = saveAng;
                }
                if (pData[2] > (GAMETICSPERSEC * 3)
                    || !cansee(pSprite->x, pSprite->y, pSprite->z - ZOFFSET2, pSprite->sectnum, pPlayer->pos.x, pPlayer->pos.y,
                               pPlayer->pos.z, pPlayer->cursectnum))
                {
                    pData[0] = 0;
                    pData[2] = 0;
                }
                else actor[spriteNum].tempang += G_GetAngleDelta(actor[spriteNum].tempang,
                                                                 getangle(pPlayer->pos.x - pSprite->x,
                                                                          pPlayer->pos.y - pSprite->y)) / 3;
            }
            else if (pData[0] == 2 || pData[0] == 3)
            {
                pData[3]      = 0;
                pSprite->xvel = (pSprite->xvel > 0) ? pSprite->xvel - 16 : 0;

                if (pData[0] == 2)
                {
                    int const zDiff = pPlayer->pos.z - pSprite->z;

                    if (klabs(zDiff) < (48 << 8))
                        pData[0] = 3;
                    else
                        pSprite->z += ksgn(pPlayer->pos.z - pSprite->z) << 10;
                }
                else
                {
                    pData[2]++;
                    if (pData[2] > (GAMETICSPERSEC*3) ||
                        !cansee(pSprite->x,pSprite->y,pSprite->z-ZOFFSET2,pSprite->sectnum, pPlayer->pos.x,pPlayer->pos.y,pPlayer->pos.z,pPlayer->cursectnum))
                    {
                        pData[0] = 1;
                        pData[2] = 0;
                    }
                    else if ((pData[2]&15) == 0)
                    {
                        A_PlaySound(RECO_ATTACK,spriteNum);
                        A_Shoot(spriteNum,FIRELASER);
                    }
                }
                pSprite->ang += G_GetAngleDelta(pSprite->ang, getangle(pPlayer->pos.x - pSprite->x, pPlayer->pos.y - pSprite->y)) >> 2;
            }

            if (pData[0] != 2 && pData[0] != 3)
            {
                int newAngle;
                int locatorDist = ldist(&sprite[spriteOwner], pSprite);
                if (locatorDist <= 1524)
                {
                    newAngle = pSprite->ang;
                    pSprite->xvel >>= 1;
                }
                else newAngle = getangle(sprite[spriteOwner].x - pSprite->x, sprite[spriteOwner].y - pSprite->y);

                if (pData[0] == 1 || pData[0] == 4) // Found a locator and going with it
                {
                    locatorDist = dist(&sprite[spriteOwner], pSprite);

                    if (locatorDist <= 1524)
                    {
                        pData[0] = (pData[0] == 1) ? 0 : 5;
                    }
                    else
                    {
                        // Control speed here
                        if (pSprite->xvel < 256) pSprite->xvel += 32;
                    }

                    if (pData[0] < 2) pData[2]++;

                    if (playerDist < 6144 && pData[0] < 2 && pData[2] > (GAMETICSPERSEC*4))
                    {
                        pData[0] = 2+(krand()&2);
                        pData[2] = 0;
                        actor[spriteNum].tempang = pSprite->ang;
                    }
                }

                int locatorSprite = pSprite->owner;

                if (pData[0] == 0 || pData[0] == 5)
                {
                    pData[0]       = (pData[0] == 0) ? 1 : 4;
                    pSprite->owner = A_FindLocator(pSprite->hitag, -1);
                    locatorSprite  = pSprite->owner;

                    if (locatorSprite == -1)
                    {
                        locatorSprite  = actor[spriteNum].t_data[5];
                        pSprite->hitag = locatorSprite;
                        pSprite->owner = A_FindLocator(locatorSprite, -1);
                        locatorSprite  = pSprite->owner;

                        if (locatorSprite == -1)
                            DELETE_SPRITE_AND_CONTINUE(spriteNum);
                    }
                    else pSprite->hitag++;
                }

                // RECON_T4
                pData[3] = G_GetAngleDelta(pSprite->ang,newAngle);
                pSprite->ang += pData[3]>>3;

                if (pSprite->z < sprite[locatorSprite].z - 512)
                    pSprite->z += 512;
                else if (pSprite->z > sprite[locatorSprite].z + 512)
                    pSprite->z -= 512;
                else
                    pSprite->z = sprite[locatorSprite].z;
            }

            if (!A_CheckSoundPlaying(spriteNum,RECO_ROAM))
                A_PlaySound(RECO_ROAM,spriteNum);

            A_SetSprite(spriteNum,CLIPMASK0);

            goto next_sprite;
        }

        case GREENSLIME__:
        {
            // #ifndef VOLUMEONE
            if (!g_netServer && ud.multimode < 2)
            {
                if (g_noEnemies == 1)
                {
                    pSprite->cstat = 32768;
                    goto next_sprite;
                }
                else if (g_noEnemies == 2) pSprite->cstat = 257;
            }
            // #endif

            pData[1]+=128;

            if (sector[sectNum].floorstat&1)
                DELETE_SPRITE_AND_CONTINUE(spriteNum);

            int32_t             playerDist;
            int const           playerNum = A_FindPlayer(pSprite, &playerDist);
            auto const pPlayer   = g_player[playerNum].ps;

            if (playerDist > 20480)
            {
                if (++actor[spriteNum].timetosleep > SLEEPTIME)
                {
                    actor[spriteNum].timetosleep = 0;
                    changespritestat(spriteNum, STAT_ZOMBIEACTOR);
                    goto next_sprite;
                }
            }

            enum
            {
                GREENSLIME_FROZEN = -5,
                GREENSLIME_ONPLAYER,
                GREENSLIME_DEAD,  // set but not checked anywhere...
                GREENSLIME_EATINGACTOR,
                GREENSLIME_DONEEATING,
                GREENSLIME_ONFLOOR,
                GREENSLIME_TOCEILING,
                GREENSLIME_ONCEILING,
                GREENSLIME_TOFLOOR,
            };

            if (pData[0] == GREENSLIME_FROZEN)
            {
                pData[3]++;
                if (pData[3] > 280)
                {
                    pSprite->pal = 0;
                    pData[0] = GREENSLIME_ONFLOOR;
                    pData[3] = 0;
                    goto next_sprite;
                }
                A_Fall(spriteNum);

                pSprite->cstat  = 257;
                pSprite->picnum = GREENSLIME + 2;
                pSprite->extra  = 1;
                pSprite->pal    = 1;

                int const damageTile = A_IncurDamage(spriteNum);
                if (damageTile >= 0)
                {
                    if (damageTile == FREEZEBLAST)
                        goto next_sprite;

                    P_AddKills(pPlayer, 1);

                    for (bssize_t j = 16; j >= 0; --j)
                    {
                        int32_t newSprite = A_InsertSprite(SECT(spriteNum), SX(spriteNum), SY(spriteNum), SZ(spriteNum),
                                                           GLASSPIECES + (j % 3), -32, 36, 36, krand() & 2047, 32 + (krand() & 63),
                                                           1024 - (krand() & 1023), spriteNum, 5);
                        sprite[newSprite].pal = 1;
                    }

                    A_PlaySound(GLASS_BREAKING, spriteNum);
                    DELETE_SPRITE_AND_CONTINUE(spriteNum);
                }
                else if (playerDist < 1024 && pPlayer->quick_kick == 0)
                {
                    int const angDiff = G_GetAngleDelta(fix16_to_int(pPlayer->q16ang), getangle(SX(spriteNum) - pPlayer->pos.x,
                                                                               SY(spriteNum) - pPlayer->pos.y));

                    if (angDiff > -128 && angDiff < 128)
                        pPlayer->quick_kick = 14;
                }

                goto next_sprite;
            }

            pSprite->cstat = (playerDist < 1596) ? 0 : 257;

            if (pData[0] == GREENSLIME_ONPLAYER)
            {
                if (sprite[pPlayer->i].extra < 1 && pPlayer->somethingonplayer == spriteNum)
                {
                    pPlayer->somethingonplayer = -1;
                    pData[0] = GREENSLIME_TOFLOOR;
                    goto next_sprite;
                }

                setsprite(spriteNum,&pSprite->xyz);

                pSprite->ang = fix16_to_int(pPlayer->q16ang);

                if ((TEST_SYNC_KEY(g_player[playerNum].input.bits, SK_FIRE) || (pPlayer->quick_kick > 0)) && sprite[pPlayer->i].extra > 0)
                    if (pPlayer->quick_kick > 0 ||
                        (PWEAPON(playerNum, pPlayer->curr_weapon, WorksLike) != HANDREMOTE_WEAPON && PWEAPON(playerNum, pPlayer->curr_weapon, WorksLike) != HANDBOMB_WEAPON &&
                        PWEAPON(playerNum, pPlayer->curr_weapon, WorksLike) != TRIPBOMB_WEAPON && pPlayer->ammo_amount[pPlayer->curr_weapon] >= 0))
                    {
                        for (bssize_t x = 0; x < 8; ++x)
                        {
                            int const j
                            = A_InsertSprite(sectNum, pSprite->x, pSprite->y, pSprite->z - ZOFFSET3, SCRAP3 + (krand() & 3), -8, 48, 48,
                                             krand() & 2047, (krand() & 63) + 64, -(krand() & 4095) - (pSprite->zvel >> 2), spriteNum, 5);
                            sprite[j].pal = 6;
                        }

                        A_PlaySound(SLIM_DYING,spriteNum);
                        A_PlaySound(SQUISHED,spriteNum);

                        if ((krand()&255) < 32)
                        {
                            int const j = A_Spawn(spriteNum,BLOODPOOL);
                            sprite[j].pal = 0;
                        }

                        P_AddKills(pPlayer, 1);
                        pData[0] = GREENSLIME_DEAD;

                        if (pPlayer->somethingonplayer == spriteNum)
                            pPlayer->somethingonplayer = -1;

                        DELETE_SPRITE_AND_CONTINUE(spriteNum);
                    }

                pSprite->z = pPlayer->pos.z + pPlayer->pyoff - pData[2] + ZOFFSET3 + (fix16_to_int(F16(100) - pPlayer->q16horiz) << 4);

                if (pData[2] > 512)
                    pData[2] -= 128;

                if (pData[2] < 348)
                    pData[2] += 128;

                if (pPlayer->newowner >= 0)
                    G_ClearCameraView(pPlayer);

                if (pData[3] > 0)
                {
                    static const char slimeFrames[] = { 5, 5, 6, 6, 7, 7, 6, 5 };

                    Bassert(pData[3] < ARRAY_SSIZE(slimeFrames));

                    pSprite->picnum = GREENSLIME + slimeFrames[pData[3]];

                    if (pData[3] == 5)
                    {
                        sprite[pPlayer->i].extra += -(5 + (krand() & 3));
                        A_PlaySound(SLIM_ATTACK, spriteNum);
                    }

                    if (pData[3] < 7)
                        pData[3]++;
                    else
                        pData[3] = 0;
                }
                else
                {
                    pSprite->picnum = GREENSLIME + 5;
                    if (rnd(32))
                        pData[3] = 1;
                }

                pSprite->xrepeat = 20 + (sintable[pData[1] & 2047] >> 13);
                pSprite->yrepeat = 15 + (sintable[pData[1] & 2047] >> 13);
                pSprite->x       = pPlayer->pos.x + (sintable[(fix16_to_int(pPlayer->q16ang) + 512) & 2047] >> 7);
                pSprite->y       = pPlayer->pos.y + (sintable[fix16_to_int(pPlayer->q16ang) & 2047] >> 7);

                goto next_sprite;
            }
            else if (pSprite->xvel < 64 && playerDist < 768)
            {
                if (pPlayer->somethingonplayer == -1 && sprite[pPlayer->i].extra > 0)
                {
                    pPlayer->somethingonplayer = spriteNum;

                    if (pData[0] == GREENSLIME_TOFLOOR || pData[0] == GREENSLIME_ONCEILING)  // Falling downward
                        pData[2] = (12 << 8);
                    else
                        pData[2] = -(13 << 8);  // Climbing up player

                    pData[0] = GREENSLIME_ONPLAYER;
                }
            }

            int const damageTile = A_IncurDamage(spriteNum);
            if (damageTile >= 0)
            {
                A_PlaySound(SLIM_DYING,spriteNum);

                if (pPlayer->somethingonplayer == spriteNum)
                    pPlayer->somethingonplayer = -1;

                if (damageTile == FREEZEBLAST)
                {
                    A_PlaySound(SOMETHINGFROZE, spriteNum);
                    pData[0] = GREENSLIME_FROZEN;
                    pData[3] = 0;
                    goto next_sprite;
                }

                P_AddKills(pPlayer, 1);

                if ((krand()&255) < 32)
                {
                    int const j = A_Spawn(spriteNum,BLOODPOOL);
                    sprite[j].pal = 0;
                }

                for (bssize_t x=0; x<8; x++)
                {
                    int const j = A_InsertSprite(sectNum, pSprite->x, pSprite->y, pSprite->z - ZOFFSET3, SCRAP3 + (krand() & 3), -8,
                                                 48, 48, krand() & 2047, (krand() & 63) + 64, -(krand() & 4095) - (pSprite->zvel >> 2),
                                                 spriteNum, 5);
                    sprite[j].pal = 6;
                }
                pData[0] = GREENSLIME_DEAD;
                DELETE_SPRITE_AND_CONTINUE(spriteNum);
            }
            // All weap
            if (pData[0] == GREENSLIME_DONEEATING)
            {
                A_Fall(spriteNum);

                pSprite->cstat &= 65535-8;
                pSprite->picnum = GREENSLIME+4;

                if (pSprite->xrepeat > 32) pSprite->xrepeat -= krand()&7;
                if (pSprite->yrepeat > 16) pSprite->yrepeat -= krand()&7;
                else
                {
                    pSprite->xrepeat = 40;
                    pSprite->yrepeat = 16;
                    pData[5] = -1;
                    pData[0] = GREENSLIME_ONFLOOR;
                }

                goto next_sprite;
            }
            else if (pData[0] != GREENSLIME_EATINGACTOR) A_GetZLimits(spriteNum);

            if (pData[0] == GREENSLIME_EATINGACTOR) //On top of somebody
            {
                A_Fall(spriteNum);
                sprite[pData[5]].xvel = 0;

                int const ang = sprite[pData[5]].ang;
                pSprite->x    = sprite[pData[5]].x + (sintable[(ang + 512) & 2047] >> 11);
                pSprite->y    = sprite[pData[5]].y + (sintable[ang & 2047] >> 11);
                pSprite->z    = sprite[pData[5]].z;

                pSprite->picnum = GREENSLIME + 2 + (g_globalRandom & 1);

                if (pSprite->yrepeat < 64)
                    pSprite->yrepeat += 2;
                else
                {
                    if (pSprite->xrepeat < 32)
                        pSprite->xrepeat += 4;
                    else
                    {
                        pData[0]   = GREENSLIME_DONEEATING;
                        playerDist = ldist(pSprite, &sprite[pData[5]]);

                        if (playerDist < 768)
                        {
                            sprite[pData[5]].xrepeat = 0;

                            // JBF 20041129: a slimer eating another enemy really ought
                            // to decrease the maximum kill count by one.
                            if (sprite[pData[5]].extra > 0)
                                g_player[myconnectindex].ps->max_actors_killed--;
                        }
                    }
                }

                goto next_sprite;
            }

            //Check randomly to see of there is an actor near
            if (rnd(32))
            {
                for (bssize_t SPRITES_OF_SECT(sectNum, j))
                {
                    if (A_CheckSpriteFlags(j, SFLAG_GREENSLIMEFOOD))
                    {
                        if (ldist(pSprite, &sprite[j]) < 768 && (klabs(pSprite->z - sprite[j].z) < 8192))  // Gulp them
                        {
                            pData[5] = j;
                            pData[0] = GREENSLIME_EATINGACTOR;
                            pData[1] = 0;
                            goto next_sprite;
                        }
                    }
                }
            }

            //Moving on the ground or ceiling

            if (pData[0] == GREENSLIME_ONFLOOR || pData[0] == GREENSLIME_ONCEILING)
            {
                pSprite->picnum = GREENSLIME;

                if ((krand()&511) == 0)
                    A_PlaySound(SLIM_ROAM,spriteNum);

                if (pData[0]==GREENSLIME_ONCEILING)
                {
                    pSprite->zvel = 0;
                    pSprite->cstat &= (65535-8);

                    if ((sector[sectNum].ceilingstat&1) || (actor[spriteNum].ceilingz+6144) < pSprite->z)
                    {
                        pSprite->z += 2048;
                        pData[0] = GREENSLIME_TOFLOOR;
                        goto next_sprite;
                    }
                }
                else
                {
                    pSprite->cstat |= 8;
                    A_Fall(spriteNum);
                }

                if (everyothertime&1) A_SetSprite(spriteNum,CLIPMASK0);

                if (pSprite->xvel > 96)
                {
                    pSprite->xvel -= 2;
                    goto next_sprite;
                }
                else
                {
                    pSprite->xvel = 64 - (sintable[(pData[1]+512)&2047]>>9);

                    pSprite->ang += G_GetAngleDelta(pSprite->ang,
                                              getangle(pPlayer->pos.x-pSprite->x,pPlayer->pos.y-pSprite->y))>>3;
                    // TJR
                }

                pSprite->xrepeat = 36 + (sintable[(pData[1]+512)&2047]>>11);
                pSprite->yrepeat = 16 + (sintable[pData[1]&2047]>>13);

                if (rnd(4) && (sector[sectNum].ceilingstat&1) == 0 &&
                        klabs(actor[spriteNum].floorz-actor[spriteNum].ceilingz)
                        < (192<<8))
                {
                    pSprite->zvel = 0;
                    pData[0]++;
                }

            }

            if (pData[0]==GREENSLIME_TOCEILING)
            {
                pSprite->picnum = GREENSLIME;
                if (pSprite->yrepeat < 40) pSprite->yrepeat+=8;
                if (pSprite->xrepeat > 8) pSprite->xrepeat-=4;
                if (pSprite->zvel > -(2048+1024))
                    pSprite->zvel -= 348;
                pSprite->z += pSprite->zvel;
                if (pSprite->z < actor[spriteNum].ceilingz+4096)
                {
                    pSprite->z = actor[spriteNum].ceilingz+4096;
                    pSprite->xvel = 0;
                    pData[0] = GREENSLIME_ONCEILING;
                }
            }

            if (pData[0]==GREENSLIME_TOFLOOR)
            {
                pSprite->picnum = GREENSLIME+1;

                A_Fall(spriteNum);

                if (pSprite->z > actor[spriteNum].floorz-ZOFFSET3)
                {
                    pSprite->yrepeat-=4;
                    pSprite->xrepeat+=2;
                }
                else
                {
                    if (pSprite->yrepeat < (40-4)) pSprite->yrepeat+=8;
                    if (pSprite->xrepeat > 8) pSprite->xrepeat-=4;
                }

                if (pSprite->z > actor[spriteNum].floorz-2048)
                {
                    pSprite->z = actor[spriteNum].floorz-2048;
                    pData[0] = GREENSLIME_ONFLOOR;
                    pSprite->xvel = 0;
                }
            }
            goto next_sprite;
        }

        case BOUNCEMINE__:
        if (pSprite->xvel != 0)
        case MORTER__:
        {
            int const j        = A_Spawn(spriteNum, (PLUTOPAK ? FRAMEEFFECT1 : FRAMEEFFECT1_13));
            actor[j].t_data[0] = 3;
        }
            fallthrough__;
        case HEAVYHBOMB__:
        {
            int           playerNum;
            DukePlayer_t *pPlayer;
            int           detonatePlayer;

            if ((pSprite->cstat&32768))
            {
                if (--pData[2] <= 0)
                {
                    A_PlaySound(TELEPORTER, spriteNum);
                    A_Spawn(spriteNum, TRANSPORTERSTAR);
                    pSprite->cstat = 257;
                }
                goto next_sprite;
            }

            int32_t playerDist;
            playerNum = A_FindPlayer(pSprite, &playerDist);
            pPlayer   = g_player[playerNum].ps;

            if (playerDist < 1220)
                pSprite->cstat &= ~257;
            else
                pSprite->cstat |= 257;

            if (pData[3] == 0)
            {
                if (A_IncurDamage(spriteNum) >= 0)
                {
                    pData[3]       = 1;
                    pData[2]       = 0;
                    detonatePlayer = 0;
                    pSprite->xvel  = 0;
                    goto DETONATEB;
                }
            }

            if (pSprite->picnum != BOUNCEMINE)
            {
                A_Fall(spriteNum);

                if ((sector[sectNum].lotag != ST_1_ABOVE_WATER || actor[spriteNum].floorz != sector[sectNum].floorz) && pSprite->z >= actor[spriteNum].floorz - AC_FZOFFSET(spriteNum) && pSprite->yvel < 3)
                {
                    if (pSprite->yvel > 0 || (pSprite->yvel == 0 && actor[spriteNum].floorz == sector[sectNum].floorz))
                        A_PlaySound(PIPEBOMB_BOUNCE,spriteNum);
                    pSprite->zvel = -((4-pSprite->yvel)<<8);
                    if (sector[pSprite->sectnum].lotag == ST_2_UNDERWATER)
                        pSprite->zvel >>= 2;
                    pSprite->yvel++;
                }
                if (pSprite->z < actor[spriteNum].ceilingz)   // && sector[sect].lotag != ST_2_UNDERWATER )
                {
                    pSprite->z = actor[spriteNum].ceilingz+(3<<8);
                    pSprite->zvel = 0;
                }
            }

            // can't initialize this because of the goto above
            int moveSprite;
            moveSprite = A_MoveSprite(spriteNum, { (pSprite->xvel * (sintable[(pSprite->ang + 512) & 2047])) >> 14,
                                                   (pSprite->xvel * (sintable[pSprite->ang & 2047])) >> 14, pSprite->zvel }, CLIPMASK0);

            actor[spriteNum].movflag = moveSprite;

            if (sector[SECT(spriteNum)].lotag == ST_1_ABOVE_WATER && pSprite->zvel == 0 && actor[spriteNum].floorz == sector[sectNum].floorz)
            {
                pSprite->z += ZOFFSET5;
                if (pData[5] == 0)
                {
                    pData[5] = 1;
                    A_Spawn(spriteNum,WATERSPLASH2);
                }
            }
            else pData[5] = 0;

            if (pData[3] == 0 && (pSprite->picnum == BOUNCEMINE || pSprite->picnum == MORTER) && (moveSprite || playerDist < 844))
            {
                pData[3] = 1;
                pData[2] = 0;
                detonatePlayer = 0;
                pSprite->xvel = 0;
                goto DETONATEB;
            }

            if (sprite[pSprite->owner].picnum == APLAYER)
                detonatePlayer = P_Get(pSprite->owner);
            else detonatePlayer = -1;

            if (pSprite->xvel > 0)
            {
                pSprite->xvel -= 5;
                if (sector[sectNum].lotag == ST_2_UNDERWATER)
                    pSprite->xvel -= 10;

                if (pSprite->xvel < 0)
                    pSprite->xvel = 0;
                if (pSprite->xvel&8) pSprite->cstat ^= 4;
            }

            if ((moveSprite&49152) == 32768)
            {
                moveSprite &= (MAXWALLS - 1);
                A_DamageWall(spriteNum, moveSprite, pSprite->xyz, pSprite->picnum);
                Proj_BounceOffWall(pSprite, moveSprite);
                pSprite->xvel >>= 1;
            }

DETONATEB:
            // Pipebomb control set to timer? (see player.c)
            // TIMER_CONTROL
            if (pSprite->picnum == HEAVYHBOMB && pData[6] == 1)
            {
                if (pData[7] >= 1)
                    pData[7]--;

                if (pData[7] <= 0)
                    pData[6] = 3;
            }

            if ((detonatePlayer >= 0 && g_player[detonatePlayer].ps->hbomb_on == 0 && pData[6] == 2) || pData[3] == 1)
                pData[6] = 3;

            if (pData[6] == 3)
            {
                pData[2]++;

                if (pData[2] == 2)
                {
                    int const x      = pSprite->extra;
                    int       radius = 0;

                    switch (tileGetMapping(pSprite->picnum))
                    {
                        case HEAVYHBOMB__: radius = g_pipebombRadius; break;
                        case MORTER__: radius     = g_morterRadius; break;
                        case BOUNCEMINE__: radius = g_bouncemineRadius; break;
                    }

                    A_RadiusDamage(spriteNum, radius, x >> 2, x >> 1, x - (x >> 2), x);

                    int const j = A_Spawn(spriteNum, EXPLOSION2);
                    A_PlaySound(PIPEBOMB_EXPLODE, j);

                    if (pSprite->zvel == 0)
                        A_Spawn(spriteNum,EXPLOSION2BOT);

                    for (bssize_t x = 0; x < 8; ++x)
                        RANDOMSCRAP(pSprite, spriteNum);
                }

                if (pSprite->yrepeat)
                {
                    pSprite->yrepeat = 0;
                    goto next_sprite;
                }

                if (pData[2] > 20)
                {
                    if (pSprite->owner != spriteNum || ud.respawn_items == 0)
                    {
                        DELETE_SPRITE_AND_CONTINUE(spriteNum);
                    }
                    else
                    {
                        pData[2] = g_itemRespawnTime;
                        A_Spawn(spriteNum,RESPAWNMARKERRED);
                        pSprite->cstat = 32768;
                        pSprite->yrepeat = 9;
                        goto next_sprite;
                    }
                }
            }
            else if (pSprite->picnum == HEAVYHBOMB && playerDist < 788 && pData[0] > 7 && pSprite->xvel == 0)
            {
                if (cansee(pSprite->x, pSprite->y, pSprite->z - ZOFFSET3, pSprite->sectnum,
                           pPlayer->pos.x, pPlayer->pos.y, pPlayer->pos.z, pPlayer->cursectnum))
                {
                    if (pPlayer->ammo_amount[HANDBOMB_WEAPON] < pPlayer->max_ammo_amount[HANDBOMB_WEAPON])
                    {
                        if ((g_gametypeFlags[ud.coop] & GAMETYPE_WEAPSTAY) && pSprite->owner == spriteNum)
                        {
                            for (bssize_t j = 0; j < pPlayer->weapreccnt; j++)
                            {
                                if (pPlayer->weaprecs[j] == pSprite->picnum)
                                    goto next_sprite;
                            }

                            if (pPlayer->weapreccnt < MAX_WEAPONS)
                                pPlayer->weaprecs[pPlayer->weapreccnt++] = pSprite->picnum;
                        }

                        P_AddAmmo(pPlayer, HANDBOMB_WEAPON, 1);
                        A_PlaySound(DUKE_GET, pPlayer->i);

                        if ((pPlayer->gotweapon & (1<<HANDBOMB_WEAPON)) == 0 || pSprite->owner == pPlayer->i)
                        {
                            int doSwitch = ((pPlayer->weaponswitch & 1) ||
                                PWEAPON(playerNum, pPlayer->curr_weapon, WorksLike) == HANDREMOTE_WEAPON);
                            P_AddWeapon(pPlayer, HANDBOMB_WEAPON, doSwitch);
                        }

                        if (sprite[pSprite->owner].picnum != APLAYER)
                            P_PalFrom(pPlayer, 32, 0, 32, 0);

                        if (pSprite->owner != spriteNum || ud.respawn_items == 0)
                        {
                            if (pSprite->owner == spriteNum && (g_gametypeFlags[ud.coop] & GAMETYPE_WEAPSTAY))
                                goto next_sprite;
                            DELETE_SPRITE_AND_CONTINUE(spriteNum);
                        }
                        else
                        {
                            pData[2] = g_itemRespawnTime;
                            A_Spawn(spriteNum, RESPAWNMARKERRED);
                            pSprite->cstat = 32768;
                        }
                    }
                }
            }

            if (pData[0] < 8)
                pData[0]++;

            goto next_sprite;
        }

        case REACTORBURNT__:
        case REACTOR2BURNT__:
            goto next_sprite;

        case REACTOR__:
        case REACTOR2__:
        {
            if (pData[4] == 1)
            {
                for (bssize_t SPRITES_OF_SECT(sectNum, j))
                {
                    switch (tileGetMapping(sprite[j].picnum))
                    {
                    case SECTOREFFECTOR__:
                        if (sprite[j].lotag == 1)
                        {
                            sprite[j].lotag = 65535u;
                            sprite[j].hitag = 65535u;
                        }
                        break;
                    case REACTOR__:
                        sprite[j].picnum = REACTORBURNT;
                        break;
                    case REACTOR2__:
                        sprite[j].picnum = REACTOR2BURNT;
                        break;
                    case REACTORSPARK__:
                    case REACTOR2SPARK__:
                        sprite[j].cstat = 32768;
                        break;
                    }
                }

                goto next_sprite;
            }

            if (pData[1] >= 20)
            {
                pData[4] = 1;
                goto next_sprite;
            }

            int32_t    playerDist;
            int        playerNum = A_FindPlayer(pSprite, &playerDist);
            auto const pPlayer   = g_player[playerNum].ps;

            if (++pData[2] == 4)
                pData[2] = 0;

            if (playerDist < 4096)
            {
                if ((krand() & 255) < 16)
                {
                    if (!A_CheckSoundPlaying(pPlayer->i, DUKE_LONGTERM_PAIN))
                        A_PlaySound(DUKE_LONGTERM_PAIN, pPlayer->i);

                    A_PlaySound(SHORT_CIRCUIT, spriteNum);
                    sprite[pPlayer->i].extra--;
                    P_PalFrom(pPlayer, 32, 32, 0, 0);
                }

                pData[0] += 128;

                if (pData[3] == 0)
                    pData[3] = 1;
            }
            else pData[3] = 0;

            if (pData[1])
            {
                pData[1]++;
                pData[4]   = pSprite->z;
                pSprite->z = sector[sectNum].floorz - (krand() % (sector[sectNum].floorz - sector[sectNum].ceilingz));

                switch (pData[1])
                {
                    case 3:
                        // Turn on all of those flashing sectoreffector.
                        A_RadiusDamage(spriteNum, 4096, g_impactDamage << 2, g_impactDamage << 2, g_impactDamage << 2, g_impactDamage << 2);

                        for (bssize_t SPRITES_OF(STAT_STANDABLE, j))
                        {
                            if (sprite[j].picnum == MASTERSWITCH && sprite[j].hitag == pSprite->hitag && sprite[j].yvel == 0)
                                sprite[j].yvel = 1;
                        }
                        break;

                    case 4:
                    case 7:
                    case 10:
                    case 15:
                        for (bssize_t SPRITES_OF_SECT(sectNum, j))
                        {
                            if (j != spriteNum)
                            {
                                A_DeleteSprite(j);
                                break;
                            }
                        }

                        break;
                }

                for (bssize_t x = 0; x < 16; x++)
                    RANDOMSCRAP(pSprite, spriteNum);

                pSprite->z = pData[4];
                pData[4]   = 0;
            }
            else if (A_IncurDamage(spriteNum) >= 0)
            {
                for (bssize_t x = 0; x < 32; x++)
                    RANDOMSCRAP(pSprite, spriteNum);

                if (pSprite->extra < 0)
                    pData[1] = 1;
            }
            goto next_sprite;
        }
        }
#endif // EDUKE32_STANDALONE

        if (!g_netServer && ud.multimode < 2 && A_CheckEnemySprite(pSprite))
        {
            if (g_noEnemies == 1)
            {
                pSprite->cstat = 32768;
                goto next_sprite;
            }
            else if (g_noEnemies == 2)
            {
                pSprite->cstat = 0;
                if (pSprite->extra)
                    pSprite->cstat = 257;
            }
        }

        if (G_TileHasActor(sprite[spriteNum].picnum))
        {
            int32_t playerDist;
            int const playerNum = A_FindPlayer(pSprite, &playerDist);
            A_Execute(spriteNum, playerNum, playerDist);
        }
next_sprite:
        A_MaybeAwakenBadGuys(spriteNum);
        spriteNum = nextSprite;
    }
}

ACTOR_STATIC void G_MoveMisc(void)  // STATNUM 5
{
    int spriteNum = headspritestat[STAT_MISC];

    while (spriteNum >= 0)
    {
        int const  nextSprite = nextspritestat[spriteNum];
        int32_t    playerDist;
        auto const pData   = actor[spriteNum].t_data;
        auto const pSprite = &sprite[spriteNum];
        int        sectNum = pSprite->sectnum;  // XXX: not const
        int        switchPic;

        if (sectNum < 0 || pSprite->xrepeat == 0)
            DELETE_SPRITE_AND_CONTINUE(spriteNum);

        switchPic = pSprite->picnum;

#ifndef EDUKE32_STANDALONE
        if (pSprite->picnum > NUKEBUTTON && pSprite->picnum <= NUKEBUTTON+3)
            switchPic = NUKEBUTTON;

        if (pSprite->picnum > GLASSPIECES && pSprite->picnum <= GLASSPIECES+2)
            switchPic = GLASSPIECES;

        if (pSprite->picnum == INNERJAW+1)
            switchPic--;

        if ((pSprite->picnum == MONEY+1) || (pSprite->picnum == MAIL+1) || (pSprite->picnum == PAPER+1))
            actor[spriteNum].floorz = pSprite->z = getflorzofslope(pSprite->sectnum,pSprite->x,pSprite->y);
        else
#endif
        {
            switch (tileGetMapping(switchPic))
            {
                case APLAYER__: pSprite->cstat = 32768; goto next_sprite;
                case FRAMEEFFECT1_13__:
                    if (PLUTOPAK) goto next_sprite;	// JBF: ideally this should never happen...
                    fallthrough__;
                case FRAMEEFFECT1__:

                    if (pSprite->owner >= 0)
                    {
                        pData[0]++;

                        if (pData[0] > 7)
                        {
                            DELETE_SPRITE_AND_CONTINUE(spriteNum);
                        }
                        else if (pData[0] > 4)
                            pSprite->cstat |= 512+2;
                        else if (pData[0] > 2)
                            pSprite->cstat |= 2;
                        pSprite->xoffset = sprite[pSprite->owner].xoffset;
                        pSprite->yoffset = sprite[pSprite->owner].yoffset;
                    }
                    goto next_sprite;

#ifndef EDUKE32_STANDALONE
                case ONFIRESMOKE__:
                case ONFIRE__:
                case BURNEDCORPSE__:
                case LAVAPOOLBUBBLE__:
                case WHISPYSMOKE__:
                case LAVAPOOL__:
                    if (!WORLDTOUR)
                        goto next_sprite;
                    fallthrough__;
#endif
                case EXPLOSION2__:
                case EXPLOSION2BOT__:
                case FORCERIPPLE__:
                case TRANSPORTERSTAR__:
                case TRANSPORTERBEAM__:
                case SMALLSMOKE__:
#ifndef EDUKE32_STANDALONE
                case WATERBUBBLE__:
                case BURNING__:
                case BURNING2__:
                case FECES__:
                case SHRINKEREXPLOSION__:
                case BLOOD__:
                case LASERSITE__:
#endif
                {
                    if (!G_TileHasActor(sprite[spriteNum].picnum))
                        goto next_sprite;
                    int const playerNum = A_FindPlayer(pSprite, &playerDist);
                    A_Execute(spriteNum, playerNum, playerDist);
                    goto next_sprite;
                }
            }

#ifndef EDUKE32_STANDALONE
            if (!FURY)
            switch (tileGetMapping(switchPic))
            {
                case NEON1__:
                case NEON2__:
                case NEON3__:
                case NEON4__:
                case NEON5__:
                case NEON6__:
                    pSprite->shade = ((tabledivide32_noinline(g_globalRandom, pSprite->lotag + 1) & 31) > 4) ? -127 : 127;
                    goto next_sprite;

                case BLOODSPLAT1__:
                case BLOODSPLAT2__:
                case BLOODSPLAT3__:
                case BLOODSPLAT4__:
                    if (pData[0] == 3 * GAMETICSPERSEC)
                        goto next_sprite;

                    actor[spriteNum].bpos.z -= pSprite->z;

                    if ((++pData[0] % 9) == 0)
                    {
                        pSprite->yrepeat++;
                        pSprite->z += (tilesiz[pSprite->picnum].y * pSprite->yrepeat) >> 2;
                    }
                    else
                        pSprite->z += 16 + (krand() & 15);

                    actor[spriteNum].bpos.z += pSprite->z;
                    goto next_sprite;

                case NUKEBUTTON__:
                    //        case NUKEBUTTON+1:
                    //        case NUKEBUTTON+2:
                    //        case NUKEBUTTON+3:

                    if (pData[0])
                    {
                        pData[0]++;
                        if (pData[0] == 8)
                            pSprite->picnum = NUKEBUTTON + 1;
                        else if (pData[0] == 16)
                        {
                            pSprite->picnum = NUKEBUTTON + 2;
                            g_player[P_Get(pSprite->owner)].ps->fist_incs = 1;
                        }
                        if (g_player[P_Get(pSprite->owner)].ps->fist_incs == GAMETICSPERSEC)
                            pSprite->picnum = NUKEBUTTON + 3;
                    }
                    goto next_sprite;

                case FORCESPHERE__:
                {
                    int forceRepeat = pSprite->xrepeat;
                    if (pData[1] > 0)
                    {
                        pData[1]--;
                        if (pData[1] == 0)
                            DELETE_SPRITE_AND_CONTINUE(spriteNum);
                    }
                    if (actor[pSprite->owner].t_data[1] == 0)
                    {
                        if (pData[0] < 64)
                        {
                            pData[0]++;
                            forceRepeat += 3;
                        }
                    }
                    else if (pData[0] > 64)
                    {
                        pData[0]--;
                        forceRepeat -= 3;
                    }

                    pSprite->xyz = sprite[pSprite->owner].xyz;
                    pSprite->ang += actor[pSprite->owner].t_data[0];

                    forceRepeat      = clamp2(forceRepeat, 1, 64);
                    pSprite->xrepeat = forceRepeat;
                    pSprite->yrepeat = forceRepeat;
                    pSprite->shade   = (forceRepeat >> 1) - 48;

                    for (int j = pData[0]; j > 0; j--)
                        A_SetSprite(spriteNum, CLIPMASK0);
                    goto next_sprite;
                }

            case WATERSPLASH2__:
                pData[0]++;
                if (pData[0] == 1)
                {
                    if (sector[sectNum].lotag != ST_1_ABOVE_WATER && sector[sectNum].lotag != ST_2_UNDERWATER)
                        DELETE_SPRITE_AND_CONTINUE(spriteNum);
                    /*
                    else
                    {
                        l = getflorzofslope(sect,s->x,s->y)-s->z;
                        if( l > ZOFFSET2 ) KILLIT(i);
                    }
                    else
                    */
                    if (!S_CheckSoundPlaying(ITEM_SPLASH))
                        A_PlaySound(ITEM_SPLASH,spriteNum);
                }
                if (pData[0] == 3)
                {
                    pData[0] = 0;
                    pData[1]++;  // WATERSPLASH_T2
                }
                if (pData[1] == 5)
                    A_DeleteSprite(spriteNum);
                goto next_sprite;
            case INNERJAW__:
            {
                //        case INNERJAW+1:
                int32_t playerDist, playerNum = A_FindPlayer(pSprite,&playerDist);

                if (playerDist < 512)
                {
                    P_PalFrom(g_player[playerNum].ps, 32, 32,0,0);
                    sprite[g_player[playerNum].ps->i].extra -= 4;
                }
            }
            fallthrough__;
            case FIRELASER__:
                if (pSprite->extra != 5)
                    pSprite->extra = 5;
                else DELETE_SPRITE_AND_CONTINUE(spriteNum);
                break;
            case TONGUE__:
                DELETE_SPRITE_AND_CONTINUE(spriteNum);

            case MONEY__:
            case MAIL__:
            case PAPER__:
            {
                pSprite->xvel = (krand()&7)+(sintable[T1(spriteNum)&2047]>>9);
                T1(spriteNum) += (krand()&63);
                if ((T1(spriteNum)&2047) > 512 && (T1(spriteNum)&2047) < 1536)
                {
                    if (sector[sectNum].lotag == ST_2_UNDERWATER)
                    {
                        if (pSprite->zvel < 64)
                            pSprite->zvel += (g_spriteGravity>>5)+(krand()&7);
                    }
                    else if (pSprite->zvel < 144)
                        pSprite->zvel += (g_spriteGravity>>5)+(krand()&7);
                }

                A_SetSprite(spriteNum, CLIPMASK0);

                if ((krand()&3) == 0)
                    setsprite(spriteNum, &pSprite->xyz);

                if (pSprite->sectnum == -1)
                    DELETE_SPRITE_AND_CONTINUE(spriteNum);

                int const floorZ = getflorzofslope(pSprite->sectnum, pSprite->x, pSprite->y);

                if (pSprite->z > floorZ)
                {
                    pSprite->z = floorZ;
                    A_AddToDeleteQueue(spriteNum);
                    PN(spriteNum)++;

                    for (bssize_t SPRITES_OF(STAT_MISC, j))
                    {
                        if (sprite[j].picnum == BLOODPOOL && ldist(pSprite, &sprite[j]) < 348)
                        {
                            pSprite->pal = 2;
                            break;
                        }
                    }
                }

                break;
            }

            case JIBS1__:
            case JIBS2__:
            case JIBS3__:
            case JIBS4__:
            case JIBS5__:
            case JIBS6__:
            case HEADJIB1__:
            case ARMJIB1__:
            case LEGJIB1__:
            case LIZMANHEAD1__:
            case LIZMANARM1__:
            case LIZMANLEG1__:
            case DUKETORSO__:
            case DUKEGUN__:
            case DUKELEG__:
            {
                pSprite->xvel = (pSprite->xvel > 0) ? pSprite->xvel - 1 : 0;

                if (++pData[5] == (30*10))
                    DELETE_SPRITE_AND_CONTINUE(spriteNum);

                if (pSprite->zvel > 1024 && pSprite->zvel < 1280)
                {
                    setsprite(spriteNum, &pSprite->xyz);
                    sectNum = pSprite->sectnum;
                }

                int32_t floorZ, ceilZ;
                getzsofslope(sectNum, pSprite->x, pSprite->y, &ceilZ, &floorZ);

                if (ceilZ == floorZ || sectNum < 0 || sectNum >= MAXSECTORS)
                    DELETE_SPRITE_AND_CONTINUE(spriteNum);

                if (pSprite->z < floorZ-(2<<8))
                {
                    if (pData[1] < 2) pData[1]++;
                    else if (sector[sectNum].lotag != ST_2_UNDERWATER)
                    {
                        pData[1] = 0;

                        if (pSprite->picnum == DUKELEG || pSprite->picnum == DUKETORSO || pSprite->picnum == DUKEGUN)
                        {
                            pData[0] = (pData[0] > 6) ? 0 : pData[0] + 1;
                        }
                        else
                        {
                            pData[0] = (pData[0] > 2) ? 0 : pData[0] + 1;
                        }
                    }

                    if (pSprite->zvel < ACTOR_MAXFALLINGZVEL)
                    {
                        if (sector[sectNum].lotag == ST_2_UNDERWATER)
                        {
                            if (pSprite->zvel < 1024)
                                pSprite->zvel += 48;
                            else pSprite->zvel = 1024;
                        }
                        else pSprite->zvel += g_spriteGravity-50;
                    }

                    pSprite->x += (pSprite->xvel*sintable[(pSprite->ang+512)&2047])>>14;
                    pSprite->y += (pSprite->xvel*sintable[pSprite->ang&2047])>>14;
                    pSprite->z += pSprite->zvel;
                }
                else
                {
                    if (pData[2] == 0)
                    {
                        if (pSprite->sectnum == -1)
                            DELETE_SPRITE_AND_CONTINUE(spriteNum);

                        if ((sector[pSprite->sectnum].floorstat&2))
                            DELETE_SPRITE_AND_CONTINUE(spriteNum);

                        pData[2]++;
                    }

                    floorZ        = getflorzofslope(pSprite->sectnum, pSprite->x, pSprite->y);
                    pSprite->z    = floorZ - (2 << 8);
                    pSprite->xvel = 0;

                    if (pSprite->picnum == JIBS6)
                    {
                        pData[1]++;

                        if ((pData[1]&3) == 0 && pData[0] < 7)
                            pData[0]++;

                        if (pData[1] > 20)
                            DELETE_SPRITE_AND_CONTINUE(spriteNum);
                    }
                    else
                    {
                        pSprite->picnum = JIBS6;
                        pData[0] = 0;
                        pData[1] = 0;
                    }
                }
                goto next_sprite;
            }

            case BLOODPOOL__:
            case PUKE__:
            {
                if (pData[0] == 0)
                {
                    pData[0] = 1;
                    if (sector[sectNum].floorstat&2)
                    {
                        DELETE_SPRITE_AND_CONTINUE(spriteNum);
                    }
                    else A_AddToDeleteQueue(spriteNum);
                }

                A_Fall(spriteNum);

                int32_t   playerDist;
                int const playerNum = A_FindPlayer(pSprite, &playerDist);

                pSprite->z = actor[spriteNum].floorz - 1;

                auto const pPlayer = g_player[playerNum].ps;

                if (pData[2] < 32)
                {
                    pData[2]++;

                    if (actor[spriteNum].htpicnum == TIRE)
                    {
                        if (pSprite->xrepeat < 64 && pSprite->yrepeat < 64)
                        {
                            pSprite->xrepeat += krand()&3;
                            pSprite->yrepeat += krand()&3;
                        }
                    }
                    else
                    {
                        if (pSprite->xrepeat < 32 && pSprite->yrepeat < 32)
                        {
                            pSprite->xrepeat += krand()&3;
                            pSprite->yrepeat += krand()&3;
                        }
                    }
                }

                if (playerDist < 844 && pSprite->xrepeat > 6 && pSprite->yrepeat > 6)
                {
                    if (pSprite->pal == 0 && pSprite->picnum != PUKE && (krand()&255) < 16)
                    {
                        if (pPlayer->inv_amount[GET_BOOTS] > 0)
                            pPlayer->inv_amount[GET_BOOTS]--;
                        else
                        {
                            if (!A_CheckSoundPlaying(pPlayer->i,DUKE_LONGTERM_PAIN))
                                A_PlaySound(DUKE_LONGTERM_PAIN,pPlayer->i);

                            sprite[pPlayer->i].extra --;

                            P_PalFrom(pPlayer, 32, 16,0,0);
                        }
                    }

                    if (pData[1] == 1) goto next_sprite;

                    pData[1] = 1;

                    pPlayer->footprintcount = (actor[spriteNum].htpicnum == TIRE) ? 10 : 3;
                    pPlayer->footprintpal   = (pSprite->pal == 0 && pSprite->picnum != PUKE) ? 8 : pSprite->pal;
                    pPlayer->footprintshade = pSprite->shade;

                    if (pData[2] == 32)
                    {
                        pSprite->xrepeat -= 6;
                        pSprite->yrepeat -= 6;
                    }
                }
                else pData[1] = 0;
                goto next_sprite;
            }

            case SHELL__:
            case SHOTGUNSHELL__:

                A_SetSprite(spriteNum,CLIPMASK0);

                if (sectNum < 0 || (sector[sectNum].floorz + 256) < pSprite->z)
                    DELETE_SPRITE_AND_CONTINUE(spriteNum);

                if (sector[sectNum].lotag == ST_2_UNDERWATER)
                {
                    pData[1]++;
                    if (pData[1] > 8)
                    {
                        pData[1] = 0;
                        pData[0]++;
                        pData[0] &= 3;
                    }
                    if (pSprite->zvel < 128) pSprite->zvel += (g_spriteGravity/13); // 8
                    else pSprite->zvel -= 64;
                    if (pSprite->xvel > 0)
                        pSprite->xvel -= 4;
                    else pSprite->xvel = 0;
                }
                else
                {
                    pData[1]++;
                    if (pData[1] > 3)
                    {
                        pData[1] = 0;
                        pData[0]++;
                        pData[0] &= 3;
                    }
                    if (pSprite->zvel < 512) pSprite->zvel += (g_spriteGravity/3); // 52;
                    if (pSprite->xvel > 0)
                        pSprite->xvel --;
                    //                else KILLIT(i);
                }

                goto next_sprite;

            case GLASSPIECES__:
                //        case GLASSPIECES+1:
                //        case GLASSPIECES+2:

                A_Fall(spriteNum);

                if (pSprite->zvel > 4096) pSprite->zvel = 4096;
                if (sectNum < 0)
                    DELETE_SPRITE_AND_CONTINUE(spriteNum);

                if (pSprite->z == actor[spriteNum].floorz-AC_FZOFFSET(spriteNum) && pData[0] < 3)
                {
                    pSprite->zvel = -((3-pData[0])<<8)-(krand()&511);
                    if (sector[sectNum].lotag == ST_2_UNDERWATER)
                        pSprite->zvel >>= 1;
                    pSprite->xrepeat >>= 1;
                    pSprite->yrepeat >>= 1;
                    if (rnd(96))
                        setsprite(spriteNum,&pSprite->xyz);
                    pData[0]++;//Number of bounces
                }
                else if (pData[0] == 3)
                    DELETE_SPRITE_AND_CONTINUE(spriteNum);

                if (pSprite->xvel > 0)
                {
                    pSprite->xvel -= 2;
                    pSprite->cstat = ((pSprite->xvel&3)<<2);
                }
                else pSprite->xvel = 0;

                A_SetSprite(spriteNum,CLIPMASK0);

                goto next_sprite;

            case FIREFLYFLYINGEFFECT__:
                if (WORLDTOUR && G_TileHasActor(sprite[spriteNum].picnum))
                {
                    int playerDist;
                    int const playerNum = A_FindPlayer(pSprite, &playerDist);
                    A_Execute(spriteNum, playerNum, playerDist);
                    spritetype *pPlayer = &sprite[g_player[playerNum].ps->i];
                    spritetype* pOwner = &sprite[pSprite->owner];
                    if (pOwner->picnum != FIREFLY) DELETE_SPRITE_AND_CONTINUE(spriteNum);
                    if (pOwner->xrepeat < 24 && pOwner->pal != 1)
                        pSprite->cstat &= ~32768;
                    else
                        pSprite->cstat |= 32768;
                    float dx = pOwner->x-pPlayer->x;
                    float dy = pOwner->y-pPlayer->y;
                    float dn = sqrt(dx*dx+dy*dy);
                    if (dn > 0.f)
                    {
                        dx /= dn;
                        dy /= dn;
                    }
                    pSprite->x = pOwner->x-int(dx*-10.f);
                    pSprite->y = pOwner->y-int(dy*-10.f);
                    pSprite->z = pOwner->z+0x800;
                    if (pOwner->extra <= 0) DELETE_SPRITE_AND_CONTINUE(spriteNum);
                }
                goto next_sprite;
            }
#endif
        }

#ifndef EDUKE32_STANDALONE
        if (!FURY && PN(spriteNum) >= SCRAP6 && PN(spriteNum) <= SCRAP5+3)
        {
            if (pSprite->xvel > 0)
                pSprite->xvel--;
            else pSprite->xvel = 0;

            if (pSprite->zvel > 1024 && pSprite->zvel < 1280)
            {
                setsprite(spriteNum,&pSprite->xyz);
                sectNum = pSprite->sectnum;
            }

            if (pSprite->z < sector[sectNum].floorz-(2<<8))
            {
                if (pData[1] < 1) pData[1]++;
                else
                {
                    pData[1] = 0;

                    if (pSprite->picnum < SCRAP6 + 8)
                        pData[0] = (pData[0] > 6) ? 0 : pData[0] + 1;
                    else
                        pData[0] = (pData[0] > 2) ? 0 : pData[0] + 1;
                }
                if (pSprite->zvel < 4096)
                    pSprite->zvel += g_spriteGravity - 50;
                pSprite->x += (pSprite->xvel*sintable[(pSprite->ang+512)&2047])>>14;
                pSprite->y += (pSprite->xvel*sintable[pSprite->ang&2047])>>14;
                pSprite->z += pSprite->zvel;
            }
            else
            {
                if (pSprite->picnum == SCRAP1 && pSprite->yvel > 0 && pSprite->yvel < MAXUSERTILES)
                {
                    int32_t j = A_Spawn(spriteNum, pSprite->yvel);

                    setsprite(j,&pSprite->xyz);
                    A_GetZLimits(j);
                    sprite[j].hitag = sprite[j].lotag = 0;
                }

                DELETE_SPRITE_AND_CONTINUE(spriteNum);
            }
            goto next_sprite;
        }
#endif

next_sprite:
        spriteNum = nextSprite;
    }
}


// i: SE spritenum
static void HandleSE31(int spriteNum, int setFloorZ, int spriteZ, int SEdir, int zDifference)
{
    auto const pSprite = &sprite[spriteNum];
    auto const pSector = &sector[sprite[spriteNum].sectnum];
    auto const pData   = actor[spriteNum].t_data;

    if (klabs(pSector->floorz - spriteZ) < SP(spriteNum))
    {
        if (setFloorZ)
            pSector->floorz = spriteZ;

        pData[2] = SEdir;
        pData[0] = 0;
        pData[3] = pSprite->hitag;

        for (bssize_t SPRITES_OF_SECT(pSprite->sectnum, j))
        {
            if (sprite[j].zvel == 0 && sprite[j].statnum != STAT_EFFECTOR && sprite[j].statnum != STAT_PROJECTILE)
            {
                actor[j].bpos.z = sprite[j].z;
                actor[j].floorz = pSector->floorz;
            }
        }

        A_CallSound(pSprite->sectnum, spriteNum);
    }
    else
    {
        int const zChange = ksgn(zDifference) * SP(spriteNum);

        pSector->floorz += zChange;

        for (bssize_t SPRITES_OF_SECT(pSprite->sectnum, j))
        {
            if (sprite[j].picnum == APLAYER && sprite[j].owner >= 0)
            {
                int const playerNum = P_Get(j);

                if (g_player[playerNum].ps->on_ground == 1)
                    g_player[playerNum].ps->pos.z += zChange;
            }

            if (sprite[j].zvel == 0 && sprite[j].statnum != STAT_EFFECTOR && sprite[j].statnum != STAT_PROJECTILE)
            {
                actor[j].bpos.z = sprite[j].z;
                sprite[j].z += zChange;
                actor[j].floorz = pSector->floorz;
            }
        }
    }
}

// s: SE sprite
static void MaybeTrainKillPlayer(const spritetype *pSprite, int const setOPos)
{
    for (bssize_t TRAVERSE_CONNECT(playerNum))
    {
        auto const pPlayer = g_player[playerNum].ps;

        if (sprite[pPlayer->i].extra > 0)
        {
            int16_t playerSectnum = pPlayer->cursectnum;

            updatesector(pPlayer->pos.x, pPlayer->pos.y, &playerSectnum);

            if (pPlayer->cursectnum != pSprite->sectnum && (playerSectnum == -1 || playerSectnum == pSprite->sectnum))
            {
                pPlayer->pos.xy = pSprite->xy;

                if (setOPos)
                    pPlayer->opos.xy = pPlayer->pos.xy;

                pPlayer->cursectnum = pSprite->sectnum;

                setsprite(pPlayer->i, &pSprite->xyz);
                P_QuickKill(pPlayer);
            }
        }
    }
}

static void actorGibEnemy(int findSprite, int spriteNum)
{
    actor[findSprite].htpicnum = RADIUSEXPLOSION;
    actor[findSprite].htextra  = INT16_MAX;
    actor[findSprite].htowner  = spriteNum;
}
// i: SE spritenum

static void MaybeTrainKillEnemies(int const spriteNum)
{
    int findSprite = headspritesect[sprite[OW(spriteNum)].sectnum];

    do
    {
        int const nextSprite = nextspritesect[findSprite];

        if (sprite[findSprite].extra >= 0 && sprite[findSprite].statnum == STAT_ACTOR && A_CheckEnemySprite(&sprite[findSprite]))
        {
            int16_t sectNum = sprite[findSprite].sectnum;

            updatesector(sprite[findSprite].x,sprite[findSprite].y,&sectNum);

            if (sectNum == sprite[spriteNum].sectnum || sectNum == -1)
                actorGibEnemy(findSprite, spriteNum);
        }

        findSprite = nextSprite;
    }
    while (findSprite >= 0);
}

int dukeValidateSectorEffectorPlaysSound(int num)
{
    switch (SLT(num))
    {
        case SE_11_SWINGING_DOOR:
        case SE_15_SLIDING_DOOR:
        case SE_18_INCREMENTAL_SECTOR_RISE_FALL:
        case SE_20_STRETCH_BRIDGE:
        case SE_21_DROP_FLOOR:
        case SE_31_FLOOR_RISE_FALL:
        case SE_32_CEILING_RISE_FALL:
        case SE_36_PROJ_SHOOTER:
            return 1;
    }
    return 0;
}

int dukeValidateSectorPlaysSound(int sect)
{
    switch (sector[sect].lotag)
    {
        case ST_9_SLIDING_ST_DOOR:
        case ST_16_PLATFORM_DOWN:
        case ST_17_PLATFORM_UP:
        case ST_18_ELEVATOR_DOWN:
        case ST_19_ELEVATOR_UP:
        case ST_29_TEETH_DOOR:
        case ST_20_CEILING_DOOR:
        case ST_21_FLOOR_DOOR:
        case ST_22_SPLITTING_DOOR:
        case ST_23_SWINGING_DOOR:
        case ST_25_SLIDING_DOOR:
        case ST_27_STRETCH_BRIDGE:
        case ST_28_DROP_FLOOR:
        case ST_30_ROTATE_RISE_BRIDGE:
        case ST_31_TWO_WAY_TRAIN:
            return 1;
    }
    return 0;
}

ACTOR_STATIC void G_MoveEffectors(void)   //STATNUM 3
{
    int32_t q = 0, j, k, l, m, x;
    int spriteNum = headspritestat[STAT_EFFECTOR];

#ifndef EDUKE32_STANDALONE
    if (!FURY)
    {
        for (native_t TRAVERSE_CONNECT(playerNum))
        {
            vec2_t & fric = g_player[playerNum].ps->fric;
            fric.x = fric.y = 0;
        }
    }
#endif
    while (spriteNum >= 0)
    {
        int const  nextSprite = nextspritestat[spriteNum];
        auto const pSprite    = &sprite[spriteNum];
        int32_t    playerDist;
        int        playerNum = A_FindPlayer(pSprite, &playerDist);
        auto const pPlayer   = g_player[playerNum].ps;

        if (VM_OnEvent(EVENT_MOVEEFFECTORS, spriteNum, playerNum, playerDist, 0))
        {
            spriteNum = nextSprite;
            continue;
        }

        sectortype *const pSector     = &sector[pSprite->sectnum];
        int const         spriteLotag = pSprite->lotag;
        int const         spriteHitag = pSprite->hitag;
        int32_t *const    pData       = &actor[spriteNum].t_data[0];

        switch (spriteLotag)
        {
        case SE_0_ROTATING_SECTOR:
        {
            int32_t zchange = 0;

            j = pSprite->owner;

            if ((uint16_t)sprite[j].lotag == UINT16_MAX)
                DELETE_SPRITE_AND_CONTINUE(spriteNum);

            q = pSector->extra>>3;
            l = 0;

            if (pSector->lotag == ST_30_ROTATE_RISE_BRIDGE)
            {
                q >>= 2;

                if (sprite[spriteNum].extra == 1)
                {
                    if (actor[spriteNum].tempang < 256)
                    {
                        actor[spriteNum].tempang += 4;
                        if (actor[spriteNum].tempang >= 256)
                            A_CallSound(pSprite->sectnum,spriteNum);
                        if (pSprite->clipdist) l = 1;
                        else l = -1;
                    }
                    else actor[spriteNum].tempang = 256;

                    if (pSector->floorz > pSprite->z)   //z's are touching
                    {
                        pSector->floorz -= 512;
                        zchange = -512;
                        if (pSector->floorz < pSprite->z)
                            pSector->floorz = pSprite->z;
                    }
                    else if (pSector->floorz < pSprite->z)   //z's are touching
                    {
                        pSector->floorz += 512;
                        zchange = 512;
                        if (pSector->floorz > pSprite->z)
                            pSector->floorz = pSprite->z;
                    }
                }
                else if (sprite[spriteNum].extra == 3)
                {
                    if (actor[spriteNum].tempang > 0)
                    {
                        actor[spriteNum].tempang -= 4;
                        if (actor[spriteNum].tempang <= 0)
                            A_CallSound(pSprite->sectnum,spriteNum);
                        if (pSprite->clipdist) l = -1;
                        else l = 1;
                    }
                    else actor[spriteNum].tempang = 0;

                    if (pSector->floorz > T4(spriteNum))   //z's are touching
                    {
                        pSector->floorz -= 512;
                        zchange = -512;
                        if (pSector->floorz < T4(spriteNum))
                            pSector->floorz = T4(spriteNum);
                    }
                    else if (pSector->floorz < T4(spriteNum))   //z's are touching
                    {
                        pSector->floorz += 512;
                        zchange = 512;
                        if (pSector->floorz > T4(spriteNum))
                            pSector->floorz = T4(spriteNum);
                    }
                }
            }
            else
            {
                if (actor[j].t_data[0] == 0) break;
                if (actor[j].t_data[0] == 2) DELETE_SPRITE_AND_CONTINUE(spriteNum);

                l = (sprite[j].ang > 1024) ? -1 : 1;

                if (pData[3] == 0)
                    pData[3] = ldist(pSprite,&sprite[j]);
                pSprite->xvel = pData[3];
                pSprite->x = sprite[j].x;
                pSprite->y = sprite[j].y;
            }

            pSprite->ang += (l*q);
            pData[2] += (l*q);

            if (l && (pSector->floorstat&64))
            {
                for (TRAVERSE_CONNECT(playerNum))
                {
                    auto const pPlayer = g_player[playerNum].ps;

                    if (pPlayer->cursectnum == pSprite->sectnum && pPlayer->on_ground == 1)
                    {
                        g_player[playerNum].smoothcamera = true;
                        pPlayer->q16ang += fix16_from_int(l*q);
                        pPlayer->q16ang &= 0x7FFFFFF;

                        pPlayer->pos.z += zchange;

                        vec2_t r;
                        rotatepoint(sprite[j].xy,pPlayer->pos.xy,(q*l),&r);

                        pPlayer->bobpos.x += r.x-pPlayer->pos.x;
                        pPlayer->bobpos.y += r.y-pPlayer->pos.y;

                        pPlayer->pos.xy = r;

                        if (sprite[pPlayer->i].extra <= 0)
                            sprite[pPlayer->i].xy = r;
                    }
                }

                for (bssize_t SPRITES_OF_SECT(pSprite->sectnum, p))
                {
                    // KEEPINSYNC1
                    if (sprite[p].statnum != STAT_EFFECTOR && sprite[p].statnum != STAT_PROJECTILE)
                        if (sprite[p].picnum != LASERLINE)
                        {
                            if (sprite[p].picnum == APLAYER && sprite[p].owner >= 0)
                                continue;

                            sprite[p].ang += (l*q);
                            sprite[p].ang &= 2047;

                            sprite[p].z += zchange;

                            // interpolation fix
                            actor[p].bpos.xy = sprite[p].xy;

                            if (move_rotfixed_sprite(p, j, pData[2]))
                                rotatepoint(sprite[j].xy, sprite[p].xy, (q * l), &sprite[p].xy);
                        }
                }

            }
            else if (l==0 && (pSector->floorstat&64))
            {
                // fix for jittering of sprites in halted rotating sectors
                for (bssize_t SPRITES_OF_SECT(pSprite->sectnum, p))
                {
                    // KEEPINSYNC1
                    if (sprite[p].statnum != STAT_EFFECTOR && sprite[p].statnum != STAT_PROJECTILE)
                        if (sprite[p].picnum != LASERLINE)
                        {
                            if (sprite[p].picnum == APLAYER && sprite[p].owner >= 0)
                                continue;

                            actor[p].bpos.xy = sprite[p].xy;
                        }
                }
            }

            A_MoveSector(spriteNum);
        }
        break;

        case SE_1_PIVOT: //Nothing for now used as the pivot
            if (pSprite->owner == -1) //Init
            {
                pSprite->owner = spriteNum;

                for (SPRITES_OF(STAT_EFFECTOR, j))
                {
                    if (sprite[j].lotag == SE_19_EXPLOSION_LOWERS_CEILING && sprite[j].hitag == spriteHitag)
                    {
                        pData[0] = 0;
                        break;
                    }
                }
            }
            break;

        case SE_6_SUBWAY:
            k = pSector->extra;

            if (pData[4] > 0)
            {
                pData[4]--;
                if (pData[4] >= (k-(k>>3)))
                    pSprite->xvel -= (k>>5);
                if (pData[4] > ((k>>1)-1) && pData[4] < (k-(k>>3)))
                    pSprite->xvel = 0;
                if (pData[4] < (k>>1))
                    pSprite->xvel += (k>>5);
                if (pData[4] < ((k>>1)-(k>>3)))
                {
                    pData[4] = 0;
                    pSprite->xvel = k;
                }
            }
            else pSprite->xvel = k;

            for (SPRITES_OF(STAT_EFFECTOR, j))
            {
                if (sprite[j].lotag == SE_14_SUBWAY_CAR && spriteHitag == sprite[j].hitag && actor[j].t_data[0] == pData[0])
                {
                    sprite[j].xvel = pSprite->xvel;
                    //                        if( t[4] == 1 )
                    {
                        if (actor[j].t_data[5] == 0)
                            actor[j].t_data[5] = dist(&sprite[j],pSprite);
                        x = ksgn(dist(&sprite[j],pSprite)-actor[j].t_data[5]);
                        if (sprite[j].extra)
                            x = -x;
                        pSprite->xvel += x;
                    }
                    actor[j].t_data[4] = pData[4];
                }
            }
            x = 0;  // XXX: This assignment is dead?
            fallthrough__;

        case SE_14_SUBWAY_CAR:
            if (pSprite->owner==-1)
                pSprite->owner = A_FindLocator((int16_t)pData[3],(int16_t)pData[0]);

            if (pSprite->owner == -1)
            {
                // debugging subway cars (mapping-wise) is freakin annoying
                // let's at least have a helpful message...
                Bsprintf(tempbuf,"Could not find any locators in sector %d"
                         " for SE# 6 or 14 with hitag %d.\n", (int)pData[0], (int)pData[3]);
                G_GameExit(tempbuf);
            }

            j = ldist(&sprite[pSprite->owner],pSprite);

            if (j < 1024L)
            {
                if (spriteLotag==SE_6_SUBWAY)
                    if (sprite[pSprite->owner].hitag&1)
                        pData[4]=pSector->extra; //Slow it down
                pData[3]++;
                pSprite->owner = A_FindLocator(pData[3],pData[0]);
                if (pSprite->owner==-1)
                {
                    pData[3]=0;
                    pSprite->owner = A_FindLocator(0,pData[0]);
                }
            }

            if (pSprite->xvel)
            {
#ifdef YAX_ENABLE
                int32_t firstrun = 1;
#endif
                x = getangle(sprite[pSprite->owner].x-pSprite->x,sprite[pSprite->owner].y-pSprite->y);
                q = G_GetAngleDelta(pSprite->ang,x)>>3;

                pData[2] += q;
                pSprite->ang += q;

                if (pSprite->xvel == pSector->extra)
                {
                    if ((pSector->floorstat&1) == 0 && (pSector->ceilingstat&1) == 0)
                    {
                        if (!S_CheckSoundPlaying(actor[spriteNum].lastv.x))
                            A_PlaySound(actor[spriteNum].lastv.x,spriteNum);
                    }
                    else if (ud.monsters_off == 0 && pSector->floorpal == 0 && (pSector->floorstat&1) && rnd(8))
                    {
                        if (playerDist < 20480)
                        {
                            j = pSprite->ang;
                            pSprite->ang = getangle(pSprite->x-g_player[playerNum].ps->pos.x,pSprite->y-g_player[playerNum].ps->pos.y);
                            A_Shoot(spriteNum,RPG);
                            pSprite->ang = j;
                        }
                    }
                }

                if (pSprite->xvel <= 64 && (pSector->floorstat&1) == 0 && (pSector->ceilingstat&1) == 0)
                    S_StopEnvSound(actor[spriteNum].lastv.x,spriteNum);

                if ((pSector->floorz-pSector->ceilingz) < (108<<8))
                {
                    if (ud.noclip == 0 && pSprite->xvel >= 192)
                        MaybeTrainKillPlayer(pSprite, 0);
                }

                m = (pSprite->xvel*sintable[(pSprite->ang+512)&2047])>>14;
                x = (pSprite->xvel*sintable[pSprite->ang&2047])>>14;

                if (sector[pSprite->sectnum].lotag != ST_2_UNDERWATER)
                {
                    // Move player spawns with sector.
                    for (int spawnNum = 0; spawnNum < g_playerSpawnCnt; spawnNum++)
                    {
                        if (g_playerSpawnPoints[spawnNum].sect == pSprite->sectnum)
                        {
                            g_playerSpawnPoints[spawnNum].x += m;
                            g_playerSpawnPoints[spawnNum].y += x;
                        }
                    }

                    for (TRAVERSE_CONNECT(playerNum))
                    {
                        auto const pPlayer = g_player[playerNum].ps;

                        // might happen when squished into void space
                        if (pPlayer->cursectnum < 0)
                            break;

                        if (pSprite->sectnum == pPlayer->cursectnum
#ifdef YAX_ENABLE
                            || (pData[9] >= 0 && pData[9] == pPlayer->cursectnum)
#endif
                        )
                        {
                            rotatepoint(pSprite->xy, pPlayer->pos.xy, q, &pPlayer->pos.xy);

                            pPlayer->pos.x += m;
                            pPlayer->pos.y += x;

                            pPlayer->bobpos.x += m;
                            pPlayer->bobpos.y += x;

                            g_player[playerNum].smoothcamera = true;

                            pPlayer->q16ang += fix16_from_int(q);
                            pPlayer->q16ang &= 0x7FFFFFF;

                            if (sprite[pPlayer->i].extra <= 0)
                                sprite[pPlayer->i].xy = pPlayer->pos.xy;
                        }
                    }

                    // NOTE: special loop handling
                    j = headspritesect[pSprite->sectnum];
                    while (j >= 0)
                    {
                        if ((sprite[j].picnum != SECTOREFFECTOR || (sprite[j].lotag == SE_49_POINT_LIGHT || sprite[j].lotag == SE_50_SPOT_LIGHT))
                            && sprite[j].picnum != LOCATORS)
                        {
                            if (move_rotfixed_sprite(j, pSprite - sprite, pData[2]))
                                rotatepoint(pSprite->xy, sprite[j].xy, q, &sprite[j].xy);

                            sprite[j].x += m;
                            sprite[j].y += x;

                            sprite[j].ang += q;
                        }
                        j = nextspritesect[j];
#ifdef YAX_ENABLE
                        if (j < 0)
                        {
                            if (pData[9] >= 0 && firstrun)
                            {
                                firstrun = 0;
                                j = headspritesect[pData[9]];
                            }
                        }
#endif
                    }
                }

                A_MoveSector(spriteNum);
                setsprite(spriteNum,&pSprite->xyz);

                if ((pSector->floorz-pSector->ceilingz) < (108<<8))
                {
                    if (ud.noclip == 0 && pSprite->xvel >= 192)
                        MaybeTrainKillPlayer(pSprite, 1);

                    MaybeTrainKillEnemies(spriteNum);
                }
            }

            break;

        case SE_30_TWO_WAY_TRAIN:
            if (pSprite->owner == -1)
            {
                pData[3] = !pData[3];
                pSprite->owner = A_FindLocator(pData[3],pData[0]);
            }
            else
            {

                if (pData[4] == 1) // Starting to go
                {
                    if (ldist(&sprite[pSprite->owner],pSprite) < (2048-128))
                        pData[4] = 2;
                    else
                    {
                        if (pSprite->xvel == 0)
                            G_OperateActivators(pSprite->hitag+(!pData[3]),-1);
                        if (pSprite->xvel < 256)
                            pSprite->xvel += 16;
                    }
                }
                if (pData[4] == 2)
                {
                    l = FindDistance2D(sprite[pSprite->owner].x-pSprite->x,sprite[pSprite->owner].y-pSprite->y);

                    if (l <= 128)
                        pSprite->xvel = 0;

                    if (pSprite->xvel > 0)
                        pSprite->xvel -= 16;
                    else
                    {
                        pSprite->xvel = 0;
                        G_OperateActivators(pSprite->hitag+(int16_t)pData[3],-1);
                        pSprite->owner = -1;
                        pSprite->ang += 1024;
                        pData[4] = 0;
                        G_OperateForceFields(spriteNum,pSprite->hitag);

                        for (SPRITES_OF_SECT(pSprite->sectnum, j))
                        {
                            if (sprite[j].picnum != SECTOREFFECTOR && sprite[j].picnum != LOCATORS)
                                actor[j].bpos.xy = sprite[j].xy;
                        }

                    }
                }
            }

            if (pSprite->xvel)
            {
                l = (pSprite->xvel*sintable[(pSprite->ang+512)&2047])>>14;
                x = (pSprite->xvel*sintable[pSprite->ang&2047])>>14;

                if ((pSector->floorz-pSector->ceilingz) < (108<<8))
                    if (ud.noclip == 0)
                        MaybeTrainKillPlayer(pSprite, 0);

                // Move player spawns with sector.
                for (int spawnNum = 0; spawnNum < g_playerSpawnCnt; spawnNum++)
                {
                    if (g_playerSpawnPoints[spawnNum].sect == pSprite->sectnum)
                    {
                        g_playerSpawnPoints[spawnNum].x += l;
                        g_playerSpawnPoints[spawnNum].y += x;
                    }
                }

                for (int TRAVERSE_CONNECT(playerNum))
                {
                    auto const pPlayer = g_player[playerNum].ps;

                    if (pPlayer->cursectnum == pSprite->sectnum)
                    {
                        pPlayer->pos.x += l;
                        pPlayer->pos.y += x;

                        if (g_netServer || numplayers > 1)
                            pPlayer->opos.xy = pPlayer->pos.xy;

                        pPlayer->bobpos.x += l;
                        pPlayer->bobpos.y += x;
                    }
                }

                for (SPRITES_OF_SECT(pSprite->sectnum, j))
                {
                    // TODO: replace some checks for SE 49/50 with statnum LIGHT instead?
                    if ((sprite[j].picnum != SECTOREFFECTOR || sprite[j].lotag==SE_49_POINT_LIGHT || sprite[j].lotag==SE_50_SPOT_LIGHT)
                            && sprite[j].picnum != LOCATORS)
                    {
                        if (numplayers < 2 && !g_netServer)
                            actor[j].bpos.xy = sprite[j].xy;

                        sprite[j].x += l;
                        sprite[j].y += x;

                        if (g_netServer || numplayers > 1)
                            actor[j].bpos.xy = sprite[j].xy;
                    }
                }

                A_MoveSector(spriteNum);
                setsprite(spriteNum,&pSprite->xyz);

                if (pSector->floorz-pSector->ceilingz < (108<<8))
                {
                    if (ud.noclip == 0)
                        MaybeTrainKillPlayer(pSprite, 1);

                    MaybeTrainKillEnemies(spriteNum);
                }
            }

            break;


        case SE_2_EARTHQUAKE://Quakes
            if (pData[4] > 0 && pData[0] == 0)
            {
                if (pData[4] < spriteHitag)
                    pData[4]++;
                else pData[0] = 1;
            }

            if (pData[0] > 0)
            {
                pData[0]++;

                pSprite->xvel = 3;

                if (pData[0] > 96)
                {
                    pData[0] = -1; //Stop the quake
                    pData[4] = -1;
                    DELETE_SPRITE_AND_CONTINUE(spriteNum);
                }
                else
                {
                    if ((pData[0]&31) ==  8)
                    {
                        g_earthquakeTime = 48;
                        A_PlaySound(EARTHQUAKE,g_player[screenpeek].ps->i);
                    }

                    pSector->floorheinum = (klabs(pSector->floorheinum - pData[5]) < 8)
                                           ? pData[5]
                                           : pSector->floorheinum + (ksgn(pData[5] - pSector->floorheinum) << 4);
                }

                vec2_t const vect = { (pSprite->xvel * sintable[(pSprite->ang + 512) & 2047]) >> 14,
                                      (pSprite->xvel * sintable[pSprite->ang & 2047]) >> 14 };

                for (TRAVERSE_CONNECT(playerNum))
                {
                    auto const pPlayer = g_player[playerNum].ps;

                    if (pPlayer->cursectnum == pSprite->sectnum && pPlayer->on_ground)
                    {
                        pPlayer->pos.x += vect.x;
                        pPlayer->pos.y += vect.y;

                        pPlayer->bobpos.x += vect.x;
                        pPlayer->bobpos.y += vect.y;
                    }
                }

                for (bssize_t nextSprite, SPRITES_OF_SECT_SAFE(pSprite->sectnum, sectSprite, nextSprite))
                {
                    if (sprite[sectSprite].picnum != SECTOREFFECTOR)
                    {
                        sprite[sectSprite].x+=vect.x;
                        sprite[sectSprite].y+=vect.y;
                        setsprite(sectSprite,&sprite[sectSprite].xyz);
                    }
                }

                A_MoveSector(spriteNum);
                setsprite(spriteNum,&pSprite->xyz);
            }
            break;

            //Flashing sector lights after reactor EXPLOSION2

        case SE_3_RANDOM_LIGHTS_AFTER_SHOT_OUT:
        {
            if (pData[4] == 0) break;

            //    if(t[5] > 0) { t[5]--; break; }

            if ((tabledivide32_noinline(g_globalRandom, spriteHitag+1)&31) < 4 && !pData[2])
            {
                //       t[5] = 4+(g_globalRandom&7);
                pSector->ceilingpal = pSprite->owner >> 8;
                pSector->floorpal   = pSprite->owner & 0xff;
                pData[0]            = pSprite->shade + (g_globalRandom & 15);
            }
            else
            {
                //       t[5] = 4+(g_globalRandom&3);
                pSector->ceilingpal = pSprite->pal;
                pSector->floorpal   = pSprite->pal;
                pData[0]            = pData[3];
            }

            pSector->ceilingshade = pData[0];
            pSector->floorshade   = pData[0];

            walltype *pWall = &wall[pSector->wallptr];

            for (x=pSector->wallnum; x > 0; x--,pWall++)
            {
                if (pWall->hitag != 1)
                {
                    pWall->shade = pData[0];

                    if ((pWall->cstat & 2) && pWall->nextwall >= 0)
                        wall[pWall->nextwall].shade = pWall->shade;
                }
            }

            break;
        }

        case SE_4_RANDOM_LIGHTS:
        {
            // See A_Spawn():
            //  s->owner: original ((ceilingpal<<8) | floorpal)
            //  t[2]: original floor shade
            //  t[3]: max wall shade
            int lightFlag;

            if ((tabledivide32_noinline(g_globalRandom, spriteHitag+1)&31) < 4)
            {
                pData[1]            = pSprite->shade + (g_globalRandom & 15);  // Got really bright
                pData[0]            = pSprite->shade + (g_globalRandom & 15);
                pSector->ceilingpal = pSprite->owner >> 8;
                pSector->floorpal   = pSprite->owner & 0xff;
                lightFlag           = 1;
            }
            else
            {
                pData[1] = pData[2];
                pData[0] = pData[3];

                pSector->ceilingpal = pSprite->pal;
                pSector->floorpal   = pSprite->pal;

                lightFlag = 0;
            }

            pSector->floorshade = pData[1];
            pSector->ceilingshade = pData[1];

            walltype *pWall = &wall[pSector->wallptr];

            for (x=pSector->wallnum; x > 0; x--,pWall++)
            {
                if (lightFlag) pWall->pal = (pSprite->owner&0xff);
                else pWall->pal = pSprite->pal;

                if (pWall->hitag != 1)
                {
                    pWall->shade = pData[0];
                    if ((pWall->cstat&2) && pWall->nextwall >= 0)
                        wall[pWall->nextwall].shade = pWall->shade;
                }
            }

            for (bssize_t SPRITES_OF_SECT(SECT(spriteNum), sectSprite))
            {
                if (sprite[sectSprite].cstat&16 && !A_CheckSpriteFlags(sectSprite,SFLAG_NOSHADE))
                    sprite[sectSprite].shade = (pSector->ceilingstat & 1) ? pSector->ceilingshade : pSector->floorshade;
            }

            if (pData[4])
                DELETE_SPRITE_AND_CONTINUE(spriteNum);

            break;
        }

            //BOSS
        case SE_5:
        {
            if (playerDist < 8192)
            {
                int const saveAng = pSprite->ang;
                pSprite->ang      = getangle(pSprite->x - pPlayer->pos.x, pSprite->y - pPlayer->pos.y);
                A_Shoot(spriteNum, FIRELASER);
                pSprite->ang      = saveAng;
            }

            if (pSprite->owner==-1) //Start search
            {
                pData[4]               = 0;
                int closestLocatorDist = INT32_MAX;
                int closestLocator     = pSprite->owner;

                //Find the shortest dist
                do
                {
                    pSprite->owner = A_FindLocator((int16_t)pData[4], -1);  // t[0] hold sectnum

                    if (pSprite->owner == -1)
                        break;

                    int const locatorDist = ldist(&sprite[pPlayer->i],&sprite[pSprite->owner]);

                    if (closestLocatorDist > locatorDist)
                    {
                        closestLocator     = pSprite->owner;
                        closestLocatorDist = locatorDist;
                    }

                    pData[4]++;
                }
                while (1);

                pSprite->owner = closestLocator;
                pSprite->zvel  = ksgn(sprite[closestLocator].z - pSprite->z) << 4;
            }

            if (ldist(&sprite[pSprite->owner],pSprite) < 1024)
            {
                pSprite->owner = -1;
                goto next_sprite;
            }
            else pSprite->xvel=256;

            int const angInc = G_GetAngleDelta(pSprite->ang, getangle(sprite[pSprite->owner].x-pSprite->x,
                                                                      sprite[pSprite->owner].y-pSprite->y))>>3;
            pSprite->ang += angInc;

            if (rnd(32))
            {
                pData[2] += angInc;
                pSector->ceilingshade = 127;
            }
            else
            {
                pData[2] += G_GetAngleDelta(pData[2] + 512, getangle(pPlayer->pos.x - pSprite->x, pPlayer->pos.y - pSprite->y)) >> 2;
                pSector->ceilingshade = 0;
            }

            if (A_IncurDamage(spriteNum) >= 0)
            {
                if (++pData[3] == 5)
                {
                    pSprite->zvel += 1024;
                    P_DoQuote(QUOTE_WASTED, g_player[myconnectindex].ps);
                }
            }

            pSprite->z                += pSprite->zvel;
            pSector->ceilingz         += pSprite->zvel;
            sector[pData[0]].ceilingz += pSprite->zvel;

            A_MoveSector(spriteNum);
            setsprite(spriteNum, &pSprite->xyz);
            break;
        }

        case SE_8_UP_OPEN_DOOR_LIGHTS:
        case SE_9_DOWN_OPEN_DOOR_LIGHTS:
        {

            // work only if its moving

            int animGoal = -1;

            if (actor[spriteNum].t_data[4])
            {
                if (++actor[spriteNum].t_data[4] > 8)
                    DELETE_SPRITE_AND_CONTINUE(spriteNum);

                animGoal = 1;
            }
            else animGoal = GetAnimationGoal(&pSector->ceilingz);

            if (animGoal >= 0)
            {
                int shadeInc = ((pSector->lotag & 0x8000u) || actor[spriteNum].t_data[4]) ? -pData[3] : pData[3];

                if (spriteLotag == SE_9_DOWN_OPEN_DOOR_LIGHTS)
                    shadeInc = -shadeInc;

                for (bssize_t SPRITES_OF(STAT_EFFECTOR, sectorEffector))
                {
                    if (sprite[sectorEffector].lotag == spriteLotag && sprite[sectorEffector].hitag == spriteHitag)
                    {
                        int const sectNum = sprite[sectorEffector].sectnum;
                        int const spriteShade = sprite[sectorEffector].shade;

                        walltype *pWall = &wall[sector[sectNum].wallptr];

                        for (int l=sector[sectNum].wallnum; l>0; l--, pWall++)
                        {
                            if (pWall->hitag == 1)
                                continue;

                            pWall->shade += shadeInc;

                            if (pWall->shade < spriteShade)
                                pWall->shade = spriteShade;
                            else if (pWall->shade > actor[sectorEffector].t_data[2])
                                pWall->shade = actor[sectorEffector].t_data[2];

                            if (pWall->nextwall >= 0 && wall[pWall->nextwall].hitag != 1)
                                wall[pWall->nextwall].shade = pWall->shade;
                        }

                        sector[sectNum].floorshade   += shadeInc;
                        sector[sectNum].ceilingshade += shadeInc;

                        if (sector[sectNum].floorshade < spriteShade)
                            sector[sectNum].floorshade = spriteShade;
                        else if (sector[sectNum].floorshade > actor[sectorEffector].t_data[0])
                            sector[sectNum].floorshade = actor[sectorEffector].t_data[0];

                        if (sector[sectNum].ceilingshade < spriteShade)
                            sector[sectNum].ceilingshade = spriteShade;
                        else if (sector[sectNum].ceilingshade > actor[sectorEffector].t_data[1])
                            sector[sectNum].ceilingshade = actor[sectorEffector].t_data[1];
                    }
                }
            }
            break;
        }

        case SE_10_DOOR_AUTO_CLOSE:
            // pSector->lotag == (int16_t)32791u appears to be checking for a ST_23_SWINGING_DOOR in motion (lotag & 0x8000u)
            if ((pSector->lotag&0xff) == ST_27_STRETCH_BRIDGE || (pSector->floorz > pSector->ceilingz && (pSector->lotag&0xff) != ST_23_SWINGING_DOOR) || pSector->lotag == (int16_t)32791u)
            {
                j = 1;

                if ((pSector->lotag&0xff) != ST_27_STRETCH_BRIDGE)
                    for (bssize_t TRAVERSE_CONNECT(playerNum))
                        if (pSector->lotag != ST_30_ROTATE_RISE_BRIDGE && pSector->lotag != ST_31_TWO_WAY_TRAIN && pSector->lotag != 0
                            && pSprite->sectnum == pPlayer->cursectnum)
                            j = 0;

                if (j == 1)
                {
                    if (pData[0] > spriteHitag)
                        switch (sector[pSprite->sectnum].lotag)
                        {
                        case ST_20_CEILING_DOOR:
                        case ST_21_FLOOR_DOOR:
                        case ST_22_SPLITTING_DOOR:
                        case ST_26_SPLITTING_ST_DOOR:
                            if (GetAnimationGoal(&sector[pSprite->sectnum].ceilingz) >= 0)
                                break;
                            fallthrough__;
                        default:
                            G_ActivateBySector(pSprite->sectnum,spriteNum);
                            pData[0] = 0;
                            break;
                        }
                    else pData[0]++;
                }
            }
            else pData[0]=0;
            break;

        case SE_11_SWINGING_DOOR:
            if (pData[5] > 0)
            {
                pData[5]--;
                break;
            }

            if (pData[4])
            {
                auto reverseSwingDoor = [&](void)
                {
                    uint16_t const tag = pSector->lotag & 0x8000u;

                    for (auto SPRITES_OF(STAT_EFFECTOR, i))
                    {
                        if (tag == (sector[SECT(i)].lotag & 0x8000u) && SLT(i) == SE_11_SWINGING_DOOR && pSprite->hitag == SHT(i))
                        {
                            actor[i].t_data[5] = 2; // delay
                            actor[i].t_data[2] -= l;
                            actor[i].t_data[4] -= l;
                            A_MoveSector(i);

                            actor[i].t_data[3] = -actor[i].t_data[3];
                            if (actor[i].t_data[4] <= 0)
                                actor[i].t_data[4] += 512;
                            else
                                actor[i].t_data[4] -= 512;

                            if (sector[SECT(i)].lotag & 0x8000u) sector[SECT(i)].lotag &= 0x7fff;
                            else sector[SECT(i)].lotag |= 0x8000u;
                        }
                    }

                    A_CallSound(pSprite->sectnum, spriteNum);
                };

                auto maybeHoldDoorOpen = [&](void)
                {
                    if ((pSector->lotag & (16384u | 32768u)) == 0)
                    {
                        int j;

                        for (SPRITES_OF_SECT(pSprite->sectnum, j))
                            if (sprite[j].picnum == ACTIVATOR)
                                break;

                        if (j == -1)
                        {
                            pData[2] -= l;
                            pData[4] -= l;
                            pData[5] = 2;

                            A_MoveSector(spriteNum);

                            return true;
                        }
                    }

                    return false;
                };

                int const endWall = pSector->wallptr+pSector->wallnum;

                l = (SP(spriteNum) >> 3) * pData[3];
                pData[2] += l;
                pData[4] += l;

                A_MoveSector(spriteNum);

                auto const lengths = (int32_t *)Balloca(pSector->wallnum * sizeof(int32_t));

                for (int w = pSector->wallptr; w < endWall; w++)
                    lengths[w - pSector->wallptr] = wallength(w);

                for (auto SPRITES_OF(STAT_ACTOR, findSprite))
                {
                    auto const foundSprite = &sprite[findSprite];

                    int w = pSector->wallptr;
                    for (; w < endWall; w++)
                    {
                        if (ldist(foundSprite, &wall[w]) < lengths[w - pSector->wallptr] + foundSprite->clipdist)
                            break;
                    }

                    if (w == endWall)
                        continue;

                    int32_t floorZ, ceilZ;
                    getcorrectzsofslope(pSprite->sectnum, foundSprite->x, foundSprite->y, &ceilZ, &floorZ);

                    if ((foundSprite->z > floorZ || foundSprite->z - ((foundSprite->yrepeat * tilesiz[foundSprite->picnum].y) << 2) < ceilZ)
                        && A_CheckEnemySprite(foundSprite))
                    {
                        auto const clipdist = A_GetClipdist(findSprite);

                        for (int w = pSector->wallptr; w < endWall; w++)
                        {
                            if (clipinsidebox(foundSprite->xy, w, clipdist))
                            {
                                if (foundSprite->extra <= 0)
                                {
                                    if (clipinsidebox(foundSprite->xy, w, clipdist >> 1))
                                        actorGibEnemy(findSprite, spriteNum);
                                    break;
                                }

                                if (maybeHoldDoorOpen())
                                    goto end;

                                int16_t    sectnum    = foundSprite->sectnum;
                                int const  pushResult = pushmove(&foundSprite->xyz, &sectnum, clipdist - 1, (4L << 8), (4L << 8), CLIPMASK0);
                                bool const squish     = sectnum == pSprite->sectnum || sectnum == -1 || pushResult < 0;

                                if (sectnum != -1 && sectnum != foundSprite->sectnum)
                                    changespritesect(findSprite, sectnum);

                                if (squish)
                                    actorGibEnemy(findSprite, spriteNum);

                                break;
                            }
                        }
                    }
                }

                for (auto TRAVERSE_CONNECT(plr))
                {
                    auto const foundPlayer       = g_player[plr].ps;
                    auto const foundPlayerSprite = &sprite[foundPlayer->i];

                    int32_t floorZ, ceilZ;
                    getcorrectzsofslope(pSprite->sectnum, foundPlayer->pos.x, foundPlayer->pos.y, &ceilZ, &floorZ);

                    if ((foundPlayerSprite->z > floorZ || foundPlayer->pos.z < ceilZ) && foundPlayerSprite->extra > 0)
                    {
                        for (int w = pSector->wallptr; w < endWall; w++)
                        {
                            if (clipinsidebox(foundPlayer->pos.xy, w, foundPlayer->clipdist - 1))
                            {
                                if (!maybeHoldDoorOpen())
                                {
                                    if (pushmove(&foundPlayer->pos, &foundPlayer->cursectnum, foundPlayer->clipdist - 1, (4L << 8), (4L << 8), CLIPMASK0) < 0)
                                        reverseSwingDoor();
                                }
                                goto end;
                            }
                        }
                    }
                }
            end:
                if (pData[4] <= -511 || pData[4] >= 512)
                {
                    pData[4] = 0;
                    pData[2] &= 0xffffff00;
                    A_MoveSector(spriteNum);
                }
            }
            break;

        case SE_12_LIGHT_SWITCH:
            if (pData[0] == 3 || pData[3] == 1)   //Lights going off
            {
                pSector->floorpal   = 0;
                pSector->ceilingpal = 0;

                walltype *pWall = &wall[pSector->wallptr];

                for (j = pSector->wallnum; j > 0; j--, pWall++)
                {
                    if (pWall->hitag != 1)
                    {
                        pWall->shade = pData[1];
                        pWall->pal   = 0;
                    }
                }

                pSector->floorshade   = pData[1];
                pSector->ceilingshade = pData[2];
                pData[0]              = 0;

                for (SPRITES_OF_SECT(SECT(spriteNum), j))
                {
                    if ((sprite[j].cstat & 16) && !A_CheckSpriteFlags(j, SFLAG_NOSHADE))
                        sprite[j].shade = (pSector->ceilingstat & 1) ? pSector->ceilingshade : pSector->floorshade;
                }

                if (pData[3] == 1)
                    DELETE_SPRITE_AND_CONTINUE(spriteNum);
            }

            if (pData[0] == 1)   //Lights flickering on
            {
                if (pSector->floorshade > pSprite->shade)
                {
                    pSector->floorpal   = pSprite->pal;
                    pSector->ceilingpal = pSprite->pal;

                    pSector->floorshade   -= 2;
                    pSector->ceilingshade -= 2;

                    walltype *pWall = &wall[pSector->wallptr];
                    for (j = pSector->wallnum; j > 0; j--, pWall++)
                    {
                        if (pWall->hitag != 1)
                        {
                            pWall->pal = pSprite->pal;
                            pWall->shade -= 2;
                        }
                    }
                }
                else pData[0] = 2;

                for (SPRITES_OF_SECT(SECT(spriteNum), j))
                {
                    if ((sprite[j].cstat & 16) && !A_CheckSpriteFlags(j, SFLAG_NOSHADE))
                        sprite[j].shade = (pSector->ceilingstat & 1) ? pSector->ceilingshade : pSector->floorshade;
                }
            }
            break;


        case SE_13_EXPLOSIVE:
            if (pData[2])
            {
                // t[0]: ceiling z
                // t[1]: floor z
                // s->owner: 1 if affect ceiling, 0 if affect floor
                // t[3]: 1 if ceiling was parallaxed at premap, 0 else

                j = (SP(spriteNum)<<5)|1;

                if (pSprite->ang == 512)
                {
                    if (pSprite->owner)
                    {
                        pSector->ceilingz = (klabs(pData[0] - pSector->ceilingz) >= j)
                                            ? pSector->ceilingz + ksgn(pData[0] - pSector->ceilingz) * j
                                            : pData[0];
                    }
                    else
                    {
                        pSector->floorz = (klabs(pData[1] - pSector->floorz) >= j)
                                          ? pSector->floorz + ksgn(pData[1] - pSector->floorz) * j
                                          : pData[1];
                    }
                }
                else
                {
                    pSector->floorz = (klabs(pData[1] - pSector->floorz) >= j)
                                      ? pSector->floorz + ksgn(pData[1] - pSector->floorz) * j
                                      : pData[1];

                    pSector->ceilingz = (klabs(pData[0] - pSector->ceilingz) >= j)
                                      ? pSector->ceilingz + ksgn(pData[0] - pSector->ceilingz) * j
                                      : pData[0];
                }
#ifdef YAX_ENABLE
                if (pSprite->ang == 512)
                {
                    int16_t cf=!pSprite->owner, bn=yax_getbunch(pSector-sector, cf);
                    int32_t jj, daz=SECTORFLD(pSector-sector,z, cf);

                    if (bn >= 0)
                    {
                        for (SECTORS_OF_BUNCH(bn, cf, jj))
                        {
                            SECTORFLD(jj,z, cf) = daz;
                            SECTORFLD(jj,stat, cf) &= ~(128+256 + 512+2048);
                        }
                        for (SECTORS_OF_BUNCH(bn, !cf, jj))
                        {
                            SECTORFLD(jj,z, !cf) = daz;
                            SECTORFLD(jj,stat, !cf) &= ~(128+256 + 512+2048);
                        }
                    }
                }
#endif
                if (pData[3] == 1)
                {
                    //Change the shades

                    pData[3]++;
                    pSector->ceilingstat ^= 1;

                    if (pSprite->ang == 512)
                    {
                        walltype *pWall = &wall[pSector->wallptr];

                        for (j = pSector->wallnum; j > 0; j--, pWall++)
                            pWall->shade = pSprite->shade;

                        pSector->floorshade = pSprite->shade;

                        if (g_player[0].ps->parallax_sectnum >= 0)
                        {
                            pSector->ceilingpicnum = sector[g_player[0].ps->parallax_sectnum].ceilingpicnum;
                            pSector->ceilingshade  = sector[g_player[0].ps->parallax_sectnum].ceilingshade;
                        }
                    }
                }

                for (int SPRITES_OF_SECT(pSprite->sectnum, p))
                    if (sprite[p].statnum >= STAT_DEFAULT && sprite[p].statnum <= STAT_ZOMBIEACTOR)
                        A_GetZLimits(p);

                if (++pData[2] > 256)
                    DELETE_SPRITE_AND_CONTINUE(spriteNum);
            }

#ifndef EDUKE32_STANDALONE
            if (!FURY && pData[2] == 4 && pSprite->ang != 512)
                for (x=0; x<7; x++) RANDOMSCRAP(pSprite, spriteNum);
#endif
            break;


        case SE_15_SLIDING_DOOR:

            if (pData[4])
            {
                pSprite->xvel = 16;

                if (pData[4] == 1) //Opening
                {
                    if (pData[3] >= (SP(spriteNum)>>3))
                    {
                        pData[4] = 0; //Turn off the sliders
                        A_CallSound(pSprite->sectnum,spriteNum);
                        break;
                    }
                    pData[3]++;
                }
                else if (pData[4] == 2)
                {
                    if (pData[3]<1)
                    {
                        pData[4] = 0;
                        A_CallSound(pSprite->sectnum,spriteNum);
                        break;
                    }
                    pData[3]--;
                }

                A_MoveSector(spriteNum);
                setsprite(spriteNum,&pSprite->xyz);
            }
            break;

        case SE_16_REACTOR: //Reactor

            pData[2]+=32;

            if (pSector->floorz < pSector->ceilingz)
                pSprite->shade = 0;
            else if (pSector->ceilingz < pData[3])
            {
                //The following code check to see if
                //there is any other sprites in the sector.
                //If there isn't, then kill this sectoreffector
                //itself.....

                for (SPRITES_OF_SECT(pSprite->sectnum, j))
                {
                    if (sprite[j].picnum == REACTOR || sprite[j].picnum == REACTOR2)
                        break;
                }

                if (j == -1)
                    DELETE_SPRITE_AND_CONTINUE(spriteNum);

                pSprite->shade = 1;
            }

            pSector->ceilingz = (pSprite->shade)
                                ? pSector->ceilingz + 1024
                                : pSector->ceilingz - 512;

            A_MoveSector(spriteNum);
            setsprite(spriteNum,&pSprite->xyz);

            break;

        case SE_17_WARP_ELEVATOR:
        {
            q = pData[0]*(SP(spriteNum)<<2);

            pSector->ceilingz += q;
            pSector->floorz += q;

            for (SPRITES_OF_SECT(pSprite->sectnum, j))
            {
                if (sprite[j].statnum == STAT_PLAYER && sprite[j].owner >= 0)
                {
                    int const  warpPlayer = P_Get(j);
                    auto const pPlayer    = g_player[warpPlayer].ps;

                    if (numplayers < 2 && !g_netServer)
                        pPlayer->opos.z = pPlayer->pos.z;

                    pPlayer->pos.z += q;
                    pPlayer->truefz += q;
                    pPlayer->truecz += q;

                    if (g_netServer || numplayers > 1)
                        pPlayer->opos.z = pPlayer->pos.z;
                }

                if (sprite[j].statnum != STAT_EFFECTOR)
                {
                    actor[j].bpos.z = sprite[j].z;
                    sprite[j].z += q;
                }

                actor[j].floorz   = pSector->floorz;
                actor[j].ceilingz = pSector->ceilingz;
            }

            if (pData[0]) //If in motion
            {
                if (klabs(pSector->floorz-pData[2]) <= SP(spriteNum))
                {
                    G_ActivateWarpElevators(spriteNum,0);
                    break;
                }

                // If we still see the opening, we can't yet teleport.
                if (pData[0]==-1)
                {
                    if (pSector->floorz > pData[3])
                        break;
                }
                else if (pSector->ceilingz < pData[4]) break;

                if (pData[1] == 0) break;
                pData[1] = 0;

                for (SPRITES_OF(STAT_EFFECTOR, j))
                {
                    if (spriteNum != j && sprite[j].lotag == SE_17_WARP_ELEVATOR)
                        if (pSector->hitag-pData[0] == sector[sprite[j].sectnum].hitag
                                && spriteHitag == sprite[j].hitag)
                            break;
                }

                if (j == -1) break;

                int32_t nextk;

                for (SPRITES_OF_SECT_SAFE(pSprite->sectnum, k, nextk))
                {
                    if (sprite[k].statnum == STAT_PLAYER && sprite[k].owner >= 0)
                    {
                        int const  warpPlayer = P_Get(k);
                        auto const pPlayer    = g_player[warpPlayer].ps;
                        vec2_t const diff = sprite[j].xy - pSprite->xy;
                        pPlayer->pos.xy += diff;

                        pPlayer->opos.z -= pPlayer->pos.z;
                        pPlayer->pos.z = sector[sprite[j].sectnum].floorz - (pSector->floorz - pPlayer->pos.z);
                        pPlayer->opos.z += pPlayer->pos.z;

                        actor[k].floorz     = sector[sprite[j].sectnum].floorz;
                        actor[k].ceilingz   = sector[sprite[j].sectnum].ceilingz;
                        pPlayer->opos.xy    = pPlayer->pos.xy;
                        pPlayer->bobpos     = pPlayer->pos.xy;
                        pPlayer->truefz     = actor[k].floorz;
                        pPlayer->truecz     = actor[k].ceilingz;
                        pPlayer->bobcounter = 0;

                        changespritesect(k, sprite[j].sectnum);
                        pPlayer->cursectnum = sprite[j].sectnum;
                    }
                    else if (sprite[k].statnum != STAT_EFFECTOR)
                    {
                        sprite[k].xy += sprite[j].xy - pSprite->xy;
                        actor[k].bpos.xy = sprite[k].xy;

                        actor[k].bpos.z -= sprite[k].z;
                        sprite[k].z = sector[sprite[j].sectnum].floorz - (pSector->floorz - sprite[k].z);
                        actor[k].bpos.z += sprite[k].z;

                        changespritesect(k,sprite[j].sectnum);
                        setsprite(k,&sprite[k].xyz);

                        actor[k].floorz   = sector[sprite[j].sectnum].floorz;
                        actor[k].ceilingz = sector[sprite[j].sectnum].ceilingz;
                    }
                }
            }
            break;
        }

        case SE_18_INCREMENTAL_SECTOR_RISE_FALL:
            if (pData[0])
            {
                if (pSprite->pal)
                {
                    if (pSprite->ang == 512)
                    {
                        pSector->ceilingz -= pSector->extra;
                        if (pSector->ceilingz <= pData[1])
                        {
                            pSector->ceilingz = pData[1];
                            DELETE_SPRITE_AND_CONTINUE(spriteNum);
                        }
                    }
                    else
                    {
                        pSector->floorz += pSector->extra;

                        for (bssize_t SPRITES_OF_SECT(pSprite->sectnum, sectSprite))
                        {
                            if (sprite[sectSprite].picnum == APLAYER && sprite[sectSprite].owner >= 0 && g_player[P_Get(sectSprite)].ps->on_ground == 1)
                                g_player[P_Get(sectSprite)].ps->pos.z += pSector->extra;

                            if (sprite[sectSprite].zvel == 0 && sprite[sectSprite].statnum != STAT_EFFECTOR && sprite[sectSprite].statnum != STAT_PROJECTILE)
                            {
                                actor[sectSprite].bpos.z = sprite[sectSprite].z += pSector->extra;
                                actor[sectSprite].floorz = pSector->floorz;
                            }
                        }

                        if (pSector->floorz >= pData[1])
                        {
                            pSector->floorz = pData[1];
                            DELETE_SPRITE_AND_CONTINUE(spriteNum);
                        }
                    }
                }
                else
                {
                    if (pSprite->ang == 512)
                    {
                        pSector->ceilingz += pSector->extra;
                        if (pSector->ceilingz >= pSprite->z)
                        {
                            pSector->ceilingz = pSprite->z;
                            DELETE_SPRITE_AND_CONTINUE(spriteNum);
                        }
                    }
                    else
                    {
                        pSector->floorz -= pSector->extra;

                        for (bssize_t SPRITES_OF_SECT(pSprite->sectnum, sectSprite))
                        {
                            if (sprite[sectSprite].picnum == APLAYER && sprite[sectSprite].owner >= 0 &&g_player[P_Get(sectSprite)].ps->on_ground == 1)
                                g_player[P_Get(sectSprite)].ps->pos.z -= pSector->extra;

                            if (sprite[sectSprite].zvel == 0 && sprite[sectSprite].statnum != STAT_EFFECTOR && sprite[sectSprite].statnum != STAT_PROJECTILE)
                            {
                                actor[sectSprite].bpos.z = sprite[sectSprite].z -= pSector->extra;
                                actor[sectSprite].floorz = pSector->floorz;
                            }
                        }

                        if (pSector->floorz <= pSprite->z)
                        {
                            pSector->floorz = pSprite->z;
                            DELETE_SPRITE_AND_CONTINUE(spriteNum);
                        }
                    }
                }

                if (++pData[2] >= pSprite->hitag)
                {
                    pData[2] = 0;
                    pData[0] = 0;
                }
            }
            break;

        case SE_19_EXPLOSION_LOWERS_CEILING: //Battlestar galactia shields

            if (pData[0])
            {
                if (pData[0] == 1)
                {
                    pData[0]++;
                    x = pSector->wallptr;
                    q = x+pSector->wallnum;

                    for (j=x; j<q; j++)
                    {
                        if (wall[j].overpicnum == BIGFORCE)
                        {
                            wall[j].cstat &= (128+32+8+4+2);
                            wall[j].overpicnum = 0;

                            if (wall[j].nextwall >= 0)
                            {
                                wall[wall[j].nextwall].overpicnum = 0;
                                wall[wall[j].nextwall].cstat &= (128+32+8+4+2);
                            }
                        }
                    }
                }

                if (pSector->ceilingz < pSector->floorz)
                    pSector->ceilingz += SP(spriteNum);
                else
                {
                    pSector->ceilingz = pSector->floorz;

                    for (SPRITES_OF(STAT_EFFECTOR, j))
                    {
                        if (sprite[j].lotag == SE_0_ROTATING_SECTOR && sprite[j].hitag==spriteHitag)
                        {
                            sectortype *const pSector     = &sector[sprite[j].sectnum];
                            int const         ownerSector = sprite[sprite[j].owner].sectnum;

                            pSector->ceilingpal   = sector[ownerSector].floorpal;
                            pSector->floorpal     = pSector->ceilingpal;
                            pSector->ceilingshade = sector[ownerSector].floorshade;
                            pSector->floorshade   = pSector->ceilingshade;

                            actor[sprite[j].owner].t_data[0] = 2;
                        }
                    }

                    DELETE_SPRITE_AND_CONTINUE(spriteNum);
                }
            }
            else //Not hit yet
            {
                if (G_FindExplosionInSector(pSprite->sectnum) >= 0)
                {
                    P_DoQuote(QUOTE_UNLOCKED, g_player[myconnectindex].ps);

                    for (SPRITES_OF(STAT_EFFECTOR, l))
                    {
                        switch (sprite[l].lotag & 0x7fff)
                        {
                        case SE_0_ROTATING_SECTOR:
                            if (sprite[l].hitag == spriteHitag)
                            {
                                int const spriteOwner = sprite[l].owner;
                                int const sectNum     = sprite[l].sectnum;

                                sector[sectNum].ceilingshade = sprite[spriteOwner].shade;
                                sector[sectNum].floorshade   = sector[sectNum].ceilingshade;
                                sector[sectNum].ceilingpal   = sprite[spriteOwner].pal;
                                sector[sectNum].floorpal     = sector[sectNum].ceilingpal;
                            }
                            break;

                        case SE_1_PIVOT:
                        case SE_12_LIGHT_SWITCH:
//                        case SE_18_INCREMENTAL_SECTOR_RISE_FALL:
                        case SE_19_EXPLOSION_LOWERS_CEILING:
                            if (spriteHitag == sprite[l].hitag)
                                if (actor[l].t_data[0] == 0)
                                {
                                    actor[l].t_data[0] = 1;  // Shut them all on
                                    sprite[l].owner    = spriteNum;
                                }

                            break;
                        }
                    }
                }
            }

            break;

        case SE_20_STRETCH_BRIDGE: //Extend-o-bridge
            if (pData[0] == 0) break;
            pSprite->xvel = (pData[0] == 1) ? 8 : -8;

            if (pSprite->xvel)   //Moving
            {
                vec2_t const vect = { (pSprite->xvel * sintable[(pSprite->ang + 512) & 2047]) >> 14,
                                      (pSprite->xvel * sintable[pSprite->ang & 2047]) >> 14 };

                pData[3] += pSprite->xvel;

                pSprite->x += vect.x;
                pSprite->y += vect.y;

                if (pData[3] <= 0 || (pData[3] >> 6) >= (SP(spriteNum) >> 6))
                {
                    pSprite->x -= vect.x;
                    pSprite->y -= vect.y;
                    pData[0] = 0;
                    A_CallSound(pSprite->sectnum, spriteNum);
                    break;
                }

                for (bssize_t nextSprite, SPRITES_OF_SECT_SAFE(pSprite->sectnum, sectSprite, nextSprite))
                {
                    if (sprite[sectSprite].statnum != STAT_EFFECTOR && sprite[sectSprite].zvel == 0)
                    {
                        sprite[sectSprite].x += vect.x;
                        sprite[sectSprite].y += vect.y;

                        setsprite(sectSprite, &sprite[sectSprite].xyz);

                        if (sector[sprite[sectSprite].sectnum].floorstat & 2 && sprite[sectSprite].statnum == STAT_ZOMBIEACTOR)
                            A_Fall(sectSprite);
                    }
                }

                dragpoint((int16_t)pData[1], wall[pData[1]].x + vect.x, wall[pData[1]].y + vect.y, 0);
                dragpoint((int16_t)pData[2], wall[pData[2]].x + vect.x, wall[pData[2]].y + vect.y, 0);

                for (bssize_t TRAVERSE_CONNECT(playerNum))
                {
                    auto const pPlayer = g_player[playerNum].ps;

                    if (pPlayer->cursectnum == pSprite->sectnum && pPlayer->on_ground)
                    {
                        pPlayer->pos.x += vect.x;
                        pPlayer->pos.y += vect.y;

                        pPlayer->opos.x = pPlayer->pos.x;
                        pPlayer->opos.y = pPlayer->pos.y;

                        pPlayer->pos.z += pPlayer->spritezoffset;
                        setsprite(pPlayer->i, &pPlayer->pos);
                        pPlayer->pos.z -= pPlayer->spritezoffset;
                    }
                }

                pSector->floorxpanning -= vect.x >> 3;
                pSector->floorypanning -= vect.y >> 3;

                pSector->ceilingxpanning -= vect.x >> 3;
                pSector->ceilingypanning -= vect.y >> 3;
            }

            break;

        case SE_21_DROP_FLOOR: // Cascading effect
        {
            if (pData[0] == 0) break;

            int32_t *zptr = (pSprite->ang == 1536) ? &pSector->ceilingz : &pSector->floorz;

            if (pData[0] == 1)   //Decide if the s->sectnum should go up or down
            {
                pSprite->zvel = ksgn(pSprite->z-*zptr) * (SP(spriteNum)<<4);
                pData[0]++;
            }

            if (pSector->extra == 0)
            {
                *zptr += pSprite->zvel;

                if (klabs(*zptr-pSprite->z) < 1024)
                {
                    *zptr = pSprite->z;
                    DELETE_SPRITE_AND_CONTINUE(spriteNum); //All done   // SE_21_KILLIT, see sector.c
                }
            }
            else pSector->extra--;
            break;
        }

        case SE_22_TEETH_DOOR:
            if (pData[1])
            {
                if (GetAnimationGoal(&sector[pData[0]].ceilingz) >= 0)
                    pSector->ceilingz += pSector->extra*9;
                else pData[1] = 0;
            }
            break;

        case SE_24_CONVEYOR:
        case SE_34_CONVEYOR2:
        {
            if (pData[4])
                break;

            vec2_t const vect = { (SP(spriteNum) * sintable[(pSprite->ang + 512) & 2047]) >> 18,
                                  (SP(spriteNum) * sintable[pSprite->ang & 2047]) >> 18 };

            k = 0;

            for (bssize_t nextSprite, SPRITES_OF_SECT_SAFE(pSprite->sectnum, sectSprite, nextSprite))
            {
                if (sprite[sectSprite].zvel < 0)
                    continue;

                switch (sprite[sectSprite].statnum)
                {
                    case STAT_MISC:
                        switch (tileGetMapping(sprite[sectSprite].picnum))
                        {
                            case BLOODPOOL__:
                            case PUKE__:
                            case FOOTPRINTS__:
                            case FOOTPRINTS2__:
                            case FOOTPRINTS3__:
                            case FOOTPRINTS4__:
                            case BULLETHOLE__:
                            case BLOODSPLAT1__:
                            case BLOODSPLAT2__:
                            case BLOODSPLAT3__:
                            case BLOODSPLAT4__: sprite[sectSprite].xrepeat = sprite[sectSprite].yrepeat = 0; continue;

                            case LASERLINE__: continue;
                        }
                        fallthrough__;
                    case STAT_STANDABLE:
                        if (sprite[sectSprite].picnum == TRIPBOMB)
                            break;
                        fallthrough__;
                    case STAT_ACTOR:
                    case STAT_DEFAULT:
                        if ((sprite[sectSprite].picnum >= BOLT1 && sprite[sectSprite].picnum <= BOLT1 + 3)
                            || (sprite[sectSprite].picnum >= SIDEBOLT1 && sprite[sectSprite].picnum <= SIDEBOLT1 + 3)
                            || A_CheckSwitchTile(sectSprite))
                            break;

                        if (!(sprite[sectSprite].picnum >= CRANE && sprite[sectSprite].picnum <= CRANE + 3))
                        {
                            if (sprite[sectSprite].z > actor[sectSprite].floorz - ZOFFSET2)
                            {
                                actor[sectSprite].bpos.xy = sprite[sectSprite].xy;

                                sprite[sectSprite].x += vect.x >> 2;
                                sprite[sectSprite].y += vect.y >> 2;

                                setsprite(sectSprite, &sprite[sectSprite].xyz);

                                if (sector[sprite[sectSprite].sectnum].floorstat & 2)
                                    if (sprite[sectSprite].statnum == STAT_ZOMBIEACTOR)
                                        A_Fall(sectSprite);
                            }
                        }
                        break;
                }
            }

            for (bssize_t TRAVERSE_CONNECT(playerNum))
            {
                auto const pPlayer = g_player[playerNum].ps;

                if (pPlayer->cursectnum == pSprite->sectnum && pPlayer->on_ground)
                {
                    if (klabs(pPlayer->pos.z - pPlayer->truefz) < pPlayer->spritezoffset + ZOFFSET3)
                    {
                        pPlayer->fric.x += vect.x << 3;
                        pPlayer->fric.y += vect.y << 3;
                    }
                }
            }
            pSector->floorxpanning += SP(spriteNum)>>7;
            actor[spriteNum].bpos.x = pSector->floorxpanning;

            break;
        }

        case SE_35:
            if (pSector->ceilingz > pSprite->z)
            {
                for (j = 0; j < 8; j++)
                {
                    pSprite->ang += krand()&511;
                    k = A_Spawn(spriteNum, SMALLSMOKE);
                    sprite[k].xvel = 96+(krand()&127);
                    A_SetSprite(k, CLIPMASK0);
                    setsprite(k, &sprite[k].xyz);
                    if (rnd(16))
                        A_Spawn(spriteNum, EXPLOSION2);
                }

            }
            switch (pData[0])
            {
            case 0:
                pSector->ceilingz += pSprite->yvel;
                if (pSector->ceilingz > pSector->floorz)
                    pSector->floorz = pSector->ceilingz;
                if (pSector->ceilingz > pSprite->z+ZOFFSET5)
                    pData[0]++;
                break;
            case 1:
                pSector->ceilingz-=(pSprite->yvel<<2);
                if (pSector->ceilingz < pData[4])
                {
                    pSector->ceilingz = pData[4];
                    pData[0] = 0;
                }
                break;
            }
            break;

        case SE_25_PISTON: //PISTONS
            if (pData[4] == 0) break;

            if (pSector->floorz <= pSector->ceilingz)
                pSprite->shade = 0;
            else if (pSector->ceilingz <= pData[3])
                pSprite->shade = 1;

            if (pSprite->shade)
            {
                pSector->ceilingz += SP(spriteNum)<<4;
                if (pSector->ceilingz > pSector->floorz)
                    pSector->ceilingz = pSector->floorz;
            }
            else
            {
                pSector->ceilingz   -= SP(spriteNum)<<4;
                if (pSector->ceilingz < pData[3])
                    pSector->ceilingz = pData[3];
            }

            break;

        case SE_26_ESCALATOR:
        {
            int32_t p, nextj;

            pSprite->xvel = pSector->extra != 256 ? pSector->extra : 32;
            l = (pSprite->xvel*sintable[(pSprite->ang+512)&2047])>>14;
            x = (pSprite->xvel*sintable[pSprite->ang&2047])>>14;

            pSprite->shade++;
            if (pSprite->shade > 7)
            {
                pSprite->x = pData[3];
                pSprite->y = pData[4];
                pSector->floorz -= ((pSprite->zvel*pSprite->shade)-pSprite->zvel);
                pSprite->shade = 0;
            }
            else
                pSector->floorz += pSprite->zvel;

            for (SPRITES_OF_SECT_SAFE(pSprite->sectnum, j, nextj))
            {
                if (sprite[j].statnum != STAT_EFFECTOR && sprite[j].statnum != STAT_PLAYER && sprite[j].statnum != STAT_PROJECTILE)
                {
                    actor[j].bpos.xy = sprite[j].xy;

                    sprite[j].x += l;
                    sprite[j].y += x;
                    sprite[j].z += pSprite->zvel;

                    setsprite(j, &sprite[j].xyz);
                }
            }

            for (TRAVERSE_CONNECT(p))
            {
                auto const pPlayer = g_player[p].ps;

                if (pSprite->sectnum == pPlayer->cursectnum && pPlayer->on_ground)
                {
                    pPlayer->pos.x += l;
                    pPlayer->pos.y += x;
                    pPlayer->pos.z += pSprite->zvel;

                    updatesector(pPlayer->pos.x, pPlayer->pos.y, &pPlayer->cursectnum);
                    changespritesect(pPlayer->i, pPlayer->cursectnum);

                    pPlayer->bobpos.x += l;
                    pPlayer->bobpos.y += x;

                    if (g_netServer || numplayers > 1)
                        pPlayer->opos.xy = pPlayer->pos.xy;

                    if (sprite[pPlayer->i].extra <= 0)
                        sprite[pPlayer->i].xy = pPlayer->pos.xy;
                }
            }

            A_MoveSector(spriteNum);
            setsprite(spriteNum,&pSprite->xyz);

            break;
        }

        case SE_27_DEMO_CAM:
        {
            if (pSprite->extra < 1 && (ud.recstat == 0 || !ud.democams) && g_BenchmarkMode == BENCHMARKMODE_OFF) break;

            if (klabs(pSprite->extra) == 2)
            {
                actor[spriteNum].tempang = pSprite->ang;
                if (ud.camerasprite != spriteNum)
                {
                    //level the camera out by default (yvel stores the up/down angle)
                    pSprite->yvel = 100;
                    ud.camerasprite = spriteNum;
                }

            findCameraDestination:
                if (pSprite->owner == spriteNum)
                {
                    pSprite->owner = A_FindLocatorWithHiLoTags(pSprite->hitag, pData[0], -1);

                    //reset our elapsed time since reaching a locator
                    pData[1] = 0;
                    //store our starting point
                    pData[2] = pSprite->x;
                    pData[3] = pSprite->y;
                    pData[4] = pSprite->z;
                    pData[5] = pSprite->ang;
                    pData[6] = pSprite->yvel;
                    if (pSprite->owner != -1)
                    {
                        spritetype* const destLocator = &sprite[pSprite->owner];
                        int32_t subjectLocatorIndex = A_FindLocatorWithHiLoTags(pSprite->hitag, destLocator->owner, -1);
                        pData[7] = G_GetAngleDelta(pData[5], destLocator->ang);
                        //level the camera out by default (pData[8] stores our destination up/down angle)
                        pData[8] = 100;
                        if (subjectLocatorIndex != -1)
                        {
                            spritetype* const subjectLocator = &sprite[subjectLocatorIndex];
                            const vec3_t cameraDirection = {subjectLocator->x - destLocator->x,
                                                            subjectLocator->y - destLocator->y,
                                                            subjectLocator->z - destLocator->z};
                            pData[7] = G_GetAngleDelta(pData[5], getangle(cameraDirection.x,
                                                                          cameraDirection.y));
                            pData[8] = (((int32_t) getangle(-ksqrt(cameraDirection.x*cameraDirection.x+cameraDirection.y*cameraDirection.y), cameraDirection.z)*(400.f/1024.f)))-300;
                        }
                    }

                    //if we are benchmarking, take a screenshot at each waypoint (camera start point/locator)
                    benchmarkScreenshot = g_BenchmarkMode == BENCHMARKMODE_GENERATE_REFERENCE;
                }
                if (pSprite->owner == -1)
                {
                    break;
                }

                spritetype* const destLocator = &sprite[pSprite->owner];
                if (pData[1] == destLocator->extra)
                {
                    pSprite->owner = spriteNum;
                    ++pData[0];
                    goto findCameraDestination;
                }

                //smoothstep to the new location and camera direction over the duration (in ticks) stored in the destLocator's extra value
                const vec3_t heading = {destLocator->x-pData[2],
                                        destLocator->y-pData[3],
                                        destLocator->z-pData[4]};
                float interpolation = (pData[1]/(float) destLocator->extra);
                interpolation = interpolation*interpolation*(3-2*interpolation);
                pSprite->x = pData[2]+interpolation*heading.x;
                pSprite->y = pData[3]+interpolation*heading.y;
                pSprite->z = pData[4]+interpolation*heading.z;
                pSprite->ang = pData[5]+interpolation*pData[7];
                pSprite->yvel = (pData[6]+((int32_t) interpolation*pData[8])+100)%400-100;

                //increment elapsed time
                ++pData[1];
            }
            else
            {
                actor[spriteNum].tempang = pSprite->ang;

                int const  p  = A_FindPlayer(pSprite, &x);
                auto const ps = g_player[p].ps;

                if (sprite[ps->i].extra > 0 && myconnectindex == screenpeek)
                {
                    if (pData[0] < 0)
                    {
                        ud.camerasprite = spriteNum;
                        pData[0]++;
                    }
                    else if (ud.recstat == 2 && ps->newowner == -1)
                    {
                        if (cansee(pSprite->x,pSprite->y,pSprite->z,SECT(spriteNum),ps->pos.x,ps->pos.y,ps->pos.z,ps->cursectnum))
                        {
                            if (x < (int32_t)((unsigned)spriteHitag))
                            {
                                ud.camerasprite = spriteNum;
                                pData[0] = 999;
                                pSprite->ang += G_GetAngleDelta(pSprite->ang,getangle(ps->pos.x-pSprite->x,ps->pos.y-pSprite->y))>>3;
                                SP(spriteNum) = 100+((pSprite->z-ps->pos.z)/257);

                            }
                            else if (pData[0] == 999)
                            {
                                if (ud.camerasprite == spriteNum)
                                    pData[0] = 0;
                                else pData[0] = -10;
                                ud.camerasprite = spriteNum;

                            }
                        }
                        else
                        {
                            pSprite->ang = getangle(ps->pos.x-pSprite->x,ps->pos.y-pSprite->y);

                            if (pData[0] == 999)
                            {
                                if (ud.camerasprite == spriteNum)
                                    pData[0] = 0;
                                else pData[0] = -20;
                                ud.camerasprite = spriteNum;
                            }
                        }
                    }
                }
            }
            break;
        }

        case SE_28_LIGHTNING:
        {
            if (pData[5] > 0)
            {
                pData[5]--;
                break;
            }

            if (T1(spriteNum) == 0)
            {
                A_FindPlayer(pSprite,&x);
                if (x > 15500)
                    break;
                T1(spriteNum) = 1;
                T2(spriteNum) = 64 + (krand()&511);
                T3(spriteNum) = 0;
            }
            else
            {
                T3(spriteNum)++;
                if (T3(spriteNum) > T2(spriteNum))
                {
                    T1(spriteNum) = 0;
                    g_player[screenpeek].ps->visibility = ud.const_visibility;
                    break;
                }
                else if (T3(spriteNum) == (T2(spriteNum)>>1))
                    A_PlaySound(THUNDER,spriteNum);
                else if (T3(spriteNum) == (T2(spriteNum)>>3))
                    A_PlaySound(LIGHTNING_SLAP,spriteNum);
                else if (T3(spriteNum) == (T2(spriteNum)>>2))
                {
                    for (SPRITES_OF(STAT_DEFAULT, j))
                        if (sprite[j].picnum == NATURALLIGHTNING && sprite[j].hitag == pSprite->hitag)
                            sprite[j].cstat |= 32768;
                }
                else if (T3(spriteNum) > (T2(spriteNum)>>3) && T3(spriteNum) < (T2(spriteNum)>>2))
                {
                    if (cansee(pSprite->x,pSprite->y,pSprite->z,pSprite->sectnum,g_player[screenpeek].ps->pos.x,g_player[screenpeek].ps->pos.y,g_player[screenpeek].ps->pos.z,g_player[screenpeek].ps->cursectnum))
                        j = 1;
                    else j = 0;

                    if (rnd(192) && (T3(spriteNum)&1))
                    {
                        if (j)
                            g_player[screenpeek].ps->visibility = 0;
                    }
                    else if (j)
                        g_player[screenpeek].ps->visibility = ud.const_visibility;

                    for (SPRITES_OF(STAT_DEFAULT, j))
                    {
                        if (sprite[j].picnum == NATURALLIGHTNING && sprite[j].hitag == pSprite->hitag)
                        {
                            if (rnd(32) && (T3(spriteNum)&1))
                            {
                                int32_t p;
                                DukePlayer_t *ps;

                                sprite[j].cstat &= 32767;
                                A_Spawn(j,SMALLSMOKE);
                                G_AddGameLight(j, sprite[j].sectnum, { 0, 0, 4096 }, 16384, 0, 100,72+(88<<8)+(140<<16), PR_LIGHT_PRIO_HIGH_GAME);
                                p = A_FindPlayer(pSprite, NULL);
                                ps = g_player[p].ps;

                                x = ldist(&sprite[ps->i], &sprite[j]);
                                if (x < 768)
                                {
                                    if (!A_CheckSoundPlaying(ps->i,DUKE_LONGTERM_PAIN))
                                        A_PlaySound(DUKE_LONGTERM_PAIN,ps->i);
                                    A_PlaySound(SHORT_CIRCUIT,ps->i);
                                    sprite[ps->i].extra -= 8+(krand()&7);

                                    P_PalFrom(ps, 32, 16,0,0);
                                }
                                break;
                            }
                            else sprite[j].cstat |= 32768;
                        }
                    }
                }
            }
            break;
        }

        case SE_29_WAVES:
            pSprite->hitag += 64;
            l = mulscale12((int32_t)pSprite->yvel,sintable[pSprite->hitag&2047]);
            pSector->floorz = pSprite->z + l;
            break;

        case SE_31_FLOOR_RISE_FALL: // True Drop Floor
            if (pData[0] == 1)
            {
                // Choose dir

                if (pData[3] > 0)
                {
                    pData[3]--;
                    break;
                }

                if (pData[2] == 1) // Retract
                {
                    if (SA(spriteNum) != 1536)
                        HandleSE31(spriteNum, 1, pSprite->z, 0, pSprite->z-pSector->floorz);
                    else
                        HandleSE31(spriteNum, 1, pData[1], 0, pData[1]-pSector->floorz);

                    Yax_SetBunchZs(pSector-sector, YAX_FLOOR, pSector->floorz);

                    break;
                }

                if ((pSprite->ang&2047) == 1536)
                    HandleSE31(spriteNum, 0, pSprite->z, 1, pSprite->z-pSector->floorz);
                else
                    HandleSE31(spriteNum, 0, pData[1], 1, pData[1]-pSprite->z);

                Yax_SetBunchZs(pSector-sector, YAX_FLOOR, pSector->floorz);
            }
            break;

        case SE_32_CEILING_RISE_FALL: // True Drop Ceiling
            if (pData[0] == 1)
            {
                // Choose dir

                if (pData[2] == 1) // Retract
                {
                    if (SA(spriteNum) != 1536)
                    {
                        if (klabs(pSector->ceilingz - pSprite->z) < (SP(spriteNum)<<1))
                        {
                            pSector->ceilingz = pSprite->z;
                            A_CallSound(pSprite->sectnum,spriteNum);
                            pData[2] = 0;
                            pData[0] = 0;
                        }
                        else pSector->ceilingz += ksgn(pSprite->z-pSector->ceilingz)*SP(spriteNum);
                    }
                    else
                    {
                        if (klabs(pSector->ceilingz - pData[1]) < (SP(spriteNum)<<1))
                        {
                            pSector->ceilingz = pData[1];
                            A_CallSound(pSprite->sectnum,spriteNum);
                            pData[2] = 0;
                            pData[0] = 0;
                        }
                        else pSector->ceilingz += ksgn(pData[1]-pSector->ceilingz)*SP(spriteNum);
                    }

                    Yax_SetBunchZs(pSector-sector, YAX_CEILING, pSector->ceilingz);

                    break;
                }

                if ((pSprite->ang&2047) == 1536)
                {
                    if (klabs(pSector->ceilingz-pSprite->z) < (SP(spriteNum)<<1))
                    {
                        pData[0] = 0;
                        pData[2] = !pData[2];
                        A_CallSound(pSprite->sectnum,spriteNum);
                        pSector->ceilingz = pSprite->z;
                    }
                    else pSector->ceilingz += ksgn(pSprite->z-pSector->ceilingz)*SP(spriteNum);
                }
                else
                {
                    if (klabs(pSector->ceilingz-pData[1]) < (SP(spriteNum)<<1))
                    {
                        pData[0] = 0;
                        pData[2] = !pData[2];
                        A_CallSound(pSprite->sectnum,spriteNum);
                    }
                    else pSector->ceilingz -= ksgn(pSprite->z-pData[1])*SP(spriteNum);
                }

                Yax_SetBunchZs(pSector-sector, YAX_CEILING, pSector->ceilingz);
            }
            break;

#ifndef EDUKE32_STANDALONE
        case SE_33_QUAKE_DEBRIS:
            if (!FURY && g_earthquakeTime > 0 && (krand()&7) == 0)
                RANDOMSCRAP(pSprite, spriteNum);
            break;
#endif
        case SE_36_PROJ_SHOOTER:
            if (pData[0])
            {
                if (pData[0] == 1)
                    A_Shoot(spriteNum,pSector->extra);
                else if (pData[0] == GAMETICSPERSEC*5)
                    pData[0] = 0;
                pData[0]++;
            }
            break;

        case 128: //SE to control glass breakage
            {
                walltype *pWall = &wall[pData[2]];

                pWall->cstat &= (255-32);
                pWall->cstat |= 16;
                if (pWall->nextwall >= 0)
                {
                    wall[pWall->nextwall].cstat &= (255-32);
                    wall[pWall->nextwall].cstat |= 16;
                }

                pWall->overpicnum++;
                if (pWall->nextwall >= 0)
                    wall[pWall->nextwall].overpicnum++;

                if (pData[0] < pData[1]) pData[0]++;
                else
                {
                    pWall->cstat &= (128+32+8+4+2);
                    if (pWall->nextwall >= 0)
                        wall[pWall->nextwall].cstat &= (128+32+8+4+2);
                    DELETE_SPRITE_AND_CONTINUE(spriteNum);
                }
            }
            break;

        case SE_130:
            if (pData[0] > 80)
            {
                DELETE_SPRITE_AND_CONTINUE(spriteNum);
            }
            else pData[0]++;

            x = pSector->floorz-pSector->ceilingz;

            if (rnd(64))
            {
                k = A_Spawn(spriteNum,EXPLOSION2);
                sprite[k].xrepeat = sprite[k].yrepeat = 2+(krand()&7);
                sprite[k].z = pSector->floorz-(krand()%x);
                sprite[k].ang += 256-(krand()%511);
                sprite[k].xvel = krand()&127;
                A_SetSprite(k,CLIPMASK0);
            }
            break;

        case SE_131:
            if (pData[0] > 40)
            {
                DELETE_SPRITE_AND_CONTINUE(spriteNum);
            }
            else pData[0]++;

            x = pSector->floorz-pSector->ceilingz;

            if (rnd(32))
            {
                k = A_Spawn(spriteNum,EXPLOSION2);
                sprite[k].xrepeat = sprite[k].yrepeat = 2+(krand()&3);
                sprite[k].z = pSector->floorz-(krand()%x);
                sprite[k].ang += 256-(krand()%511);
                sprite[k].xvel = krand()&127;
                A_SetSprite(k,CLIPMASK0);
            }
            break;

        case SE_49_POINT_LIGHT:
        case SE_50_SPOT_LIGHT:
            changespritestat(spriteNum, STAT_LIGHT);
            break;
        }
next_sprite:
        spriteNum = nextSprite;
    }

    //Sloped sin-wave floors!
    for (SPRITES_OF(STAT_EFFECTOR, spriteNum))
    {
        auto const s = &sprite[spriteNum];

        if (s->lotag == SE_29_WAVES)
        {
            auto const sc = (usectorptr_t)&sector[s->sectnum];

            if (sc->wallnum == 4)
            {
                auto const pWall = &wall[sc->wallptr+2];
                if (pWall->nextsector >= 0)
                    alignflorslope(s->sectnum, pWall->x,pWall->y, sector[pWall->nextsector].floorz);
            }
        }
    }
}

#ifdef POLYMER
static void G_DoEffectorLights(void)  // STATNUM 14
{
    static int16_t lasti = -1;
    int16_t i;

    if (lasti != -1 && sprite[lasti].statnum == STAT_LIGHT)
    {
        i = lasti;
        goto in;
    }

    for (SPRITES_OF(STAT_LIGHT, i))
    {
in:
        if ((int32_t)(totalclock - ototalclock) >= TICSPERFRAME || dukeMaybeDrawFrame())
        {
            lasti = i;
            return;
        }

        switch (sprite[i].lotag)
        {
        case SE_49_POINT_LIGHT:
        {
            if (!A_CheckSpriteFlags(i, SFLAG_NOLIGHT) && videoGetRenderMode() == REND_POLYMER &&
                    !(A_CheckSpriteFlags(i, SFLAG_USEACTIVATOR) && sector[sprite[i].sectnum].lotag & 16384))
            {
                if (practor[i].lightptr == NULL)
                {
#pragma pack(push,1)
                    _prlight mylight;
#pragma pack(pop)
                    mylight.sector = SECT(i);
                    mylight.xyz = sprite[i].xyz;
                    mylight.range = SHT(i);
                    mylight.color[0] = sprite[i].xvel;
                    mylight.color[1] = sprite[i].yvel;
                    mylight.color[2] = sprite[i].zvel;
                    mylight.radius = 0;
                    mylight.angle = SA(i);
                    mylight.horiz = SH(i);
                    mylight.minshade = sprite[i].xoffset;
                    mylight.maxshade = sprite[i].yoffset;
                    mylight.tilenum = 0;
                    mylight.publicflags.emitshadow = 0;
                    mylight.publicflags.negative = !!(CS(i) & 128);
                    mylight.owner = i;

                    if (CS(i) & 2)
                    {
                        if (CS(i) & 512)
                            mylight.priority = PR_LIGHT_PRIO_LOW;
                        else
                            mylight.priority = PR_LIGHT_PRIO_HIGH;
                    }
                    else
                        mylight.priority = PR_LIGHT_PRIO_MAX;

                    practor[i].lightId = polymer_addlight(&mylight);
                    if (practor[i].lightId >= 0)
                    {
                        practor[i].lightptr = &prlights[practor[i].lightId];
                        practor[i].lightrange = practor[i].olightrange = mylight.range;
                        practor[i].lightoffset = {};
                    }
                    break;
                }

                if (SHT(i) != practor[i].lightptr->range)
                {
                    practor[i].lightptr->range = SHT(i);
                    practor[i].lightptr->flags.invalidate = 1;
                }
                if ((sprite[i].xvel != practor[i].lightptr->color[0]) ||
                        (sprite[i].yvel != practor[i].lightptr->color[1]) ||
                        (sprite[i].zvel != practor[i].lightptr->color[2]))
                {
                    practor[i].lightptr->color[0] = sprite[i].xvel;
                    practor[i].lightptr->color[1] = sprite[i].yvel;
                    practor[i].lightptr->color[2] = sprite[i].zvel;
                }
                if ((int)!!(CS(i) & 128) != practor[i].lightptr->publicflags.negative) {
                    practor[i].lightptr->publicflags.negative = !!(CS(i) & 128);
                }
            }
            break;
        }
        case SE_50_SPOT_LIGHT:
        {
            if (!A_CheckSpriteFlags(i, SFLAG_NOLIGHT) && videoGetRenderMode() == REND_POLYMER &&
                    !(A_CheckSpriteFlags(i, SFLAG_USEACTIVATOR) && sector[sprite[i].sectnum].lotag & 16384))
            {
                if (practor[i].lightptr == NULL)
                {
#pragma pack(push,1)
                    _prlight mylight;
#pragma pack(pop)

                    mylight.sector = SECT(i);
                    mylight.xyz = sprite[i].xyz;
                    mylight.range = SHT(i);
                    mylight.color[0] = sprite[i].xvel;
                    mylight.color[1] = sprite[i].yvel;
                    mylight.color[2] = sprite[i].zvel;
                    mylight.radius = (256-(SS(i)+128))<<1;
                    mylight.faderadius = (int16_t)(mylight.radius * 0.75f);
                    mylight.angle = SA(i);
                    mylight.horiz = SH(i);
                    mylight.minshade = sprite[i].xoffset;
                    mylight.maxshade = sprite[i].yoffset;
                    mylight.tilenum = actor[i].htpicnum;
                    mylight.publicflags.emitshadow = !(CS(i) & 64);
                    mylight.publicflags.negative = !!(CS(i) & 128);
                    mylight.owner = i;

                    if (CS(i) & 2)
                    {
                        if (CS(i) & 512)
                            mylight.priority = PR_LIGHT_PRIO_LOW;
                        else
                            mylight.priority = PR_LIGHT_PRIO_HIGH;
                    }
                    else
                        mylight.priority = PR_LIGHT_PRIO_MAX;

                    practor[i].lightId = polymer_addlight(&mylight);
                    if (practor[i].lightId >= 0)
                    {
                        practor[i].lightptr = &prlights[practor[i].lightId];
                        practor[i].lightrange = practor[i].olightrange = mylight.range;
                        // Hack in case polymer_addlight tweaked the horiz value
                        if (practor[i].lightptr->horiz != SH(i))
                            SH(i) = practor[i].lightptr->horiz;
                        practor[i].lightoffset = {};
                    }
                    break;
                }

                if (SHT(i) != practor[i].lightptr->range)
                {
                    practor[i].lightptr->range = SHT(i);
                    practor[i].lightptr->flags.invalidate = 1;
                }
                if ((sprite[i].xvel != practor[i].lightptr->color[0]) ||
                        (sprite[i].yvel != practor[i].lightptr->color[1]) ||
                        (sprite[i].zvel != practor[i].lightptr->color[2]))
                {
                    practor[i].lightptr->color[0] = sprite[i].xvel;
                    practor[i].lightptr->color[1] = sprite[i].yvel;
                    practor[i].lightptr->color[2] = sprite[i].zvel;
                }
                if (((256-(SS(i)+128))<<1) != practor[i].lightptr->radius)
                {
                    practor[i].lightptr->radius = (256-(SS(i)+128))<<1;
                    practor[i].lightptr->faderadius = (int16_t)(practor[i].lightptr->radius * 0.75f);
                    practor[i].lightptr->flags.invalidate = 1;
                }
                if (SA(i) != practor[i].lightptr->angle)
                {
                    practor[i].lightptr->angle = SA(i);
                    practor[i].lightptr->flags.invalidate = 1;
                }
                if (SH(i) != practor[i].lightptr->horiz)
                {
                    practor[i].lightptr->horiz = SH(i);
                    practor[i].lightptr->flags.invalidate = 1;
                }
                if ((int)!(CS(i) & 64) != practor[i].lightptr->publicflags.emitshadow) {
                    practor[i].lightptr->publicflags.emitshadow = !(CS(i) & 64);
                }
                if ((int)!!(CS(i) & 128) != practor[i].lightptr->publicflags.negative) {
                    practor[i].lightptr->publicflags.negative = !!(CS(i) & 128);
                }
                practor[i].lightptr->tilenum = actor[i].htpicnum;
            }

            break;
        }
        }
    }

    lasti = -1;
}

int savedFires = 0;

static void A_DoLight(int spriteNum)
{
    auto const pSprite = &sprite[spriteNum];

    if (pSprite->statnum == STAT_PLAYER)
    {
        if (practor[spriteNum].lightptr != NULL && practor[spriteNum].lightcount)
        {
            if (!(--practor[spriteNum].lightcount))
                A_DeleteLight(spriteNum);
        }
    }
    else if (((sector[pSprite->sectnum].floorz - sector[pSprite->sectnum].ceilingz) < 16) || pSprite->z > sector[pSprite->sectnum].floorz || pSprite->z > actor[spriteNum].floorz ||
        (pSprite->picnum != SECTOREFFECTOR && ((pSprite->cstat & 32768) || pSprite->yrepeat < 4)) ||
        A_CheckSpriteFlags(spriteNum, SFLAG_NOLIGHT) || (A_CheckSpriteFlags(spriteNum, SFLAG_USEACTIVATOR) && sector[pSprite->sectnum].lotag & 16384))
    {
        if (practor[spriteNum].lightptr != NULL)
            A_DeleteLight(spriteNum);
    }
    else
    {
        if (practor[spriteNum].lightptr != NULL && practor[spriteNum].lightcount)
        {
            if (!(--practor[spriteNum].lightcount))
                A_DeleteLight(spriteNum);
        }

        if (pr_lighting != 1 || r_pr_defaultlights < 1)
            return;

#ifndef EDUKE32_STANDALONE
        if (FURY)
            return;

        for (int ii=0; ii<2; ii++)
        {
            if (pSprite->picnum <= 0)  // oob safety
                break;

            switch (tileGetMapping(pSprite->picnum-1+ii))
            {
            case DIPSWITCH__:
            case DIPSWITCH2__:
            case DIPSWITCH3__:
            case PULLSWITCH__:
            case SLOTDOOR__:
            case LIGHTSWITCH__:
            case SPACELIGHTSWITCH__:
            case SPACEDOORSWITCH__:
            case FRANKENSTINESWITCH__:
            case POWERSWITCH1__:
            case LOCKSWITCH1__:
            case POWERSWITCH2__:
            case TECHSWITCH__:
            case ACCESSSWITCH__:
            case ACCESSSWITCH2__:
                {
                    if ((pSprite->cstat & 32768) || A_CheckSpriteFlags(spriteNum, SFLAG_NOLIGHT))
                    {
                        if (practor[spriteNum].lightptr != NULL)
                            A_DeleteLight(spriteNum);
                        break;
                    }

                    vec3_t const d = { -(sintable[(pSprite->ang+512)&2047]>>7), -(sintable[(pSprite->ang)&2047]>>7), LIGHTZOFF(spriteNum) };

                    int16_t sectnum = pSprite->sectnum;
                    updatesector(pSprite->x, pSprite->y, &sectnum);

                    if ((unsigned) sectnum < MAXSECTORS && pSprite->z <= sector[sectnum].floorz && pSprite->z >= sector[sectnum].ceilingz)
                    G_AddGameLight(spriteNum, pSprite->sectnum, d, 384-ii*64, 0, 100, ii==0 ? (172+(200<<8)+(104<<16)) : 216+(52<<8)+(20<<16), PR_LIGHT_PRIO_LOW);
                }
                break;
            }
        }

        switch (tileGetMapping(pSprite->picnum))
        {
            case ATOMICHEALTH__:
            case FREEZEAMMO__:
                G_AddGameLight(spriteNum, pSprite->sectnum, { 0, 0, LIGHTZOFF(spriteNum) }, LIGHTRAD2(spriteNum), 0, 100,128+(128<<8)+(255<<16), PR_LIGHT_PRIO_HIGH_GAME);
                break;

            case FIRE__:
            case FIRE2__:
            case BURNING__:
            case BURNING2__:
                {
                    uint32_t color;
                    int32_t jj;

                    static int32_t savedfires[32][4];  // sectnum x y z

                    /*
                    if (Actor[i].floorz - Actor[i].ceilingz < 128) break;
                    if (s->z > Actor[i].floorz+2048) break;
                    */

                    switch (pSprite->pal)
                    {
                    case 1: color = 128+(128<<8)+(255<<16); break;
                    case 2: color = 255+(48<<8)+(48<<16); break;
                    case 8: color = 48+(255<<8)+(48<<16); break;
                    default: color = 240+(160<<8)+(80<<16); break;
                    }

                    for (jj=savedFires-1; jj>=0; jj--)
                        if (savedfires[jj][0]==pSprite->sectnum && savedfires[jj][1]==(pSprite->x>>3) &&
                            savedfires[jj][2]==(pSprite->y>>3) && savedfires[jj][3]==(pSprite->z>>7))
                            break;

                    if (jj==-1 && savedFires<32)
                    {
                        jj = savedFires;
                        G_AddGameLight(spriteNum, pSprite->sectnum, { 0, 0, LIGHTZOFF(spriteNum) }, LIGHTRAD2(spriteNum), 0, 100, color, PR_LIGHT_PRIO_HIGH_GAME);
                        savedfires[jj][0] = pSprite->sectnum;
                        savedfires[jj][1] = pSprite->x>>3;
                        savedfires[jj][2] = pSprite->y>>3;
                        savedfires[jj][3] = pSprite->z>>7;
                        savedFires++;
                    }
                }
                break;

            case OOZFILTER__:
                if (pSprite->xrepeat > 4)
                    G_AddGameLight(spriteNum, pSprite->sectnum, { 0, 0, LIGHTZOFF(spriteNum) }, LIGHTRAD2(spriteNum), 0, 100,176+(252<<8)+(120<<16), PR_LIGHT_PRIO_HIGH_GAME);
                break;
            case FLOORFLAME__:
            case FIREBARREL__:
            case FIREVASE__:
                G_AddGameLight(spriteNum, pSprite->sectnum, { 0, 0, LIGHTZOFF(spriteNum)<<1 }, LIGHTRAD2(spriteNum)>>1, 0, 100,255+(95<<8), PR_LIGHT_PRIO_HIGH_GAME);
                break;

            case EXPLOSION2__:
                if (!practor[spriteNum].lightcount)
                {
                    // XXX: This block gets CODEDUP'd too much.
                    vec3_t const offset = { ((sintable[(pSprite->ang+512)&2047])>>6), ((sintable[(pSprite->ang)&2047])>>6), LIGHTZOFF(spriteNum) };
                    G_AddGameLight(spriteNum, pSprite->sectnum, offset, LIGHTRAD(spriteNum), 0, 100,
                        240+(160<<8)+(80<<16), pSprite->yrepeat > 32 ? PR_LIGHT_PRIO_HIGH_GAME : PR_LIGHT_PRIO_LOW_GAME);
                }
                break;
            case FORCERIPPLE__:
                {
                    vec3_t const offset = { -((sintable[(pSprite->ang+512)&2047])>>5), -((sintable[(pSprite->ang)&2047])>>5), LIGHTZOFF(spriteNum) };
                    G_AddGameLight(spriteNum, pSprite->sectnum, offset, LIGHTRAD(spriteNum), 0, 100,80+(80<<8)+(255<<16), PR_LIGHT_PRIO_LOW_GAME);
                    practor[spriteNum].lightcount = 2;
                }
                break;
            case TRANSPORTERBEAM__:
                G_AddGameLight(spriteNum, pSprite->sectnum, { 0, 0, LIGHTZOFF(spriteNum) }, LIGHTRAD(spriteNum), 0, 100,80+(80<<8)+(255<<16), PR_LIGHT_PRIO_LOW_GAME);
                practor[spriteNum].lightcount = 2;
                break;
            case GROWSPARK__:
                {
                    vec3_t const offset = { ((sintable[(pSprite->ang+512)&2047])>>6), ((sintable[(pSprite->ang)&2047])>>6), LIGHTZOFF(spriteNum) };
                    G_AddGameLight(spriteNum, pSprite->sectnum, offset, LIGHTRAD(spriteNum), 0, 100,216+(52<<8)+(20<<16), PR_LIGHT_PRIO_HIGH_GAME);
                }
                break;
            case NEON1__:
            case NEON3__:
            case NEON4__:
            case NEON5__:
            case NEON6__:
                {
                    vec3_t const offset = { ((sintable[(pSprite->ang+512)&2047])>>6), ((sintable[(pSprite->ang)&2047])>>6), LIGHTZOFF(spriteNum) };
                    G_AddGameLight(spriteNum, pSprite->sectnum, offset, LIGHTRAD(spriteNum)>>2, 0, 100,216+(52<<8)+(20<<16), PR_LIGHT_PRIO_HIGH_GAME);
                }
                break;
            case LASERLINE__:
                {
                    G_AddGameLight(spriteNum, pSprite->sectnum, { 0, 0, LIGHTZOFF(spriteNum) }, 1280, 0, 100,216+(52<<8)+(20<<16), PR_LIGHT_PRIO_HIGH_GAME);
                    practor[spriteNum].lightcount = 2;
                }
                break;
            case SHRINKEREXPLOSION__:
                {
                    vec3_t const offset = { ((sintable[(pSprite->ang+512)&2047])>>6), ((sintable[(pSprite->ang)&2047])>>6), LIGHTZOFF(spriteNum) };
                    G_AddGameLight(spriteNum, pSprite->sectnum, offset, LIGHTRAD(spriteNum), 0, 100,176+(252<<8)+(120<<16), PR_LIGHT_PRIO_HIGH_GAME);
                }
                break;
            case NEON2__:
            case FREEZEBLAST__:
                G_AddGameLight(spriteNum, pSprite->sectnum, { 0, 0, LIGHTZOFF(spriteNum) }, LIGHTRAD(spriteNum)<<2, 0, 100,72+(88<<8)+(140<<16), PR_LIGHT_PRIO_HIGH_GAME);
                break;
            case REACTOR__:
            case REACTOR2__:
            case REACTORSPARK__:
            case REACTOR2SPARK__:
            case BOLT1__:
            case SIDEBOLT1__:
                G_AddGameLight(spriteNum, pSprite->sectnum, { 0, 0, LIGHTZOFF(spriteNum) }, LIGHTRAD(spriteNum), 0, 100,72+(88<<8)+(140<<16), PR_LIGHT_PRIO_HIGH_GAME);
                practor[spriteNum].lightcount = 2;
                break;
            case COOLEXPLOSION1__:
                G_AddGameLight(spriteNum, pSprite->sectnum, { 0, 0, LIGHTZOFF(spriteNum) }, LIGHTRAD(spriteNum)<<2, 0, 100,128+(0<<8)+(255<<16), PR_LIGHT_PRIO_HIGH_GAME);
                break;
            case SHRINKSPARK__:
            case CRYSTALAMMO__:
            case 679: // battlelord head thing
            case 490: // cycloid head thing
                G_AddGameLight(spriteNum, pSprite->sectnum, { 0, 0, LIGHTZOFF(spriteNum) }, LIGHTRAD(spriteNum), 0, 100,176+(252<<8)+(120<<16), PR_LIGHT_PRIO_HIGH_GAME);
                break;
            case FIRELASER__:
                if (pSprite->statnum == STAT_PROJECTILE)
                    G_AddGameLight(spriteNum, pSprite->sectnum, { 0, 0, LIGHTZOFF(spriteNum) }, 64 * pSprite->yrepeat, 0, 100,255+(95<<8), PR_LIGHT_PRIO_LOW_GAME);
                break;
            case RPG__:
                G_AddGameLight(spriteNum, pSprite->sectnum, { 0, 0, LIGHTZOFF(spriteNum) }, LIGHTRAD3(spriteNum)<<2, 0, 100,255+(95<<8), PR_LIGHT_PRIO_LOW_GAME);
                break;
            case SHOTSPARK1__:
                if (AC_ACTION_COUNT(actor[spriteNum].t_data) == 0) // check for first frame of action
                {
                    vec3_t const offset = { ((sintable[(pSprite->ang+512)&2047])>>6), ((sintable[(pSprite->ang)&2047])>>6), LIGHTZOFF(spriteNum) };
                    G_AddGameLight(spriteNum, pSprite->sectnum, offset, LIGHTRAD3(spriteNum)<<1, 0, 100,240+(160<<8)+(80<<16), PR_LIGHT_PRIO_LOW_GAME);
                    practor[spriteNum].lightcount = 1;
                }
                break;

            case RECON__:
            {
                vec3_t const offset = { 0, 0, -2048 };
                auto &a = actor[spriteNum];

                uint32_t color  = 255 + (255 << 8) + (255 << 16);
                int      radius = 256;
                int      range  = 8192;

                if (a.t_data[0] == 3)
                {
                    color = everyothertime & 1 ? 255 : 255 << 16;
                    radius <<= 1;
                    range <<= 1;
                }

                G_AddGameLight(spriteNum, pSprite->sectnum, offset, range, radius, (pSprite->xvel >> 2) - 100, color, PR_LIGHT_PRIO_HIGH_GAME);
                break;
            }
            case DRONE__:
            case TANK__:
            {
                vec3_t const offset = { 0, 0, 8192 };
                G_AddGameLight(spriteNum, pSprite->sectnum, offset, 4096, 256, 100, 255/*+(255<<8)+(255<<16)*/, PR_LIGHT_PRIO_HIGH_GAME);
                break;
            }
            case DOMELITE__:
            {
                vec3_t offset = { 0, 0, LIGHTZOFF(spriteNum)<<2 };
                if (pSprite->cstat & 8)
                    offset.z = -offset.z;
                pSprite->ang += 64;
                G_AddGameLight(spriteNum, pSprite->sectnum, offset, LIGHTRAD3(spriteNum)<<3, 384, 100, 255, PR_LIGHT_PRIO_LOW_GAME);
                break;
            }

            case FOOTPRINTS__:
            case FOOTPRINTS2__:
            case FOOTPRINTS3__:
            case FOOTPRINTS4__:
                if (pSprite->pal != 8)
                    break;
                fallthrough__;
            case BLOODPOOL__:
            {
                uint32_t color;

                switch (pSprite->pal)
                {
                case 1:
                    color = 72+(88<<8)+(140<<16);
                    break;
                case 0:
                case 8:
                    color = 172+(200<<8)+(104<<16);
                    break;
                default:
                    color = 0;
                    break;
                }

                if (color)
                    G_AddGameLight(spriteNum, pSprite->sectnum, { 0, 0, (pSprite->xrepeat<<4) }, (pSprite->xrepeat<<3), 0, 100, color, PR_LIGHT_PRIO_LOW);
            }
        }
#endif // EDUKE32_STANDALONE
    }
}
#endif // POLYMER

void A_PlayAlertSound(int spriteNum)
{
    if (sprite[spriteNum].extra > 0)
    {
        if ((VM_OnEventWithReturn(EVENT_RECOGSOUND, spriteNum, screenpeek, 0)) != 0)
            return;

#ifndef EDUKE32_STANDALONE
        if (FURY)
            return;

        switch (tileGetMapping(PN(spriteNum)))
        {
            case LIZTROOPONTOILET__:
            case LIZTROOPJUSTSIT__:
            case LIZTROOPSHOOT__:
            case LIZTROOPJETPACK__:
            case LIZTROOPDUCKING__:
            case LIZTROOPRUNNING__:
            case LIZTROOP__:         A_PlaySound(PRED_RECOG, spriteNum); break;
            case LIZMAN__:
            case LIZMANSPITTING__:
            case LIZMANFEEDING__:
            case LIZMANJUMP__:       A_PlaySound(CAPT_RECOG, spriteNum); break;
            case PIGCOP__:
            case PIGCOPDIVE__:       A_PlaySound(PIG_RECOG, spriteNum); break;
            case RECON__:            A_PlaySound(RECO_RECOG, spriteNum); break;
            case DRONE__:            A_PlaySound(DRON_RECOG, spriteNum); break;
            case COMMANDER__:
            case COMMANDERSTAYPUT__: A_PlaySound(COMM_RECOG, spriteNum); break;
            case ORGANTIC__:         A_PlaySound(TURR_RECOG, spriteNum); break;
            case OCTABRAIN__:
            case OCTABRAINSTAYPUT__: A_PlaySound(OCTA_RECOG, spriteNum); break;
            case BOSS1__:
            case BOSS1STAYPUT__:     S_PlaySound(BOS1_RECOG); break;
            case BOSS2__:            S_PlaySound((sprite[spriteNum].pal != 0) ? BOS2_RECOG : WHIPYOURASS); break;
            case BOSS3__:            S_PlaySound((sprite[spriteNum].pal != 0) ? BOS3_RECOG : RIPHEADNECK); break;
            case BOSS4__:
            case BOSS4STAYPUT__:     S_PlaySound((sprite[spriteNum].pal != 0) ? BOS4_RECOG : BOSS4_FIRSTSEE); break;
            case GREENSLIME__:       A_PlaySound(SLIM_RECOG, spriteNum); break;
        }
#endif
    }
}

int A_CheckSwitchTile(int spriteNum)
{
    UNREFERENCED_PARAMETER(spriteNum);

#ifndef EDUKE32_STANDALONE
    if (FURY)
        return 0;

    // picnum 0 would oob in the switch below,

    if (PN(spriteNum) <= 0)
        return 0;

    // MULTISWITCH has 4 states so deal with it separately,
    // ACCESSSWITCH and ACCESSSWITCH2 are only active in one state so deal with
    // them separately.

    if ((PN(spriteNum) >= MULTISWITCH && PN(spriteNum) <= MULTISWITCH + 3) || (PN(spriteNum) == ACCESSSWITCH || PN(spriteNum) == ACCESSSWITCH2))
        return 1;

    // Loop to catch both states of switches.
    for (bssize_t j=1; j>=0; j--)
    {
        switch (tileGetMapping(PN(spriteNum)-j))
        {
        case HANDPRINTSWITCH__:
        case ALIENSWITCH__:
        case MULTISWITCH__:
        case PULLSWITCH__:
        case HANDSWITCH__:
        case SLOTDOOR__:
        case LIGHTSWITCH__:
        case SPACELIGHTSWITCH__:
        case SPACEDOORSWITCH__:
        case FRANKENSTINESWITCH__:
        case LIGHTSWITCH2__:
        case POWERSWITCH1__:
        case LOCKSWITCH1__:
        case POWERSWITCH2__:
        case DIPSWITCH__:
        case DIPSWITCH2__:
        case TECHSWITCH__:
        case DIPSWITCH3__:
            return 1;
        }
    }
#endif
    return 0;
}

void G_RefreshLights(void)
{
#ifdef POLYMER
    if (!Numsprites || videoGetRenderMode() != REND_POLYMER)
        return;

    int statNum = 0;
    savedFires = 0;
    do
    {
        for (int SPRITES_OF(statNum++, spriteNum))
            A_DoLight(spriteNum);
    }
    while (statNum < MAXSTATUS);
#endif
}

// Remainder of G_RecordOldSpritePos()
static FORCE_INLINE void G_RecordOldSpritePosForStatnum(int const statNum)
{
    for (int SPRITES_OF(statNum, spriteNum))
        actor[spriteNum].bpos = sprite[spriteNum].xyz;
}

static void G_RecordOldSpritePos(void)
{
    int statNum = 0;
    do
    {
        // Delay until a later point. Fixes a problem where SE7 and Touchplates cannot be activated concurrently.
        if ((statNum == STAT_PLAYER) | (statNum == STAT_EFFECTOR))
        {
            statNum++;
            continue;
        }

        G_RecordOldSpritePosForStatnum(statNum++);
    }
    while (statNum < MAXSTATUS);
}

static void G_DoEventGame(int const nEventID, bool const allowDrawing = true)
{
    if (!VM_HaveEvent(nEventID))
        return;

    int statNum = 0;
    do
    {
        for (int nextSprite, SPRITES_OF_STAT_SAFE(statNum++, spriteNum, nextSprite))
        {
            if (A_CheckSpriteFlags(spriteNum, SFLAG_NOEVENTCODE))
                continue;

            int32_t   playerDist;
            int const playerNum = A_FindPlayer(&sprite[spriteNum], &playerDist);
            VM_ExecuteEvent(nEventID, spriteNum, playerNum, playerDist);

            if (allowDrawing)
                dukeMaybeDrawFrame();
        }
    }
    while (statNum < MAXSTATUS);
}

void G_MoveWorld(void)
{
    Bassert(mco_running() != co_drawframe);

    double worldTime = timerGetFractionalTicks();
    auto framecnt = g_frameCounter;

    MICROPROFILE_SCOPEI("Game", "MoveWorld", MP_YELLOW);

    VM_OnEvent(EVENT_PREWORLD);
    G_DoEventGame(EVENT_PREGAME, false);
    G_RecordOldSpritePos();

    {
        MICROPROFILE_SCOPEI("MoveWorld", "MoveZombieActors", MP_YELLOW2);
        G_MoveZombieActors();  //ST 2
    }

    {
        MICROPROFILE_SCOPEI("MoveWorld", "MoveWeapons", MP_YELLOW3);
        G_MoveWeapons();  //ST 4
    }

    {
        MICROPROFILE_SCOPEI("MoveWorld", "MoveTransports", MP_YELLOW4);
        G_MoveTransports();  //ST 9
    }

    {
        MICROPROFILE_SCOPEI("MoveWorld", "MovePlayers", MP_YELLOW);
        G_MovePlayers();  //ST 10
    }

    // Must be called here to fix a problem where SE7 Transports and Touchplates do not activate concurrently
    G_RecordOldSpritePosForStatnum(STAT_PLAYER);

    {
        MICROPROFILE_SCOPEI("MoveWorld", "MoveFallers", MP_YELLOW2);
        G_MoveFallers();  //ST 12
    }

    {
        MICROPROFILE_SCOPEI("MoveWorld", "MoveMisc", MP_YELLOW3);
        G_MoveMisc();  //ST 5
    }

    double actorsTime = timerGetFractionalTicks();
    auto framecnt2 = g_frameCounter;

    {
        MICROPROFILE_SCOPEI("MoveWorld", "MoveActors", MP_YELLOW4);
        G_MoveActors();  //ST 1
    }

    actorsTime = timerGetFractionalTicks() - actorsTime;

    if (framecnt2 != g_frameCounter)
        actorsTime -= (double)g_lastFrameDuration2 * 1000.0 / (double)timerGetNanoTickRate();

    g_moveActorsTime = (1-0.033)*g_moveActorsTime + 0.033*actorsTime;

    // XXX: Has to be before effectors, in particular movers?
    // TODO: lights in moving sectors ought to be interpolated
#ifdef POLYMER
    G_DoEffectorLights();
#endif

    {
        MICROPROFILE_SCOPEI("MoveWorld", "MoveEffectors", MP_YELLOW);
        G_MoveEffectors();  //ST 3
    }

    {
        MICROPROFILE_SCOPEI("MoveWorld", "MoveStandables", MP_YELLOW2);
        G_MoveStandables();  //ST 6
    }


    VM_OnEvent(EVENT_WORLD);

    G_DoEventGame(EVENT_GAME);

    G_RefreshLights();
    G_DoSectorAnimations();

    {
        MICROPROFILE_SCOPEI("MoveWorld", "MoveFX", MP_YELLOW3);
        G_MoveFX();  //ST 11
    }

    worldTime = timerGetFractionalTicks() - worldTime;

    if (g_frameCounter != framecnt)
        worldTime -= (double)g_lastFrameDuration2 * 1000.0 / (double)timerGetNanoTickRate();

    g_moveWorldTime = (1-0.033)*g_moveWorldTime + 0.033*worldTime;
}
