#include "compat.h"
#include "build.h"
#include "../duke3d.h"

int rt_pfade;

//////////////////// HUD WEAPON / MISC. DISPLAY CODE ////////////////////

int RT_P_GetHudPal(const DukePlayer_t *p)
{
    if (p->cursectnum >= 0)
    {
        if (sector[p->cursectnum].lotag == ST_2_UNDERWATER)
            return 1;
        int const hudPal = sector[p->cursectnum].floorpal;
        if (!g_noFloorPal[hudPal])
            return hudPal;
    }

    return 0;
}

static void RT_P_DisplaySpit(void)
{
    DukePlayer_t *const pPlayer     = g_player[screenpeek].ps;
    int const           loogCounter = pPlayer->loogcnt;

    if (loogCounter == 0)
        return;

    int const rotY = loogCounter<<2;

    for (bssize_t i=0; i < pPlayer->numloogs; i++)
    {
        int const rotAng = klabs(sintable[((loogCounter + i) << 5) & 2047]) >> 5;
        int const rotZoom  = (4096 + ((loogCounter + i) << 9) - (i<<8))*100/65536;
        int const rotX     = (-fix16_to_int(g_player[screenpeek].inputBits->q16avel) >> 1) + (sintable[((loogCounter + i) << 6) & 2047] >> 10);

        RT_RotateSprite(pPlayer->loogiex[i] + rotX, 200 + pPlayer->loogiey[i] - rotY, rotZoom, rotZoom, LOOGIE, 0);
    }
}

static int RT_P_DisplayFist(int const fistShade)
{
    DukePlayer_t const *const pPlayer = g_player[screenpeek].ps;
    int fistInc = pPlayer->fist_incs;

    if (fistInc > 32)
        fistInc = 32;

    if (fistInc <= 0)
        return 0;

    int const fistY       = klabs(pPlayer->look_ang) / 9;
    int       fistZoom    = min(65536 - (sintable[(512 + (fistInc << 6)) & 2047] << 2), 90612);
    if (fistZoom < 40920) fistZoom = 40290;
    int const fistYOffset = 214 + (sintable[((6 + fistInc) << 7) & 2047] >> 9);

#ifdef SPLITSCREEN_MOD_HACKS
    // XXX: this is outdated, doesn't handle above/below split.
    if (g_fakeMultiMode==2)
        wx[(g_snum==0)] = (wx[0]+wx[1])/2+1;
#endif

    RT_RotateSprite(-fistInc + 222 + (fix16_to_int(g_player[screenpeek].inputBits->q16avel) >> 3), fistY + fistYOffset,
                 (fistZoom*100)/65536, (fistZoom*100)/65536, FIST, 0);

    return 1;
}

#define DRAWEAP_CENTER 262144
#define weapsc(sc) scale(sc, ud.weaponscale, 100)

static int32_t g_dts_yadd;

static void RT_DrawTileScaled(int drawX, int drawY, int tileNum, int drawShade, int drawBits, int drawPal,
    int drawScale = 65536, int angleOffset = 0)
{
    if (drawPal == 255)
        RT_RotateSpriteSetColor(255, 255, 255, 666);
    else
        RT_RotateSpriteSetShadePal(screenpeek, drawShade, drawPal);

    RT_RotateSprite(drawX, drawY, 100.f, 100.f, tileNum, drawBits);
#if 0
    int32_t wx[2] = { windowxy1.x, windowxy2.x };
    int32_t wy[2] = { windowxy1.y, windowxy2.y };

    int drawYOffset = 0;
    int drawXOffset = 192<<16;

    switch (DYNAMICWEAPONMAP(hudweap.cur))
    {
        case DEVISTATOR_WEAPON__STATIC:
        case TRIPBOMB_WEAPON__STATIC:
            drawXOffset = 160<<16;
            break;
        default:
            if (drawBits & DRAWEAP_CENTER)
            {
                drawXOffset = 160<<16;
                drawBits &= ~DRAWEAP_CENTER;
            }
            break;
    }

    // bit 4 means "flip x" for G_DrawTileScaled
    int const drawAng = ((drawBits & 4) ? 1024 : 0) + angleOffset;

#ifdef SPLITSCREEN_MOD_HACKS
    if (g_fakeMultiMode==2)
    {
        int const sideBySide = (ud.screen_size!=0);

        // splitscreen HACK
        drawBits &= ~(1024|512|256);
        if (sideBySide)
        {
            drawBits &= ~8;
            wx[(g_snum==0)] = (wx[0]+wx[1])/2 + 2;
        }
        else
        {
            drawBits |= 8;
            if (g_snum==0)
                drawYOffset = -(100<<16);
            wy[(g_snum==0)] = (wy[0]+wy[1])/2 + 2;
        }
    }
#endif

#ifdef USE_OPENGL
    if (videoGetRenderMode() >= REND_POLYMOST && usemodels && md_tilehasmodel(tileNum,drawPal) >= 0)
        drawYOffset += (224<<16)-weapsc(224<<16);
#endif
    drawY -= 24;
    rotatesprite(weapsc(drawX<<16) + (drawXOffset-weapsc(drawXOffset)),
                 weapsc((drawY<<16) + g_dts_yadd) + ((200<<16)-weapsc(200<<16)) + drawYOffset,
                 weapsc(drawScale),drawAng,tileNum,drawShade,drawPal,(2|drawBits),
                 wx[0],wy[0], wx[1],wy[1]);
#endif
}

static inline void RT_DrawWeaponTileWithID(int uniqueID, int weaponX, int weaponY, int weaponTile, int weaponShade,
                                          int weaponBits, int p, int weaponScale = 65536)
{
    int lastUniqueID = guniqhudid;
    guniqhudid       = uniqueID;

    RT_DrawTileScaled(weaponX, weaponY, weaponTile, weaponShade, weaponBits, p, weaponScale);

    guniqhudid       = lastUniqueID;
}

static int RT_P_DisplayKnee(int kneeShade)
{
    static int8_t const       knee_y[] = { 0, -8, -16, -32, -64, -84, -108, -108, -108, -72, -32, -8 };
    const DukePlayer_t *const ps = g_player[screenpeek].ps;

    if (ps->knee_incs == 0)
        return 0;

    if (ps->knee_incs >= ARRAY_SIZE(knee_y) || sprite[ps->i].extra <= 0)
        return 0;

    int const kneeY   = knee_y[ps->knee_incs] + (klabs(ps->look_ang) / 9) - (ps->hard_landing << 3);
    int const kneePal = RT_P_GetHudPal(ps);

    RT_DrawTileScaled(105+(fix16_to_int(g_player[screenpeek].inputBits->q16avel)>>5)-(ps->look_ang>>1)+(knee_y[ps->knee_incs]>>2),
                     kneeY+331-(fix16_to_int(ps->q16horiz-ps->q16horizoff)>>4),KNEE,kneeShade,4+DRAWEAP_CENTER,kneePal);

    return 1;
}

