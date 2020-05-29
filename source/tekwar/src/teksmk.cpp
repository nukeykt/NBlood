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
#include "keyboard.h"
#include "SmackerDecoder.h"

extern int nobriefflag;

SmackerHandle hSMK;
uint8_t* pFrame = nullptr;
uint8_t* pMenuBackground = nullptr;
uint32_t nWidth, nHeight;

#define kSMKPal   5
#define kSMKTile  (MAXTILES-1)


void smkplayseq(const char *name)
{
    char filename[BMAX_PATH];
 
    if ((generalplay != 0) || (nobriefflag != 0)) {
        return;
    }

    strcpy(filename, "/SMK/");
    strcat(filename, name);
    strcat(filename, ".SMK");

    hSMK = Smacker_Open(filename);
    if (!hSMK.isValid)
    {
        return;
    }

    Smacker_GetFrameSize(hSMK, nWidth, nHeight);
    pFrame = (uint8_t*)Xmalloc(nWidth * nHeight);
    walock[kSMKTile] = CACHE1D_PERMANENT;
    waloff[kSMKTile] = (intptr_t)pFrame;
    tileSetSize(kSMKTile, nHeight, nWidth);

    if (!pFrame)
    {
        Smacker_Close(hSMK);
        return;
    }

    int nFrameRate = Smacker_GetFrameRate(hSMK);
    int nFrames = Smacker_GetNumFrames(hSMK);

    auto const oyxaspect = yxaspect;

    int nScale = tabledivide32(scale(65536, ydim << 2, xdim * 3), ((max(nHeight, 240 + 1u) + 239) / 240));
    int nStat = 2|4|8|64|1024;
    renderSetAspect(viewingrange, 65536);

    #if 0
    Smacker_GotoFrame(hSMK, 16);

    rgb24_t quickpal[256];

    Smacker_GetPalette(hSMK, (uint8_t*)&quickpal);
    Smacker_GetFrame(hSMK, pFrame);

    FILE* fw = fopen("c:/temp/wilshart.raw", "wb");
    if (fw)
    {
        char pixel[3];
        for (int i = 0; i < nWidth * nHeight; i++)
        {
            memcpy(pixel, &quickpal[pFrame[i]], 3);
            fwrite(pixel, 3, 1, fw);
        }


        fclose(fw);
    }
    #endif

//    paletteSetColorTable(kSMKPal, palette);
//    videoSetPalette(/*gBrightness >> 2*/8, kSMKPal, 8 + 2);

    ClockTicks nStartTime = totalclock;

    int nFrame = 0;
    do
    {
        handleevents();
        if (scale((int)(totalclock - nStartTime), nFrameRate, CLKIPS) < nFrame)
            continue;

        if (KB_KeyPressed(sc_Escape) || KB_KeyPressed(sc_Enter))
            break;

        Smacker_GetPalette(hSMK, palette);
        paletteSetColorTable(kSMKPal, palette);
        videoSetPalette(0, kSMKPal, 8+2);
        
        Smacker_GetFrame(hSMK, pFrame);
        tileInvalidate(kSMKTile, -1, -1);

        rotatesprite_fs(160 << 16, 100 << 16, nScale, 512, kSMKTile, 0, 0, nStat);

        videoNextPage();

        nFrame++;
        Smacker_GetNextFrame(hSMK);
    } while (nFrame < nFrames);

    KB_FlushKeyboardQueue();
    KB_ClearKeysDown();

    Smacker_Close(hSMK);
    renderSetAspect(viewingrange, oyxaspect);
    videoSetPalette(0, 0, 8 + 2);
    walock[kSMKTile] = 0;
    waloff[kSMKTile] = 0;
    tileSetSize(kSMKTile, 0, 0);
    Bfree(pFrame);
    pFrame = nullptr;

//    debugprintf("smkplayseq(\"%s\")\n", name);
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

    if (!pFrame || !pMenuBackground)
    {
        Smacker_Close(hSMK);
        return;
    }

    walock[kSMKTile] = CACHE1D_PERMANENT;
    waloff[kSMKTile] = (intptr_t)pFrame;
    tileSetSize(kSMKTile, nHeight, nWidth);
    tileInvalidate(kSMKTile, 0, 1 << 4);  // JBF 20031228
    
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

    tileInvalidate(kSMKTile, 0, 1 << 4);  // JBF 20031228

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
    rotatesprite(160 << 16, 100 << 16, divscale16(200, tilesiz[kSMKTile].x),
        512, kSMKTile, 0, 0, 2 + 4, 0, 0, xdim - 1, ydim - 1);

    videoNextPage();
}

void smkclosemenu()
{
    if (!hSMK.isValid)
        return;

    Smacker_Close(hSMK);

    walock[kSMKTile] = 0;
    waloff[kSMKTile] = 0;
    tileSetSize(kSMKTile, 0, 0);
    Bfree(pFrame);
    Bfree(pMenuBackground);
}
