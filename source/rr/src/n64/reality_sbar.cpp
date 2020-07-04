// Copyright: 2020 Nuke.YKT, EDuke32 developers
// License: GPLv2
#include "compat.h"
#include "reality.h"
#include "reality_sbar.h"
#include "../duke3d.h"

float borderx1 = 16.f * 4.f / 3.f;
float bordery1 = 16;
float borderx2 = 320.f - 16.f * 4.f / 3.f;
float bordery2 = 240.f - 16.f;

int bartilescroll, barscrolldir;
int bartilescrollspeed;

float RT_SBarScale(float x)
{
    return x * ud.statusbarscale / 100.f;
}

void RT_ResetBarScroll(void)
{
    if (numplayers > 1)
    {
        bartilescroll = -99;
        barscrolldir = 1;
    }
    else
    {
        bartilescrollspeed = 12;
        bartilescroll = -24;
        barscrolldir = 0;
    }
}

void RT_DrawBarOverlay(bool drawhud)
{
    if (bartilescroll < -70)
        return;
    if (drawhud)
    {
        RT_RotateSpriteSetColor(255, 255, 255, 666);
        float x1 = RT_SBarScale(bartilescroll + 5 - borderx1) + borderx1;
        float x2 = RT_SBarScale(314 - bartilescroll - borderx2) + borderx2;
        float y = RT_SBarScale(-14) + bordery2;
        float sc = RT_SBarScale(100.f);
        RT_RotateSprite(x1, y, sc, sc, 0xe31, 0|256|RTRS_SCALED);
        RT_RotateSprite(x2, y, sc, sc, 0xe31, 0|4|512|RTRS_SCALED);
    }
    static int timer = 0;
    if (timer > (int)totalclock + 16)
        timer = 0;
    if (timer < (int)totalclock)
    {
        timer = (int)totalclock + 4;
        if (!barscrolldir)
        {
            bartilescroll = bartilescroll + bartilescrollspeed;
            bartilescrollspeed--;
            if (bartilescroll > 56)
            {
                bartilescroll = 56;
                barscrolldir = 1;
                bartilescrollspeed = 0;
            }
            if (bartilescrollspeed < 1)
                bartilescrollspeed = 1;
        }
        else
        {
            bartilescroll -= bartilescrollspeed;
            bartilescrollspeed++;
        }
    }
}

void RT_DrawBarBG(bool drawhud)
{
    if (numplayers <= 1)
    {
        if (!drawhud)
            return;
        int o = bartilescroll;
        if (barscrolldir)
            o = 56;

        float x1 = RT_SBarScale(o + 10 - borderx1) + borderx1;
        float x2 = RT_SBarScale(319 - o - borderx2) + borderx2;
        float y = RT_SBarScale(-14) + bordery2;
        float sc = RT_SBarScale(100.f);
        RT_RotateSpriteSetColor(255, 255, 255, 666);
        RT_RotateSprite(x1, y, sc, sc, 0xe30, 0|256|RTRS_SCALED);
        RT_RotateSprite(x2, y, sc, sc, 0xe2f, 0|512|RTRS_SCALED);
    }
    else
        barscrolldir = 1;
}

int RT_InvTileScale(int s, int tile)
{
    int rt_tile = rt_tilemap[tile];
    if (rt_tile == -1)
        return 0;

    if (rt_tileinfo[rt_tile].dimy == 0)
        return 0;

    return (100 * s) / rt_tileinfo[rt_tile].dimy;
}

void RT_PrintNumber(float x, float y, const char *string, int orientation)
{
    while (*string != '\0' && *string != '\n' && *string != '\r')
    {
        int tile = *string;
        if (*string >= '0' && *string <= '9')
            tile = *string + 3689;
        if (*string == '%')
            tile = 3736;

        float sx, sy;
        if (orientation & 256)
            sx = RT_SBarScale(x - borderx1) + borderx1;
        else if (orientation & 512)
            sx = RT_SBarScale(x - borderx2) + borderx2;
        else
            sx = RT_SBarScale(x - (borderx1 + borderx2) * 0.5f) + (borderx1 + borderx2) * 0.5f;
        sy = RT_SBarScale(y - bordery2) + bordery2;
        float sc = RT_SBarScale(100.f);
        RT_RotateSpriteText(sx, sy, sc, sc, tile, orientation);
        x += 4.f;
        string++;
    }
}