static int RT_P_DisplayKnuckles(int knuckleShade)
{
    const DukePlayer_t *const pPlayer = g_player[screenpeek].ps;

    if (pPlayer->knuckle_incs == 0)
        return 0;

    static int8_t const knuckleFrames[] = { 0, 1, 2, 2, 3, 3, 3, 2, 2, 1, 0 };

    if ((unsigned) (pPlayer->knuckle_incs>>1) >= ARRAY_SIZE(knuckleFrames) || sprite[pPlayer->i].extra <= 0)
        return 0;

    int const knuckleY   = (klabs(pPlayer->look_ang) / 9) - (pPlayer->hard_landing << 3);
    int const knucklePal = RT_P_GetHudPal(pPlayer);

    RT_DrawTileScaled(160 + (fix16_to_int(g_player[screenpeek].inputBits->q16avel) >> 5) - (pPlayer->look_ang >> 1),
                     knuckleY + 180 - (fix16_to_int(pPlayer->q16horiz - pPlayer->q16horizoff) >> 4),
                     CRACKKNUCKLES + knuckleFrames[pPlayer->knuckle_incs >> 1], knuckleShade, 4 + DRAWEAP_CENTER,
                     knucklePal);

    return 1;
}

void RT_P_DisplayScuba(void)
{
    if (g_player[screenpeek].ps->scuba_on)
    {
        const DukePlayer_t *const pPlayer = g_player[screenpeek].ps;

        int const scubaPal = RT_P_GetHudPal(pPlayer);

#ifdef SPLITSCREEN_MOD_HACKS
        g_snum = screenpeek;
#endif
        RT_DisablePolymost();

        RT_DrawTileScaled(103, 208, SCUBAMASK, 0, 0, scubaPal);
        RT_DrawTileScaled(217, 208, SCUBAMASK, 0, 4, scubaPal);

        RT_EnablePolymost();
    }
}

static int8_t const access_tip_y [] = {
    0, -8, -16, -32, -64, -84, -108, -108, -108, -108, -108, -108, -108, -108, -108, -108, -96, -72, -64, -32, -16
};

static int RT_P_DisplayTip(int tipShade)
{
    const DukePlayer_t *const pPlayer = g_player[screenpeek].ps;

    if (pPlayer->tipincs == 0)
        return 0;

    // Report that the tipping hand has been drawn so that the otherwise
    // selected weapon is not drawn.
    if ((unsigned)pPlayer->tipincs >= ARRAY_SIZE(access_tip_y))
        return 1;

    int const tipY       = (klabs(pPlayer->look_ang) / 9) - (pPlayer->hard_landing << 3);
    int const tipPal     = RT_P_GetHudPal(pPlayer);
    int const tipYOffset = access_tip_y[pPlayer->tipincs] >> 1;

    guniqhudid = 201;

    RT_DrawTileScaled(170 + (fix16_to_int(g_player[screenpeek].inputBits->q16avel) >> 5) - (pPlayer->look_ang >> 1),
                     tipYOffset + tipY + 240 - (fix16_to_int(pPlayer->q16horiz - pPlayer->q16horizoff) >> 4),
                     TIP + ((26 - pPlayer->tipincs) >> 4), tipShade, DRAWEAP_CENTER, tipPal);

    guniqhudid = 0;

    return 1;
}

static int RT_P_DisplayAccess(int accessShade)
{
    const DukePlayer_t *const pSprite = g_player[screenpeek].ps;

    if (pSprite->access_incs == 0)
        return 0;

    if ((unsigned)pSprite->access_incs >= ARRAY_SIZE(access_tip_y) || sprite[pSprite->i].extra <= 0)
        return 1;

    int const accessX   = access_tip_y[pSprite->access_incs] >> 2;
    int const accessY   = access_tip_y[pSprite->access_incs] + (klabs(pSprite->look_ang) / 9) - (pSprite->hard_landing << 3);
    int const accessPal = RT_P_GetHudPal(pSprite);

    guniqhudid = 200;

    if ((pSprite->access_incs - 3) > 0 && (pSprite->access_incs - 3) >> 3)
    {
        RT_DrawTileScaled(170 + (fix16_to_int(g_player[screenpeek].inputBits->q16avel) >> 5) - (pSprite->look_ang >> 1) + accessX,
                         accessY + 266 - (fix16_to_int(pSprite->q16horiz - pSprite->q16horizoff) >> 4),
                         HANDHOLDINGLASER + (pSprite->access_incs >> 3), accessShade, DRAWEAP_CENTER, accessPal);
    }
    else
    {
        int accessPal = 0;
        if (pSprite->access_spritenum >= 0)
            accessPal = sprite[pSprite->access_spritenum].pal;
        int accessTile = 0;
        if (accessPal == 0)
            accessTile = DN64TILE3715;
        else if (accessPal == 21)
            accessTile = DN64TILE3713;
        else if (accessPal == 23)
            accessTile = DN64TILE3714;
        RT_DrawTileScaled(170 + (fix16_to_int(g_player[screenpeek].inputBits->q16avel) >> 5) - (pSprite->look_ang >> 1) + accessX,
                         accessY + 279 - (fix16_to_int(pSprite->q16horiz - pSprite->q16horizoff) >> 4), DN64TILE3716, accessShade,
                         4 + DRAWEAP_CENTER, accessPal);
        RT_DrawTileScaled(170 + (fix16_to_int(g_player[screenpeek].inputBits->q16avel) >> 5) - (pSprite->look_ang >> 1) + accessX,
                         accessY + 279 - (fix16_to_int(pSprite->q16horiz - pSprite->q16horizoff) >> 4), accessTile, accessShade,
                         4 + DRAWEAP_CENTER, accessPal);
    }

    guniqhudid = 0;

    return 1;
}

/*

KNEE= 0,
PISTOL = 1
SHOTGUN = 2
CHAINGUN = 3
GRENADE = 4
HANDBOMB = 5
SHRINKER = 6
GROW = 7
RPG = 8
TRIPBOMB = 9
FREEZE = 10
HANDREMOTE = 11
PISTOL2 = 12
SHOTGUN2 = 13
RPG2 = 14

*/

