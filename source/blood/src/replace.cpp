#include "compat.h"
#include "common_game.h"
#include "crc32.h"

#include "globals.h"
#include "screen.h"

int qanimateoffs(int a1, int a2)
{
    int offset = 0;
    if (a1 >= 0 && a1 < kMaxTiles)
    {
        int frames = qpicanm[a1].animframes;
        if (frames > 0)
        {
            int vd;
            if ((a2&0xc000) == 0x8000)
                vd = (Bcrc32(&a2, 2, 0)+gFrameClock)>>qpicanm[a1].animspeed;
            else
                vd = gFrameClock>>qpicanm[a1].animspeed;
            switch (qpicanm[a1].animtype)
            {
            case 1:
                offset = vd % (2*frames);
                if (offset >= frames)
                    offset = 2*frames-offset;
                break;
            case 2:
                offset = vd % (frames+1);
                break;
            case 3:
                offset = -(vd % (frames+1));
                break;
            }
        }
    }
    return offset;
}

void qloadpalette()
{
    scrLoadPalette();
}

int32_t qgetpalookup(int32_t a1, int32_t a2)
{
    if (gFogMode)
        return ClipHigh(a1 >> 8, 15) * 16 + ClipRange(a2, 0, 15);
    else
        return ClipRange((a1 >> 8) + a2, 0, 63);
}