#include "build.h"
#include "colmatch.h"
#include "reality.h"

tileinfo_t rt_tileinfo[RT_TILENUM];
int32_t rt_tilemap[MAXTILES];
intptr_t rt_waloff[RT_TILENUM];
char rt_walock[RT_TILENUM];

float rt_viewhorizang;

static bool RT_TileLoad(int16_t tilenum);

void RT_LoadTiles(void)
{
    const int tileinfoOffset = 0x90bf0;
    Blseek(rt_group, tileinfoOffset, SEEK_SET);
    if (Bread(rt_group, rt_tileinfo, sizeof(rt_tileinfo)) != sizeof(rt_tileinfo))
    {
        initprintf("RT_LoadTiles: file read error");
        return;
    }

    Bmemset(rt_tilemap, -1, sizeof(rt_tilemap));
    Bmemset(rt_waloff, 0, sizeof(rt_waloff));
    Bmemset(rt_walock, 0, sizeof(rt_walock));

    for (int i = 0; i < RT_TILENUM; i++)
    {
        auto &t = rt_tileinfo[i];
        t.fileoff = B_BIG32(t.fileoff);
        t.waloff = B_BIG32(t.waloff);
        t.picanm = B_BIG32(t.picanm);
        t.sizx = B_BIG16(t.sizx);
        t.sizy = B_BIG16(t.sizy);
        t.filesiz = B_BIG16(t.filesiz);
        t.dimx = B_BIG16(t.dimx);
        t.dimy = B_BIG16(t.dimy);
        t.flags = B_BIG16(t.flags);
        t.tile = B_BIG16(t.tile);

        rt_tilemap[t.tile] = i;
        tilesiz[t.tile].x = t.sizx;
        tilesiz[t.tile].y = t.sizy;
        tileConvertAnimFormat(t.tile, t.picanm);
        tileUpdatePicSiz(t.tile);
    }

    rt_tileload_callback = RT_TileLoad;
#if 0
    for (auto& t : rt_tileinfo)
    {
        char *data = (char*)tileCreate(t.tile, t.sizx, t.sizy);
        int bufsize = 0;
        if (t.flags & RT_TILE8BIT)
        {
            bufsize = t.dimx * t.dimy;
        }
        else
        {
            bufsize = (t.dimx * t.dimy) / 2 + 32;
        }
        tileConvertAnimFormat(t.tile, t.picanm);
        char *inbuf = (char*)Xmalloc(t.filesiz);
        char *outbuf = (char*)Xmalloc(bufsize);
        Blseek(rt_group, dataOffset+t.fileoff, SEEK_SET);
        Bread(rt_group, inbuf, t.filesiz);
        if (RNCDecompress(inbuf, outbuf) == -1)
        {
            Bmemcpy(outbuf, inbuf, bufsize);
        }
        Xfree(inbuf);
        if (t.flags & RT_TILE8BIT)
        {
            for (int i = 0; i < t.sizx; i++)
            {
                for (int j = 0; j < t.sizy; j++)
                {
                    int ii = t.dimx - 1 - ((t.sizx - i - 1) * t.dimx) / t.sizx;
                    int jj = t.dimy - 1 - ((t.sizy - j - 1) * t.dimy) / t.sizy;
                    data[i*t.sizy+j] = outbuf[j*t.dimx+i];
                }
            }
        }
        else
        {
            int palremap[16];
            char *pix = outbuf+32;
            for (int i = 0; i < 16; i++)
            {
                int t = (outbuf[i*2+1] << 8) + outbuf[i*2];
                int r = (t >> 11) & 31;
                int g = (t >> 6) & 31;
                int b = (t >> 1) & 31;
                int a = (t >> 0) & 1;
                r = (r << 3) + (r >> 2);
                g = (g << 3) + (g >> 2);
                b = (b << 3) + (b >> 2);
                if (a == 0)
                    palremap[i] = 255;
                else
                {
                    palremap[i] = paletteGetClosestColor(r, g, b);
                }
            }
            for (int i = 0; i < t.sizx; i++)
            {
                for (int j = 0; j < t.sizy; j++)
                {
                    int ii = t.dimx - 1 - ((t.sizx - i - 1) * t.dimx) / t.sizx;
                    int jj = t.dimy - 1 - ((t.sizy - j - 1) * t.dimy) / t.sizy;
                    int ix = jj * t.dimx + ii;
                    char b = pix[ix>>1];
                    if (ix&1)
                        b &= 15;
                    else
                        b = (b >> 4) & 15;
                    data[i*t.sizy+j] = palremap[b];
                }
            }
        }
        Xfree(outbuf);
    }
#endif
}