void RT_P_DisplayWeapon(void)
{
    DukePlayer_t *const  pPlayer     = g_player[screenpeek].ps;
    const int16_t *const weaponFrame = &pPlayer->kickback_pic;

    int currentWeapon, quickKickFrame;
    int v2;

#ifdef SPLITSCREEN_MOD_HACKS
    g_snum = screenpeek;
#endif

    if (pPlayer->newowner >= 0 || ud.camerasprite >= 0 || pPlayer->over_shoulder_on > 0
        || (sprite[pPlayer->i].pal != 1 && sprite[pPlayer->i].extra <= 0))
        return;

    RT_DisablePolymost();

    int weaponX       = (160) - 90;
    int weaponY       = klabs(pPlayer->look_ang) / 9;
    int weaponYOffset = 60 - (pPlayer->weapon_pos * pPlayer->weapon_pos);
    int weaponShade   = sprite[pPlayer->i].shade <= 24 ? sprite[pPlayer->i].shade : 24;
    int weaponPal     = RT_P_GetHudPal(pPlayer);

    int32_t weaponBits = 0;
    UNREFERENCED_PARAMETER(weaponBits);

    if (RT_P_DisplayFist(weaponShade) || RT_P_DisplayKnuckles(weaponShade) || RT_P_DisplayTip(weaponShade) || RT_P_DisplayAccess(weaponShade))
        goto enddisplayweapon;

    RT_P_DisplayKnee(weaponShade);

    if (ud.weaponsway)
    {
        weaponX -= (sintable[((pPlayer->weapon_sway>>1)+512)&2047]/(1024+512));
        weaponYOffset -= (sprite[pPlayer->i].xrepeat < 32) ? klabs(sintable[(pPlayer->weapon_sway << 2) & 2047] >> 9)
                                                           : klabs(sintable[(pPlayer->weapon_sway >> 1) & 2047] >> 11);
    }
    else weaponYOffset -= 16;

    weaponX -= 58 + pPlayer->weapon_ang;
    weaponYOffset -= (pPlayer->hard_landing << 3);

    currentWeapon   = (pPlayer->last_weapon >= 0) ? pPlayer->last_weapon : pPlayer->curr_weapon;
    hudweap.gunposy     = weaponYOffset;
    hudweap.lookhoriz   = weaponY;
    hudweap.cur         = currentWeapon;
    hudweap.gunposx     = weaponX;
    hudweap.shade       = weaponShade;
    hudweap.count       = *weaponFrame;
    hudweap.lookhalfang = pPlayer->look_ang >> 1;

    weaponX += currentWeapon != TRIPBOMB_WEAPON ? (pPlayer->weapon_pos * pPlayer->weapon_pos) / 2 : 0;

    quickKickFrame = 14 - pPlayer->quick_kick;

    if (quickKickFrame != 14 && ud.drawweapon == 1)
    {
        guniqhudid = 100;

        if (quickKickFrame < 5 || quickKickFrame > 9)
            RT_DrawTileScaled(weaponX + 80 - (pPlayer->look_ang >> 1), weaponY + 250 - weaponYOffset, KNEE, weaponShade,
                                weaponBits | 4 | DRAWEAP_CENTER, weaponPal);
        else
            RT_DrawTileScaled(weaponX + 160 - 16 - (pPlayer->look_ang >> 1), weaponY + 214 - weaponYOffset, KNEE + 1,
                                weaponShade, weaponBits | 4 | DRAWEAP_CENTER, weaponPal);
        guniqhudid = 0;
    }

    if (sprite[pPlayer->i].xrepeat < 40)
    {
        static int32_t fistPos;

        if (pPlayer->jetpack_on == 0)
        {
            int const playerXvel = sprite[pPlayer->i].xvel;
            weaponY += 32 - (playerXvel >> 3);
            fistPos += playerXvel >> 3;
        }

        currentWeapon = weaponX;
        weaponX += sintable[(fistPos)&2047] >> 10;
        RT_DrawTileScaled(weaponX + 250 - (pPlayer->look_ang >> 1), weaponY + 258 - (klabs(sintable[(fistPos)&2047] >> 8)),
            FIST, weaponShade, weaponBits, weaponPal);
        weaponX = currentWeapon - (sintable[(fistPos)&2047] >> 10);
        RT_DrawTileScaled(weaponX + 40 - (pPlayer->look_ang >> 1), weaponY + 200 + (klabs(sintable[(fistPos)&2047] >> 8)), FIST,
            weaponShade, weaponBits | 4, weaponPal);
    }
    else
    {
        switch (ud.drawweapon)
        {
            case 1: break;
            case 2:
                if ((unsigned)hudweap.cur < MAX_WEAPONS && hudweap.cur != KNEE_WEAPON)
                    rotatesprite_win(160 << 16, (180 + (pPlayer->weapon_pos * pPlayer->weapon_pos)) << 16, divscale16(ud.statusbarscale, 100), 0,
                                        (hudweap.cur == GROW_WEAPON) ? GROWSPRITEICON : WeaponPickupSprites[hudweap.cur], 0,
                                        0, 2);
            default: goto enddisplayweapon;
        }

        if (currentWeapon == KNEE_WEAPON && *weaponFrame == 0)
            goto enddisplayweapon;

        int const doAnim      = !(sprite[pPlayer->i].pal == 1 || ud.pause_on || g_player[myconnectindex].ps->gm & MODE_MENU);
        int const halfLookAng = pPlayer->look_ang >> 1;

        switch (DYNAMICWEAPONMAP(currentWeapon))
        {
        case KNEE_WEAPON__STATIC:
        {
            guniqhudid = currentWeapon;
            if (*weaponFrame < 5 || *weaponFrame > 9)
                RT_DrawTileScaled(weaponX + 260 - halfLookAng, weaponY + 321 - weaponYOffset, KNEE,
                                    weaponShade, weaponBits, weaponPal);
            else
                RT_DrawTileScaled(weaponX + 200 - halfLookAng, weaponY + 285 - weaponYOffset, KNEE + 1,
                                    weaponShade, weaponBits, weaponPal);
            guniqhudid = 0;
            break;
        }

        case TRIPBOMB_WEAPON__STATIC:
            weaponX += 8;
            weaponYOffset -= 10;

            if ((*weaponFrame) > 6)
                weaponY += ((*weaponFrame) << 3);
            else if ((*weaponFrame) < 4)
                RT_DrawWeaponTileWithID(currentWeapon << 2, weaponX + 142 - halfLookAng,
                                        weaponY + 237 - weaponYOffset, HANDHOLDINGLASER + 3, weaponShade, weaponBits, weaponPal);

            RT_DrawWeaponTileWithID(currentWeapon, weaponX + 130 - halfLookAng, weaponY + 249 - weaponYOffset,
                                    HANDHOLDINGLASER + ((*weaponFrame) >> 2), weaponShade, weaponBits, weaponPal);

            RT_DrawWeaponTileWithID(currentWeapon << 1, weaponX + 152 - halfLookAng,
                                    weaponY + 249 - weaponYOffset, HANDHOLDINGLASER + ((*weaponFrame) >> 2), weaponShade, weaponBits | 4,
                                    weaponPal);
            break;

        case RPG_WEAPON__STATIC:
        case BOAT_WEAPON__STATIC:
            if (*weaponFrame < 8)
            {
                weaponX -= sintable[(1024 + ((*weaponFrame) << 7)) & 2047] >> 10;
                weaponYOffset += sintable[(1024 + ((*weaponFrame) << 7)) & 2047] >> 11;
            }

            //if (!(duke3d_globalflags & DUKE3D_NO_WIDESCREEN_PINNING))
            //    weaponBits |= 512;

            RT_DrawWeaponTileWithID(currentWeapon << 2, weaponX + 249, (weaponY << 1) + 190 - weaponYOffset, RPGGUN+1, weaponShade,
                                    weaponBits, weaponPal);

            if (*weaponFrame > 0)
            {
                RT_DrawWeaponTileWithID(currentWeapon << 1, weaponX + 249 + max(79 - ((*weaponFrame) << 1), 47),
                                        (weaponY << 1) + 176 - weaponYOffset + max(0, *weaponFrame * 3 - 60),
                                        (currentWeapon == RPG_WEAPON) ? 3792 : 3789, weaponShade, weaponBits, weaponPal);
            }

            RT_DrawWeaponTileWithID(currentWeapon, weaponX + 249, (weaponY << 1) + 189 - weaponYOffset, RPGGUN, weaponShade,
                                    weaponBits, weaponPal);

            if (*weaponFrame == 0)
            {
                if (currentWeapon == RPG_WEAPON)
                    RT_DrawWeaponTileWithID(currentWeapon << 3, weaponX + 246, (weaponY << 1) + 226 - weaponYOffset,
                                            3790, -32, weaponBits, weaponPal);
                else
                    RT_DrawWeaponTileWithID(currentWeapon << 3, weaponX + 255, (weaponY << 1) + 230 - weaponYOffset,
                                            3791, -32, weaponBits, weaponPal);
            }
            break;

        case SHOTGUN_WEAPON__STATIC:
        case MOTORCYCLE_WEAPON__STATIC:
            weaponX -= 8;

            if (*weaponFrame > 3 && *weaponFrame < 8)
            {
                weaponYOffset -= 15;
                weaponX += 7;
            }

            switch (*weaponFrame)
            {
                case 0:
                case 1:
                case 2:
                case 3:
                case 4:
                case 5:
                case 6:
                case 7:
                case 8:
                    RT_DrawWeaponTileWithID(currentWeapon, weaponX + 305 - halfLookAng, weaponY + 285 - weaponYOffset,
                                            SHOTGUN, weaponShade, weaponBits, weaponPal);
                    break;
                case 9:
                case 10:
                case 11:
                case 12:
                    RT_DrawWeaponTileWithID(currentWeapon, weaponX + 305 - halfLookAng, weaponY + 285 - weaponYOffset,
                                            SHOTGUN + 3, weaponShade, weaponBits, weaponPal);
                    break;
                case 13:
                case 14:
                case 15:
                    RT_DrawWeaponTileWithID(currentWeapon, weaponX + 305 - halfLookAng, weaponY + 285 - weaponYOffset,
                                            SHOTGUN + 4, weaponShade, weaponBits, weaponPal);
                    break;
                case 16:
                case 17:
                case 18:
                    RT_DrawWeaponTileWithID(currentWeapon, weaponX + 305 - halfLookAng, weaponY + 285 - weaponYOffset,
                                            SHOTGUN + 5, weaponShade, weaponBits, weaponPal);
                    break;
                case 19:
                case 20:
                case 21:
                case 22:
                case 23:
                case 24:
                    RT_DrawWeaponTileWithID(currentWeapon, weaponX + 305 - halfLookAng, weaponY + 285 - weaponYOffset,
                                            SHOTGUN + 6, weaponShade, weaponBits, weaponPal);
                    break;
                case 25:
                case 26:
                case 27:
                    RT_DrawWeaponTileWithID(currentWeapon, weaponX + 305 - halfLookAng, weaponY + 285 - weaponYOffset,
                                            SHOTGUN + 5, weaponShade, weaponBits, weaponPal);
                    break;
                case 28:
                case 29:
                case 30:
                    RT_DrawWeaponTileWithID(currentWeapon, weaponX + 305 - halfLookAng, weaponY + 285 - weaponYOffset,
                        SHOTGUN + 4, weaponShade, weaponBits, weaponPal);
                    break;
            }
            break;

        case CHAINGUN_WEAPON__STATIC:
        {
            int oWeaponY = weaponYOffset;
            if (*weaponFrame > 0 && (RT_FakeKRand() & 255) < 127)
            {
                weaponYOffset -= klabs(sintable[(*weaponFrame)<<7]>>12);

                if (doAnim)
                    weaponX += (RT_FakeKRand()&3);
                
                float siz = ((RT_FakeKRand() & 255) * (1.f/256.f) + 0.7);

                RT_DrawTileFlash(weaponX + 215 - halfLookAng, weaponY + 205 - weaponYOffset, 0xf4f, siz, siz, 0, 12);
            }

            RT_DrawWeaponTileWithID(currentWeapon, weaponX + 235 - (pPlayer->look_ang >> 1), weaponY + 265 - weaponYOffset,
                                    CHAINGUN, weaponShade, weaponBits, weaponPal);

            weaponYOffset = oWeaponY;

            if (*weaponFrame > 0 && (RT_FakeKRand() & 255) < 127)
            {
                weaponYOffset -= klabs(sintable[(((*weaponFrame)<<7)+512)&2047]>>12);

                if (doAnim)
                    weaponX -= (RT_FakeKRand()&3);
                
                float siz = ((RT_FakeKRand() & 255) * (1.f/256.f) + 0.7);

                RT_DrawTileFlash(-weaponX + 105 - halfLookAng, weaponY + 205 - weaponYOffset, 0xf4f, siz, siz, 4, 12);
            }
            RT_DrawWeaponTileWithID(currentWeapon<<1, -weaponX + 85 - (pPlayer->look_ang >> 1), weaponY + 265 - weaponYOffset,
                                    CHAINGUN, weaponShade, weaponBits|4, weaponPal);
            break;
        }

        case PISTOL_WEAPON__STATIC:
        case BOWLINGBALL_WEAPON__STATIC:
            if ((*weaponFrame) < 5)
            {
                static uint8_t pistolFrames[] = { 0, 1, 1 };
                int pistolOffset = 165+weaponX;

                if ((*weaponFrame) == 2)
                    pistolOffset += 6;
                
                if (pistolFrames[*weaponFrame > 2 ? 0 : *weaponFrame])
                {
                    float siz = ((RT_FakeKRand() & 127) * (1.f/256.f) + 0.75);

                    RT_DrawTileFlash(pistolOffset + 30 - halfLookAng, weaponY + 205 - weaponYOffset, 0xf4f, siz, siz, 0, 1);
                }

                RT_DrawWeaponTileWithID(currentWeapon, (pistolOffset - (pPlayer->look_ang >> 1))+115, (weaponY + 285 - weaponYOffset),
                                        FIRSTGUN + pistolFrames[*weaponFrame > 2 ? 0 : *weaponFrame], weaponShade, 0,
                                        weaponPal);

                break;
            }

            //if (!(duke3d_globalflags & DUKE3D_NO_WIDESCREEN_PINNING) && DUKE)
            //    weaponBits |= 512;

            if ((*weaponFrame) < 10)
                RT_DrawWeaponTileWithID(currentWeapon, 309 - (pPlayer->look_ang >> 1), weaponY + 291 - weaponYOffset, FIRSTGUN + 4,
                                        weaponShade, weaponBits, weaponPal);
            else if ((*weaponFrame) < 15)
            {
                RT_DrawWeaponTileWithID(currentWeapon << 1, 284 - ((*weaponFrame) << 3) - (pPlayer->look_ang >> 1),
                                        weaponY + 100 - weaponYOffset + ((*weaponFrame) << 4), FIRSTGUN + 6, weaponShade,
                                        weaponBits, weaponPal);
                RT_DrawWeaponTileWithID(currentWeapon, 298 - (pPlayer->look_ang >> 1), weaponY + 284 - weaponYOffset, FIRSTGUN + 5,
                                        weaponShade, weaponBits, weaponPal);
            }
            else if ((*weaponFrame) < 20)
            {
                RT_DrawWeaponTileWithID(currentWeapon << 2, 154 + ((*weaponFrame) << 1) - (pPlayer->look_ang >> 1),
                                        weaponY + 420 - weaponYOffset - ((*weaponFrame) << 3), FIRSTGUN + 8, weaponShade,
                                        weaponBits, weaponPal);
                RT_DrawWeaponTileWithID(currentWeapon << 1, 174 + ((*weaponFrame) << 1) - (pPlayer->look_ang >> 1),
                                        weaponY + 400 - weaponYOffset - ((*weaponFrame) << 3), FIRSTGUN + 6, weaponShade,
                                        weaponBits, weaponPal);
                RT_DrawWeaponTileWithID(currentWeapon, 298 - (pPlayer->look_ang >> 1), weaponY + 284 - weaponYOffset, FIRSTGUN + 5,
                                        weaponShade, weaponBits, weaponPal);
            }

            else if ((*weaponFrame) < 23)
            {
                RT_DrawWeaponTileWithID(currentWeapon << 2, 184 - (pPlayer->look_ang >> 1), weaponY + 245 - weaponYOffset,
                                        FIRSTGUN + 8, weaponShade, weaponBits, weaponPal);
                RT_DrawWeaponTileWithID(currentWeapon, 298 - (pPlayer->look_ang >> 1), weaponY + 284 - weaponYOffset, FIRSTGUN + 5,
                                        weaponShade, weaponBits, weaponPal);
            }
            else if ((*weaponFrame) < 25)
            {
                RT_DrawWeaponTileWithID(currentWeapon << 2, 204 - (pPlayer->look_ang >> 1), weaponY + 245 - weaponYOffset,
                                        FIRSTGUN + 8, weaponShade, weaponBits, weaponPal);
                RT_DrawWeaponTileWithID(currentWeapon, 298 - (pPlayer->look_ang >> 1), weaponY + 284 - weaponYOffset, FIRSTGUN + 5,
                                        weaponShade, weaponBits, weaponPal);
            }
            else if ((*weaponFrame) < 27)
            {
                RT_DrawWeaponTileWithID(currentWeapon << 2, 204 - (pPlayer->look_ang >> 1), weaponY + 265 - weaponYOffset,
                                        FIRSTGUN + 8, weaponShade, weaponBits, weaponPal);
                RT_DrawWeaponTileWithID(currentWeapon, 308 - (pPlayer->look_ang >> 1), weaponY + 299 - weaponYOffset, FIRSTGUN + 5,
                                        weaponShade, weaponBits, weaponPal);
            }

            break;

        case HANDBOMB_WEAPON__STATIC:
            {
                static uint8_t pipebombFrames [] = { 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2 };

                if (*weaponFrame >= ARRAY_SIZE(pipebombFrames))
                    break;

                if (*weaponFrame)
                {
                    if (*weaponFrame < 7)       weaponYOffset -= 10 * (*weaponFrame);  // D
                    else if (*weaponFrame < 12) weaponYOffset += 20 * ((*weaponFrame) - 10);  // U
                    else if (*weaponFrame < 20) weaponYOffset -= 9  * ((*weaponFrame) - 14);  // D

                    weaponYOffset -= 10;
                    weaponYOffset -= *weaponFrame << 1;
                }

                RT_DrawWeaponTileWithID(currentWeapon, weaponX + 190 - halfLookAng, weaponY + 240 - weaponYOffset,
                                        HANDTHROW + pipebombFrames[(*weaponFrame)], weaponShade, weaponBits, weaponPal);
            }
            break;

        case HANDREMOTE_WEAPON__STATIC:
            {
                static uint8_t remoteFrames[] = { 0, 0, 2, 2, 2, 2, 2, 2, 0, 0, 0 };

                if (*weaponFrame >= ARRAY_SIZE(remoteFrames))
                    break;

                weaponX = -48;
                RT_DrawWeaponTileWithID(currentWeapon, weaponX + 150 - halfLookAng, weaponY + 258 - weaponYOffset,
                                        HANDREMOTE + remoteFrames[(*weaponFrame)], weaponShade, weaponBits, weaponPal);
            }
            break;

        case DEVISTATOR_WEAPON__STATIC:
            
            if (*weaponFrame > 3 && *weaponFrame < 8)
            {
                weaponX -= sintable[(1024 + ((*weaponFrame) << 7)) & 2047] >> 10;
                weaponYOffset += sintable[(1024 + ((*weaponFrame) << 7)) & 2047] >> 10;
            }
            if (*weaponFrame > 3 && *weaponFrame < 12)
                RT_DrawWeaponTileWithID(currentWeapon, weaponX + 310 - halfLookAng, weaponY + 288 - weaponYOffset,
                                        3638+((*weaponFrame)-2)/2, weaponShade, weaponBits, weaponPal);
            else
                RT_DrawWeaponTileWithID(currentWeapon, weaponX + 310 - halfLookAng, weaponY + 283 - weaponYOffset,
                                        3638, weaponShade, weaponBits, weaponPal);
            break;

        case FREEZE_WEAPON__STATIC:
            //if (!(duke3d_globalflags & DUKE3D_NO_WIDESCREEN_PINNING) && DUKE)
            //    weaponBits |= 512;

            if ((*weaponFrame) > 0)
            {
                if (doAnim)
                {
                    weaponX += RT_FakeKRand() & 3;
                    weaponY += RT_FakeKRand() & 3;
                }
                
                float sizx = ((RT_FakeKRand() & 255) * (1.f/256.f) + 1.5f);
                float sizy = ((RT_FakeKRand() & 255) * (1.f/256.f) + 1.5f);

                RT_DrawTileFlash(weaponX + 175 - halfLookAng, weaponY + 215 - weaponYOffset, 0xf01, sizx, sizy, RT_FakeKRand()&255, 4);
            }
            RT_DrawWeaponTileWithID(currentWeapon, weaponX + 155 - (pPlayer->look_ang >> 1), weaponY + 290 - weaponYOffset,
                                    FREEZE, weaponShade, weaponBits, weaponPal);
            break;

        case GROW_WEAPON__STATIC:
        case SHRINKER_WEAPON__STATIC:
            weaponX += 28;
            weaponY += 18;

            if ((*weaponFrame) > 0 && doAnim)
            {
                weaponX += RT_FakeKRand() & 3;
                weaponYOffset += (RT_FakeKRand() & 3);
            }
            RT_DrawWeaponTileWithID(currentWeapon, weaponX + 206 - halfLookAng, weaponY + 227 - weaponYOffset,
                                    SHRINKER, weaponShade, weaponBits, weaponPal);
            RT_DrawWeaponTileWithID(currentWeapon << 2, weaponX + 149 - halfLookAng, weaponY + 214 - weaponYOffset,
                                    3884, pPlayer->ammo_amount[GROW_WEAPON] > 0 ? -32 : weaponShade + 12, weaponBits, 2);
            RT_DrawWeaponTileWithID(currentWeapon << 3, weaponX + 155 - halfLookAng, weaponY + 221 - weaponYOffset,
                                    3886, pPlayer->ammo_amount[SHRINKER_WEAPON] > 0 ? -32 : weaponShade + 12, weaponBits, 6);
            RT_DrawWeaponTileWithID(currentWeapon << 1, weaponX + 171 - halfLookAng, weaponY + 203 - weaponYOffset,
                                    3885, (*weaponFrame) > 0 ? -32 : -(sintable[pPlayer->random_club_frame & 2047] >> 10), weaponBits,
                                    currentWeapon == GROW_WEAPON ? 2 : 6);
            break;
        }
    }

enddisplayweapon:
    RT_P_DisplaySpit();

    RT_EnablePolymost();
}