void RT_DrawBar(DukePlayer_t *const pPlayer, bool drawhud)
{
    char buf[10];
    static int ammo_icon[] = {
        BOOTS, AMMO, SHOTGUNAMMO, BATTERYAMMO, RPGAMMO, HBOMBAMMO, CRYSTALAMMO, GROWAMMO,
        DEVISTATORAMMO, TRIPBOMBSPRITE, FREEZEAMMO, HBOMBAMMO, 34, 50, 43
    };
    if (!barscrolldir)
        return;

    // if (ud.multimode > 1 && !ud.coop && dukematch_mode != 4)
    // {
    // }

    if (pPlayer->dn64_372 != KNEE_WEAPON && drawhud)
    {
        if (numplayers < 3)
        {
            int tile = ammo_icon[pPlayer->dn64_372];
            int s = RT_InvTileScale(16, tile);
            float x;
            if (numplayers < 3)
                x = 54.f;
            else
                x = 14.f;
            x = RT_SBarScale(x) + borderx1;
            float y = RT_SBarScale(-13.f) + bordery2;
            float sc = RT_SBarScale(s);
            RT_RotateSpriteSetColor(128, 128, 128, 256);
            RT_RotateSprite(x, y, sc, sc, tile, 256|RTRS_SCALED);
        }
        int weaponNum = pPlayer->dn64_372;
        if (weaponNum == HANDREMOTE_WEAPON)
            weaponNum = HANDBOMB_WEAPON;

        sprintf(buf, "%d\n", pPlayer->ammo_amount[weaponNum]);
        if (weaponNum == BOWLINGBALL_WEAPON || weaponNum == MOTORCYCLE_WEAPON || weaponNum == BOAT_WEAPON)
            RT_RotateSpriteSetColor(255, 255, 128, 666);
        else
            RT_RotateSpriteSetColor(255, 255, 255, 666);
        if (numplayers < 3)
            RT_PrintNumber(borderx1 + 52.f, bordery2 - 16.f, buf, 256|RTRS_SCALED);
        // else
        //     RT_GameText();

    }
    int invTile = 0;
    int invAmount = 0;
    bool invActive = false;

    switch (pPlayer->inven_icon)
    {
    case ICON_NONE:
        invTile = 0;
        invAmount = 0;
        break;
    case ICON_FIRSTAID:
        invTile = FIRSTAID;
        invAmount = pPlayer->inv_amount[GET_FIRSTAID];
        break;
    case ICON_STEROIDS:
        invTile = STEROIDS;
        invAmount = (pPlayer->inv_amount[GET_STEROIDS] + 3) / 4;
        invActive = invAmount < 100;
        break;
    case ICON_HOLODUKE:
        invTile = HOLODUKE;
        invActive = pPlayer->holoduke_on > 0;
        invAmount = (pPlayer->inv_amount[GET_HOLODUKE] + 15) / 24;
        break;
    case ICON_JETPACK:
        invTile = JETPACK;
        invActive = pPlayer->jetpack_on != 0;
        invAmount = (pPlayer->inv_amount[GET_JETPACK] + 15) / 16;
        break;
    case ICON_HEATS:
        invTile = HEATSENSOR;
        invActive = pPlayer->heat_on != 0;
        invAmount = pPlayer->inv_amount[GET_HEATS] / 12;
        break;
    case ICON_SCUBA:
        invTile = AIRTANK;
        invAmount = (pPlayer->inv_amount[GET_SCUBA] + 63) / 64;
        break;
    case ICON_BOOTS:
        invTile = BOOTS;
        invAmount = pPlayer->inv_amount[GET_BOOTS] / 2;
        break;
    case ICON_SHIELD:
        invTile = SHIELD;
        invAmount = pPlayer->inv_amount[GET_SHIELD];
        break;
    default:
        break;
    }
    if (numplayers > 2 && invActive && invAmount < 30)
        pPlayer->invdisptime = 20;
    if (invTile && drawhud)
    {
        int s = RT_InvTileScale(16, invTile);
        int alpha;
        if (numplayers < 3)
            alpha = 256;
        else
            alpha = (pPlayer->invdisptime << 8) / 10;
        RT_RotateSpriteSetColor(255, 255, 255, alpha);
        float x = RT_SBarScale(-40.f) + borderx2;
        float y = RT_SBarScale(-11.f) + bordery2;
        float sc = RT_SBarScale(s);
        RT_RotateSprite(x, y, sc, sc, invTile, 512|RTRS_SCALED);
        sprintf(buf, "%d%%\n", invAmount);
        if (invAmount < 30)
            RT_RotateSpriteSetColor(255, 255, 64, alpha);
        else
            RT_RotateSpriteSetColor(255, 255, 255, alpha);
        RT_PrintNumber(borderx2 - 23.f, bordery2 - 10.f, buf, 512|RTRS_SCALED);
        if (pPlayer->inven_icon > 2)
        {
            x = RT_SBarScale(-23.f) + borderx2;
            y = RT_SBarScale(-20.f) + bordery2;
            sc = RT_SBarScale(100.f);
            if (pPlayer->inven_icon < 6)
                RT_RotateSpriteText(x, y, sc, sc, invActive ? 0xea5 : 0xea4, 512|RTRS_SCALED);
            else
                RT_RotateSpriteText(x, y, sc, sc, 0xea3, 512|RTRS_SCALED);
        }
    }
    if (drawhud)
    {
        sprintf(buf, "%d", pPlayer->last_extra);
        if (pPlayer->last_extra < 30)
            RT_RotateSpriteSetColor(255, 64, 64, 666);
        else
            RT_RotateSpriteSetColor(255, 255, 255, 666);
        if (numplayers < 3)
        {
            float x = RT_SBarScale(14.f) + borderx1;
            float y = RT_SBarScale(-18.f) + bordery2;
            float sc = RT_SBarScale(65536.f);
            G_ScreenText(STARTALPHANUM, (int)(x * 65536.f), (int)(y * 65536.f), (int)sc, 0, 0, buf, 0, 0, ROTATESPRITE_FULL16|256,
                0, 7, 0, 0, 0, TEXT_N64COORDS|TEXT_N64NOPAL|TEXT_XCENTER, 0, 0, xdim-1, ydim-1);
        }
    }
    RT_RotateSpriteSetColor(255, 255, 255, 666);
    if (drawhud && (ud.multimode == 1 || ud.coop == 1))
    {
        if (numplayers < 2)
        {
            if (pPlayer->got_access & 1)
            {
                float x = RT_SBarScale(-0x3d) + borderx2;
                float y = RT_SBarScale(-0x18) + bordery2;
                float sc = RT_SBarScale(100.f);
                RT_RotateSprite(x, y, sc, sc, 0xe25, 512|RTRS_SCALED);
            }
            if (pPlayer->got_access & 2)
            {
                float x = RT_SBarScale(-0x41) + borderx2;
                float y = RT_SBarScale(-0x12) + bordery2;
                float sc = RT_SBarScale(100.f);
                RT_RotateSprite(x, y, sc, sc, 0xe26, 512|RTRS_SCALED);
            }
            if (pPlayer->got_access & 4)
            {
                float x = RT_SBarScale(-0x41) + borderx2;
                float y = RT_SBarScale(-10) + bordery2;
                float sc = RT_SBarScale(100.f);
                RT_RotateSprite(x, y, sc, sc, 0xe27, 512|RTRS_SCALED);
            }
        }
    }
}