bool RT_TileLoad(int16_t tilenum)
{
    const int dataOffset = 0xc2270;
    int32_t const tileid = rt_tilemap[tilenum];
    if (tileid < 0)
        return false;
    auto &t = rt_tileinfo[tileid];
    int bufsize = 0;
    if (t.flags & RT_TILE8BIT)
    {
        bufsize = t.dimx * t.dimy;
    }
    else
    {
        bufsize = (t.dimx * t.dimy) / 2 + 32;
    }
    if (rt_waloff[tileid] == 0)
    {
        walock[tileid] = CACHE1D_UNLOCKED;
        g_cache.allocateBlock(&rt_waloff[tileid], bufsize, &walock[tileid]);
    }
    if (!rt_waloff[tileid])
        return false;
    char *inbuf = (char*)Xmalloc(t.filesiz);
    Blseek(rt_group, dataOffset+t.fileoff, SEEK_SET);
    Bread(rt_group, inbuf, t.filesiz);
    if (RNCDecompress(inbuf, (char*)rt_waloff[tileid]) == -1)
    {
        Bmemcpy((char*)rt_waloff[tileid], inbuf, bufsize);
    }
    Xfree(inbuf);

    if (waloff[tilenum])
    {
        char *data = (char*)waloff[tilenum];
        char *src = (char*)rt_waloff[tileid];
        if (t.flags & RT_TILE8BIT)
        {
            for (int i = 0; i < t.sizx; i++)
            {
                for (int j = 0; j < t.sizy; j++)
                {
                    int ii = t.dimx - 1 - ((t.sizx - i - 1) * t.dimx) / t.sizx;
                    int jj = t.dimy - 1 - ((t.sizy - j - 1) * t.dimy) / t.sizy;
                    data[i*t.sizy+j] = src[j*t.dimx+i];
                }
            }
        }
        else
        {
            int palremap[16];
            char *pix = src+32;
            for (int i = 0; i < 16; i++)
            {
                int t = (src[i*2+1] << 8) + src[i*2];
                int r = (t >> 11) & 31;
                int g = (t >> 6) & 31;
                int b = (t >> 1) & 31;
                int a = (t >> 0) & 1;
                r = (r << 3) + (r >> 2);
                g = (g << 3) + (g >> 2);
                b = (b << 3) + (b >> 2);
                if (a == 0)
                    palremap[i] = 255;
                else
                {
                    palremap[i] = paletteGetClosestColor(r, g, b);
                }
            }
            for (int i = 0; i < t.sizx; i++)
            {
                for (int j = 0; j < t.sizy; j++)
                {
                    int ii = t.dimx - 1 - ((t.sizx - i - 1) * t.dimx) / t.sizx;
                    int jj = t.dimy - 1 - ((t.sizy - j - 1) * t.dimy) / t.sizy;
                    int ix = jj * t.dimx + ii;
                    char b = pix[ix>>1];
                    if (ix&1)
                        b &= 15;
                    else
                        b = (b >> 4) & 15;
                    data[i*t.sizy+j] = palremap[b];
                }
            }
        }
    }

    return true;
}
