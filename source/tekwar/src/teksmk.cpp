/***************************************************************************
 *   TEKSMK.C  -   smack flic stuff etc. for Tekwar                        *
 *                                                                         *
 *                                                                         *
 ***************************************************************************/

#include "build.h"
#include "compat.h"
#include "baselayer.h"
#include "pragmas.h"
#include "cache1d.h"
#include "tekwar.h"
#include "SmackerDecoder.h"

SmackerHandle hSMK;
uint8_t* pFrame = nullptr;
uint8_t* pMenuBackground = nullptr;
uint32_t nWidth, nHeight;

#define kSMKPal 5
#define SMKPICNUM (MAXTILES-1)


void smkplayseq(const char *name)
{
    debugprintf("smkplayseq(\"%s\")\n", name);
}

void smkopenmenu(const char *name)
{
    hSMK = Smacker_Open(name);
    if (!hSMK.isValid)
    {
        return;
    }

    Smacker_GetFrameSize(hSMK, nWidth, nHeight);
    Smacker_GotoFrame(hSMK, 0);

    Smacker_GetPalette(hSMK, palette);
    paletteSetColorTable(kSMKPal, palette);
    videoSetPalette(/*gBrightness >> 2*/8, kSMKPal, 8 + 2);

    pFrame = (uint8_t*)Xmalloc(nWidth * nHeight);
    pMenuBackground = (uint8_t*)Xmalloc(nWidth * nHeight);

    walock[SMKPICNUM] = CACHE1D_PERMANENT;
    waloff[SMKPICNUM] = (intptr_t)pFrame;
    tileSetSize(SMKPICNUM, nHeight, nWidth);
    tileInvalidate(SMKPICNUM, 0, 1 << 4);  // JBF 20031228
    
    // first frame is menu background - keep a copy around for later
    Smacker_GetFrame(hSMK, pMenuBackground);
    memcpy(pFrame, pMenuBackground, nWidth * nHeight);
}

void smkmenuframe(int fn)
{
    if (!hSMK.isValid)
        return;

    Smacker_GetPalette(hSMK, palette);
    paletteSetColorTable(kSMKPal, palette);
    videoSetPalette(/*gBrightness >> 2*/8, kSMKPal, 8 + 2);

    tileInvalidate(SMKPICNUM, 0, 1 << 4);  // JBF 20031228

    Smacker_GotoFrame(hSMK, fn - 1);
    Smacker_GetFrame(hSMK, pFrame);

    // replace transparent pixels on overlay with background pixel
    for (int y = 0; y < nHeight; y++)
    {
        for (int x = 0; x < nWidth; x++)
        {
            uint32_t nPos = (y * nWidth) + x;
            if (pFrame[nPos] == 232) {
                pFrame[nPos] = pMenuBackground[nPos];
            }
        }
    }
}

void smkshowmenu()
{
    if (!hSMK.isValid)
        return;

    videoSetViewableArea(0, 0, xdim - 1, ydim - 1);
    videoClearViewableArea(0);
    rotatesprite(160 << 16, 100 << 16, divscale16(200, tilesiz[SMKPICNUM].x),
        512, SMKPICNUM, 0, 0, 2 + 4, 0, 0, xdim - 1, ydim - 1);

    videoNextPage();
}

void smkclosemenu()
{
    if (!hSMK.isValid)
        return;

    Smacker_Close(hSMK);

    walock[SMKPICNUM] = 0;
    waloff[SMKPICNUM] = 0;
    tileSetSize(SMKPICNUM, 0, 0);
    Bfree(pFrame);
    Bfree(pMenuBackground);
}
