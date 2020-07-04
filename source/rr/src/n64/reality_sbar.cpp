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
    if (timer > (int)totalclock)
        timer = 0;
    if (timer + 4 < (int)totalclock)
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


int RT_DrawStatusBar(int snum)
{
    RT_DisablePolymost(0);
    DukePlayer_t *const p = g_player[snum].ps;

    bool drawhud = ud.screen_size == 4 && !ud.althud;

    RT_DrawBarBG(drawhud);

    RT_DrawBarOverlay(drawhud);

    RT_EnablePolymost();

    return drawhud;
}