void RT_DisplayWeaponIcon(float x, float y, int offset, int tile)
{
    int ox = sintable[(offset * 6) & 2047];
    float oy = sintable[(offset * 6 + 512) & 2047] / 65536.f + 0.75f;
    int shade = (int)((oy * 5.f - 4.f) * 255.f);
    int alpha = (int)((oy * 2.f - 1.f) * 256.f);
    RT_RotateSpriteSetColor(shade, shade, shade, alpha);;
    float sx = RT_InvTileScale(16, tile);
    float sy = RT_InvTileScale(16, tile);
    RT_RotateSprite(RT_SBarScale(x + (float)((ox * 60) >> 14) - (borderx1+borderx2)/2.f)+(borderx1+borderx2)/2.f,
        RT_SBarScale(y - bordery2) + bordery2, RT_SBarScale(sx), RT_SBarScale(sy), tile, RTRS_SCALED);
}

void RT_DrawWeaponWheel(DukePlayer_t *const pPlayer, bool drawhud)
{
    if (pPlayer->dn64_378 > 0)
    {
        float x = (borderx1 + borderx2) / 2.f;
        float y = bordery2;
        float yfactor = drawhud ? 1.f : 1.5f;
        int weaponNum = pPlayer->dn64_372;
        if (weaponNum == BOWLINGBALL_WEAPON)
            weaponNum = PISTOL_WEAPON;
        else if (weaponNum == MOTORCYCLE_WEAPON)
            weaponNum = SHOTGUN_WEAPON;
        else if (weaponNum == BOAT_WEAPON)
            weaponNum = RPG_WEAPON;
        int wl1 = P_NextWeapon(pPlayer, weaponNum, -1);
        int wr1 = P_NextWeapon(pPlayer, weaponNum, 1);
        int wl2 = P_NextWeapon(pPlayer, wl1, -1);
        int wr2 = P_NextWeapon(pPlayer, wr1, 1);

        RT_DisplayWeaponIcon(x, y - pPlayer->dn64_378 * yfactor, -120 - pPlayer->dn64_374, WeaponPickupSprites[wl2]);
        RT_DisplayWeaponIcon(x, y - pPlayer->dn64_378 * yfactor, 120 - pPlayer->dn64_374, WeaponPickupSprites[wr2]);
        RT_DisplayWeaponIcon(x, y - pPlayer->dn64_378 * yfactor, -60 - pPlayer->dn64_374, WeaponPickupSprites[wl1]);
        RT_DisplayWeaponIcon(x, y - pPlayer->dn64_378 * yfactor, 60 - pPlayer->dn64_374, WeaponPickupSprites[wr1]);
        RT_DisplayWeaponIcon(x, y - pPlayer->dn64_378 * yfactor, -pPlayer->dn64_374, WeaponPickupSprites[weaponNum]);
    }
}


int RT_DrawStatusBar(int snum)
{
    RT_DisablePolymost(0);
    DukePlayer_t *const p = g_player[snum].ps;

    bool drawhud = ud.screen_size == 4 && !ud.althud;

    RT_DrawWeaponWheel(p, drawhud);

    RT_DrawBarBG(drawhud);

    RT_DrawBar(p, drawhud);

    RT_DrawBarOverlay(drawhud);

    RT_EnablePolymost();

    return drawhud;
}
