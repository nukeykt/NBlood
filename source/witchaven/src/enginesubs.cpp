
#include "build.h"
#include "witchaven.h"

void overwritesprite(int thex, int they, short tilenum, int8_t shade, char stat, char dapalnum)
{
#if 0
    rotatesprite(thex << 16, they << 16, 0x10000, (short)((stat & 8) << 7), tilenum, shade, dapalnum,
        (char)(((stat & 1 ^ 1) << 4) + (stat & 2) + ((stat & 4) >> 2) + ((stat & 16) >> 2) ^ ((stat & 8) >> 1)),
        windowxy1.x, windowxy1.y, windowxy2.x, windowxy2.y);
#else
    // no animation
    uint8_t animbak = picanm[tilenum].sf;
    picanm[tilenum].sf = 0;
    int offx = 0, offy = 0;
    if (stat & 1)
    {
        offx -= tilesiz[tilenum].x >> 1;
        if (stat & 8)
            offx += picanm[tilenum].xofs;
        else
            offx -= picanm[tilenum].xofs;
        offy -= (tilesiz[tilenum].y >> 1) + picanm[tilenum].yofs;
    }
    if (stat & 8)
        offx += tilesiz[tilenum].x;
    if (stat & 16)
        offy += tilesiz[tilenum].y;
    thex += offx;
    they += offy;
    rotatesprite(thex << 16, they << 16, 65536L, (stat & 8) << 7, tilenum, shade, dapalnum,
        16 + (stat & 2) + ((stat & 4) >> 2) + (((stat & 16) >> 2) ^ ((stat & 8) >> 1)),
        windowxy1.x, windowxy1.y, windowxy2.x, windowxy2.y);
    picanm[tilenum].sf = animbak;
#endif
}

void permanentwritesprite(int thex, int they, short tilenum, int8_t shade, int cx1, int cy1, int cx2, int cy2, char dapalnum)
{
    rotatesprite(thex << 16, they << 16, 65536L, 0, tilenum, shade, dapalnum, 8 + 16, cx1, cy1, cx2, cy2);
}

void setbrightness(int32_t brightness)
{
     videoSetPalette(brightness, BASEPAL, 2|8|32);
}