static int lastvisinc;

void RT_P_ProcessWeapon(int playerNum)
{
    DukePlayer_t *const pPlayer      = g_player[playerNum].ps;
    int16_t *const      weaponFrame  = &pPlayer->kickback_pic;
    int const           playerShrunk = (sprite[pPlayer->i].yrepeat < 32);
    uint32_t            playerBits   = g_player[playerNum].inputBits->bits;
    int const           sectorLotag  = sector[pPlayer->cursectnum].lotag;

    if (pPlayer->curr_weapon == SHRINKER_WEAPON || pPlayer->curr_weapon == GROW_WEAPON)
        pPlayer->random_club_frame += 64; // Glowing

    if (pPlayer->rapid_fire_hold == 1)
    {
        if (TEST_SYNC_KEY(playerBits, SK_FIRE))
            return;
        pPlayer->rapid_fire_hold = 0;
    }

    if (playerShrunk || pPlayer->tipincs || pPlayer->access_incs)
        playerBits &= ~BIT(SK_FIRE);
    else if (playerShrunk == 0 && (playerBits & (1 << 2)) && (*weaponFrame) == 0 && pPlayer->fist_incs == 0 &&
             pPlayer->last_weapon == -1 && (pPlayer->weapon_pos == 0 || pPlayer->holster_weapon == 1))
    {
        pPlayer->crack_time = 777 + (RT_KRand2()&255);

        if (pPlayer->holster_weapon == 1)
        {
            if (pPlayer->last_pissed_time <= (GAMETICSPERSEC * 218) && pPlayer->weapon_pos == WEAPON_POS_LOWER)
            {
                pPlayer->holster_weapon = 0;
                pPlayer->weapon_pos     = WEAPON_POS_RAISE;
                P_DoQuote(QUOTE_WEAPON_RAISED, pPlayer);
            }
        }
        else
        {
            switch (DYNAMICWEAPONMAP(pPlayer->curr_weapon))
            {
                case HANDBOMB_WEAPON__STATIC:
                    pPlayer->hbomb_hold_delay = 0;
                    if (pPlayer->ammo_amount[pPlayer->curr_weapon] > 0)
                        (*weaponFrame) = 1;
                    break;

                case HANDREMOTE_WEAPON__STATIC:
                    pPlayer->hbomb_hold_delay = 0;
                    (*weaponFrame)            = 1;
                    break;

                case PISTOL_WEAPON__STATIC:
                case BOWLINGBALL_WEAPON__STATIC:
                    if (pPlayer->ammo_amount[pPlayer->curr_weapon] > 0)
                    {
                        pPlayer->ammo_amount[pPlayer->curr_weapon]--;
                        (*weaponFrame) = 1;
                    }
                    break;

                case SHOTGUN_WEAPON__STATIC:
                case MOTORCYCLE_WEAPON__STATIC:
                    if (pPlayer->ammo_amount[pPlayer->curr_weapon] > 0 && pPlayer->random_club_frame == 0)
                        (*weaponFrame) = 1;
                    break;

                case TRIPBOMB_WEAPON__STATIC:
                    if (pPlayer->ammo_amount[pPlayer->curr_weapon] > 0)
                    {
                        hitdata_t hitData;

                        hitscan((const vec3_t *)pPlayer, pPlayer->cursectnum, sintable[(fix16_to_int(pPlayer->q16ang) + 512) & 2047],
                                sintable[fix16_to_int(pPlayer->q16ang) & 2047], fix16_to_int(F16(100) - pPlayer->q16horiz - pPlayer->q16horizoff) * 32, &hitData,
                                CLIPMASK1);

                        if ((hitData.sect < 0 || hitData.sprite >= 0) ||
                            (hitData.wall >= 0 && sector[hitData.sect].lotag > 2))
                            break;

                        if (hitData.wall >= 0 && wall[hitData.wall].overpicnum >= 0)
                            if (wall[hitData.wall].overpicnum == BIGFORCE)
                                break;

                        int spriteNum = headspritesect[hitData.sect];
                        while (spriteNum >= 0)
                        {
                            if (sprite[spriteNum].picnum == TRIPBOMB && klabs(sprite[spriteNum].z - hitData.pos.z) < ZOFFSET4 &&
                                ((sprite[spriteNum].x - hitData.pos.x) * (sprite[spriteNum].x - hitData.pos.x) +
                                    (sprite[spriteNum].y - hitData.pos.y) * (sprite[spriteNum].y - hitData.pos.y)) < (290 * 290))
                                break;
                            spriteNum = nextspritesect[spriteNum];
                        }

                        // ST_2_UNDERWATER
                        if (spriteNum == -1 && hitData.wall >= 0 && (wall[hitData.wall].cstat & 16) == 0)
                            if ((wall[hitData.wall].nextsector >= 0 && sector[wall[hitData.wall].nextsector].lotag <= 2) ||
                                (wall[hitData.wall].nextsector == -1 && sector[hitData.sect].lotag <= 2))
                                if (((hitData.pos.x - pPlayer->pos.x) * (hitData.pos.x - pPlayer->pos.x) +
                                        (hitData.pos.y - pPlayer->pos.y) * (hitData.pos.y - pPlayer->pos.y)) < (290 * 290))
                                {
                                    pPlayer->pos.z = pPlayer->opos.z;
                                    pPlayer->vel.z = 0;
                                    (*weaponFrame) = 1;
                                }
                    }
                    break;

                case SHRINKER_WEAPON__STATIC:
                    if (pPlayer->ammo_amount[SHRINKER_WEAPON] > 0)
                    {
                        (*weaponFrame) = 1;
                        A_PlaySound(9, pPlayer->i);
                    }
                    break;

                case GROW_WEAPON__STATIC:
                    if (pPlayer->ammo_amount[GROW_WEAPON] > 0)
                    {
                        (*weaponFrame) = 1;
                        A_PlaySound(253, pPlayer->i);
                    }
                    break;

                case FREEZE_WEAPON__STATIC:
                    pPlayer->hbomb_hold_delay = 0;
                    if (pPlayer->ammo_amount[pPlayer->curr_weapon] > 0)
                        (*weaponFrame) = 1;
                    break;

                case CHAINGUN_WEAPON__STATIC:
                case DEVISTATOR_WEAPON__STATIC:
                case RPG_WEAPON__STATIC:
                case BOAT_WEAPON__STATIC:
                    if (pPlayer->ammo_amount[pPlayer->curr_weapon] > 0)
                        (*weaponFrame) = 1;
                    break;

                case KNEE_WEAPON__STATIC:
                    if (pPlayer->quick_kick == 0)
                        (*weaponFrame) = 1;
                    break;
            }
        }
    }
    else if (*weaponFrame)
    {
        int spriteNum;
        switch (DYNAMICWEAPONMAP(pPlayer->curr_weapon))
        {
        case HANDBOMB_WEAPON__STATIC:
            if ((*weaponFrame) == 6 && TEST_SYNC_KEY(playerBits, SK_FIRE))
            {
                pPlayer->rapid_fire_hold = 1;
                break;
            }

            if (++(*weaponFrame) == 12)
            {
                pPlayer->ammo_amount[pPlayer->curr_weapon]--;

                //if (numplayers < 2 || g_netServer)
                {
                    int pipeBombFwdVel;

                    if (pPlayer->on_ground && TEST_SYNC_KEY(playerBits, SK_CROUCH))
                        pipeBombFwdVel = 15;
                    else
                        pipeBombFwdVel = 140;

                    pipeBombFwdVel += pPlayer->hbomb_hold_delay << 5;

                    if (pPlayer->cursectnum >= 0 && sector[pPlayer->cursectnum].lotag == ST_2_UNDERWATER)
                        pipeBombFwdVel /= 2;

                    rt_viewhorizang = RT_GetAngle(fix16_to_int(pPlayer->q16horiz + pPlayer->q16horizoff - F16(100)), 128.f) * (-180.f/fPI);

                    float velX = cosf(rt_viewhorizang * fPI / 180.f);
                    float velZ = sinf(rt_viewhorizang * fPI / 180.f);

                    int pipeSpriteNum = A_InsertSprite(pPlayer->cursectnum,
                                        pPlayer->pos.x+(sintable[(fix16_to_int(pPlayer->q16ang)+512)&2047]>>6),
                                        pPlayer->pos.y+(sintable[fix16_to_int(pPlayer->q16ang)&2047]>>6),
                                        pPlayer->pos.z,HEAVYHBOMB,-16,9,9,
                                        fix16_to_int(pPlayer->q16ang),int(pipeBombFwdVel*velX),int(pipeBombFwdVel*velZ)<<4,pPlayer->i,1);

                    if (pPlayer->on_ground && TEST_SYNC_KEY(playerBits, SK_CROUCH))
                    {
                        sprite[pipeSpriteNum].yvel = 3;
                        sprite[pipeSpriteNum].z += ZOFFSET3;
                    }

                    if (A_GetHitscanRange(pPlayer->i) < 512)
                    {
                        sprite[pipeSpriteNum].ang += 1024;
                        sprite[pipeSpriteNum].zvel /= 3;
                        sprite[pipeSpriteNum].xvel /= 3;
                    }
                }

                pPlayer->hbomb_on = 1;
            }
            else if ((*weaponFrame) < 12 && TEST_SYNC_KEY(playerBits, SK_FIRE))
                pPlayer->hbomb_hold_delay++;
            else if ((*weaponFrame) > 19)
            {
                (*weaponFrame) = 0;
                pPlayer->weapon_pos = WEAPON_POS_RAISE;
                pPlayer->curr_weapon = HANDREMOTE_WEAPON;
                pPlayer->wantweaponfire = HANDREMOTE_WEAPON;
                pPlayer->dn64_372 = HANDREMOTE_WEAPON;
                pPlayer->last_weapon = -1;
            }
            break;

        case HANDREMOTE_WEAPON__STATIC:
            if (++(*weaponFrame) == 2)
            {
                pPlayer->hbomb_on = 0;
            }

            if ((*weaponFrame) == 10)
            {
                (*weaponFrame) = 0;
                int weapon = HANDBOMB_WEAPON;
                if (pPlayer->ammo_amount[weapon] > 0)
                {
                    P_AddWeapon(pPlayer, weapon);
                }
                else
                {
                    P_CheckWeapon(pPlayer);
                }
            }
            break;

        case PISTOL_WEAPON__STATIC:
        case BOWLINGBALL_WEAPON__STATIC:
            if ((*weaponFrame) == 1)
            {
                A_Shoot(pPlayer->i, pPlayer->curr_weapon == BOWLINGBALL_WEAPON ? DN64TILE2598 : SHOTSPARK1);
                A_PlaySound(2, pPlayer->i);
                lastvisinc = (int32_t) totalclock+32;
                pPlayer->visibility = 0;
                pPlayer->dn64_370 = 2;
            }
            else if ((*weaponFrame) == 2 && numplayers == 1)
            {
                A_Spawn(pPlayer->i, SHELL);
            }

            if (++(*weaponFrame) >= 5)
            {
                if (pPlayer->ammo_amount[PISTOL_WEAPON] <= 0 || (pPlayer->ammo_amount[PISTOL_WEAPON]%12))
                {
                    (*weaponFrame) = 0;
                    P_CheckWeapon(pPlayer);
                }
                else
                {
                    switch ((*weaponFrame))
                    {
                    case 5:
                        A_PlaySound(3, pPlayer->i);
                        break;
                    case 8:
                        A_PlaySound(4, pPlayer->i);
                        break;
                    }
                }
            }

            if ((*weaponFrame) == 27)
            {
                (*weaponFrame) = 0;
                P_CheckWeapon(pPlayer);
            }
            break;

        case SHOTGUN_WEAPON__STATIC:
        case MOTORCYCLE_WEAPON__STATIC:
            if (++(*weaponFrame) == 4)
            {
                if (pPlayer->curr_weapon == MOTORCYCLE_WEAPON)
                    A_Shoot(pPlayer->i, DN64TILE2599);
                else
                {
                    A_Shoot(pPlayer->i, DN64TILE2596);
                    A_Shoot(pPlayer->i, DN64TILE2596);
                    A_Shoot(pPlayer->i, DN64TILE2596);
                    A_Shoot(pPlayer->i, DN64TILE2596);
                    A_Shoot(pPlayer->i, DN64TILE2596);
                    A_Shoot(pPlayer->i, DN64TILE2596);
                    A_Shoot(pPlayer->i, DN64TILE2596);
                }
                pPlayer->ammo_amount[SHOTGUN_WEAPON]--;
                pPlayer->dn64_370 = 8;

                A_PlaySound(SHOTGUN_FIRE, pPlayer->i);

                lastvisinc = (int32_t) totalclock + 32;
                pPlayer->visibility = 0;
            }

            switch ((*weaponFrame))
            {
            case 15:
                A_PlaySound(131, pPlayer->i);
                break;
            case 17:
            case 20:
                pPlayer->kickback_pic++;
                break;
            case 24:
                if (numplayers == 1)
                {
                    spriteNum = A_Spawn(pPlayer->i, SHOTGUNSHELL);
                    sprite[spriteNum].zvel -= 64;
                    sprite[spriteNum].ang += 1024;
                    A_SetSprite(spriteNum, CLIPMASK0);
                    sprite[spriteNum].ang += 1024;
                }
                pPlayer->kickback_pic++;
                break;
            case 31:
                (*weaponFrame) = 0;
                return;
            }
            break;

        case CHAINGUN_WEAPON__STATIC:
            if (++(*weaponFrame) <= 12)
            {
                if (((*weaponFrame) % 3) == 0)
                {
                    pPlayer->ammo_amount[CHAINGUN_WEAPON] -= 2;


                    if (numplayers == 1)
                    {
                        int v6 = fix16_to_int(pPlayer->q16horiz + pPlayer->q16horizoff - F16(100));
                        if (v6 > 0)
                            v6 = v6/4;
                        else
                            v6 = -(v6/2);

                        v6 += 140;
                        
                        spriteNum = A_Spawn(pPlayer->i, SHELL);
                        
                        sprite[spriteNum].ang = (sprite[pPlayer->i].ang + v6) & 2047;
                        sprite[spriteNum].x = sprite[pPlayer->i].x + (sintable[(sprite[spriteNum].ang+512)&2047]>>7);
                        sprite[spriteNum].y = sprite[pPlayer->i].y + (sintable[sprite[spriteNum].ang]>>7);
                        sprite[spriteNum].ang = (sprite[pPlayer->i].ang + 512) & 2047;
                        sprite[spriteNum].zvel -= 160;
                        sprite[spriteNum].xvel = 4;
                        sprite[spriteNum].z = pPlayer->pyoff + pPlayer->pos.z + (3 << 8) - fix16_to_int(pPlayer->q16horiz + pPlayer->q16horizoff - F16(100)) * 14;
                        A_SetSprite(spriteNum, CLIPMASK0);
                        
                        spriteNum = A_Spawn(pPlayer->i, SHELL);
                        
                        sprite[spriteNum].ang = (sprite[pPlayer->i].ang - v6) & 2047;
                        sprite[spriteNum].x = sprite[pPlayer->i].x + (sintable[(sprite[spriteNum].ang+512)&2047]>>7);
                        sprite[spriteNum].y = sprite[pPlayer->i].y + (sintable[sprite[spriteNum].ang]>>7);
                        sprite[spriteNum].ang = (sprite[pPlayer->i].ang - 512) & 2047;
                        sprite[spriteNum].zvel -= 160;
                        sprite[spriteNum].xvel = 4;
                        sprite[spriteNum].z = pPlayer->pyoff + pPlayer->pos.z + (3 << 8) - fix16_to_int(pPlayer->q16horiz + pPlayer->q16horizoff - F16(100)) * 14;
                        A_SetSprite(spriteNum, CLIPMASK0);
                    }

                    A_PlaySound(268, pPlayer->i);
                    fix16_t const q16ang = pPlayer->q16ang;
                    pPlayer->q16ang = (q16ang - F16(32)) & 0x7FFFFFF;
                    A_Shoot(pPlayer->i, CHAINGUN);
                    pPlayer->q16ang = (q16ang + F16(32)) & 0x7FFFFFF;
                    A_Shoot(pPlayer->i, CHAINGUN);
                    pPlayer->q16ang = q16ang;
                    pPlayer->dn64_370 = (RT_KRand2()&3);
                    lastvisinc = (int32_t) totalclock + 32;
                    pPlayer->visibility = 0;
                    P_CheckWeapon(pPlayer);

                    if (!TEST_SYNC_KEY(playerBits, SK_FIRE))
                    {
                        (*weaponFrame) = 0;
                        break;
                    }
                }
            }
            else if ((*weaponFrame) > 10)
            {
                if (TEST_SYNC_KEY(playerBits, SK_FIRE))
                {
                    (*weaponFrame) = 1;
                }
                else
                {
                    (*weaponFrame) = 0;
                }
            }

            break;

        case GROW_WEAPON__STATIC:
            if ((*weaponFrame) > 3)
            {
                (*weaponFrame) = 0;
                if (screenpeek == playerNum)
                {
                    pus = 1;
                }

                pPlayer->ammo_amount[GROW_WEAPON]--;

                A_Shoot(pPlayer->i, GROWSPARK);

                pPlayer->dn64_370 = (RT_KRand2()&1) * 2 - 1;
                pPlayer->visibility = 0;
                lastvisinc = (int32_t) totalclock + 32;
                P_CheckWeapon(pPlayer);
            }
            else
            {
                (*weaponFrame)++;
            }
            break;

        case SHRINKER_WEAPON__STATIC:
            if ((*weaponFrame) > 10)
            {
                (*weaponFrame) = 0;

                pPlayer->ammo_amount[SHRINKER_WEAPON]--;

                A_Shoot(pPlayer->i, SHRINKER);

                pPlayer->visibility = 0;
                lastvisinc = (int32_t) totalclock + 32;
                P_CheckWeapon(pPlayer);
            }
            else
            {
                (*weaponFrame)++;
                pPlayer->dn64_370 = (RT_KRand2()&1) * 2 - 1;
                if ((RT_KRand2()&31) < 10)
                    sprite[pPlayer->i].shade = -96;
            }
            break;

        case DEVISTATOR_WEAPON__STATIC:
            if ((*weaponFrame) > 0)
            {
                (*weaponFrame)++;
                if ((*weaponFrame) == 4)
                {
                    A_Shoot(pPlayer->i, DN64TILE3634);
                    pPlayer->dn64_370 = 16;
                    pPlayer->ammo_amount[pPlayer->curr_weapon]--;
                    P_CheckWeapon(pPlayer);
                    A_PlaySound(267, pPlayer->i);
                }
                if ((*weaponFrame) == 20)
                {
                    (*weaponFrame) = 0;
                }
            }
            break;

        case FREEZE_WEAPON__STATIC:
            (*weaponFrame)++;
            if (TEST_SYNC_KEY(playerBits, SK_FIRE))
            {
                int shake = pPlayer->hbomb_hold_delay / 8;
                if (shake)
                    pPlayer->dn64_370 = (RT_KRand2() % shake) - shake / 2;
                else
                    pPlayer->dn64_370 = 0;
                int v6 = pPlayer->ammo_amount[FREEZE_WEAPON] * 3;
                if (v6 > 99)
                    v6 = 99;
                if (pPlayer->hbomb_hold_delay < v6)
                    pPlayer->hbomb_hold_delay++;
                if ((RT_KRand2()&31) < 10)
                    sprite[pPlayer->i].shade = -96;

                if (!pPlayer->dn_388)
                    pPlayer->dn_388 = A_PlaySound(265, pPlayer->i);
            }
            else if (*weaponFrame > 3)
            {
                int dmg = pPlayer->hbomb_hold_delay / 3;
                if (dmg < 1)
                    dmg = 1;
                pPlayer->ammo_amount[FREEZE_WEAPON] -= dmg;
                if (pPlayer->ammo_amount[FREEZE_WEAPON] < 0)
                    pPlayer->ammo_amount[FREEZE_WEAPON] = 0;
                pPlayer->visibility = 0;
                lastvisinc = (int32_t) totalclock;
                A_Shoot(pPlayer->i, DN64TILE3841);
                if (pPlayer->hbomb_hold_delay < 66)
                    A_PlaySound(264, pPlayer->i);
                else
                    A_PlaySound(266, pPlayer->i);
                P_CheckWeapon(pPlayer);
                (*weaponFrame) = 0;
            }
            break;

        case TRIPBOMB_WEAPON__STATIC:
            if ((*weaponFrame) < 4)
            {
                pPlayer->pos.z = pPlayer->opos.z;
                pPlayer->vel.z = 0;
                if ((*weaponFrame) == 3)
                {
                    A_Shoot(pPlayer->i, HANDHOLDINGLASER);
                }
            }
            if ((*weaponFrame) == 16)
            {
                (*weaponFrame) = 0;
                P_CheckWeapon(pPlayer);
                pPlayer->weapon_pos = WEAPON_POS_LOWER;
            }
            else
            {
                (*weaponFrame)++;
            }
            break;

        case KNEE_WEAPON__STATIC:
            if (++(*weaponFrame) == 7)
            {
                A_Shoot(pPlayer->i, KNEE);
            }
            else if ((*weaponFrame) == 14)
            {
                if (TEST_SYNC_KEY(playerBits, SK_FIRE))
                {
                    (*weaponFrame) = 1+(RT_KRand2()&3);
                }
                else
                {
                    (*weaponFrame) = 0;
                }
            }

            if (pPlayer->wantweaponfire >= 0)
            {
                P_CheckWeapon(pPlayer);
            }
            break;

        case RPG_WEAPON__STATIC:
        case BOAT_WEAPON__STATIC:
            if (++(*weaponFrame) == 4)
            {
                pPlayer->ammo_amount[pPlayer->curr_weapon]--;
                lastvisinc = (int32_t) totalclock + 32;
                pPlayer->visibility = 0;
                if (pPlayer->curr_weapon == BOAT_WEAPON)
                    A_Shoot(pPlayer->i, DN64TILE2606);
                else
                    A_Shoot(pPlayer->i, RPG);
                pPlayer->dn64_370 = 16;
                P_CheckWeapon(pPlayer);
            }
            else if ((*weaponFrame) == 30)
            {
                (*weaponFrame) = 0;
            }
            break;
        }
    }
}
