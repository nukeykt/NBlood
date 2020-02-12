//-------------------------------------------------------------------------
/*
Copyright (C) 2010-2019 EDuke32 developers and contributors
Copyright (C) 2019 sirlemonhead, Nuke.YKT
This file is part of PCExhumed.
PCExhumed is free software; you can redistribute it and/or
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

#include "engine.h"
#include "exhumed.h"
#include "names.h"
#include "movie.h"
#include "light.h"
#include <cstdio>
#include <cstring>
#include "baselayer.h"
#include "typedefs.h"
#include "keyboard.h"
#include "fx_man.h"
#include "sound.h"
#include "mutex.h"

enum {
    kFramePalette = 0,
    kFrameSound,
    kFrameImage,
    kFrameDone
};

#define kSampleRate     22050
#define kSampleSize     2205

uint8_t bankbuf[kSampleRate];
uint32_t bankptr = 0;
uint32_t banktail = 0;

uint32_t lSoundBytesRead = 0;
uint32_t lSoundBytesUsed = 0;

uint8_t lh[32] = { 0 };

static uint8_t* CurFrame = NULL;

bool bServedSample = false;
palette_t moviepal[256];
static mutex_t mutex = 0;


int ReadFrame(FILE *fp)
{
    uint8_t nType;
    uint8_t var_1C;
    int nSize;
    uint16_t yOffset;
    uint8_t xOffset;
    uint8_t nPixels;
    uint8_t palette[768];

    while (1)
    {
        if (fread(&nType, 1, sizeof(nType), fp) == 0) {
            return 0;
        }

        fread(&nSize, sizeof(nSize), 1, fp);

        nType--;
        if (nType > 3) {
            continue;
        }

        switch (nType)
        {
            case kFramePalette:
            {
                fread(palette, sizeof(palette[0]), sizeof(palette) / sizeof(palette[0]), fp);
                fread(&var_1C, sizeof(var_1C), 1, fp);

                for (auto &c : palette)
                    c <<= 2;

                paletteSetColorTable(ANIMPAL, palette);
                videoSetPalette(0, ANIMPAL, 2+8);

                memset(CurFrame, overscanindex, 4); //sizeof(CurFrame));
                continue;
            }
            case kFrameSound:
            {
                if (lSoundBytesRead - lSoundBytesUsed >= kSampleRate)
                {
                    DebugOut("ReadFrame() - Sound buffer full\n");
                    fseek(fp, nSize, SEEK_CUR);
                }
                else
                {
                    mutex_lock(&mutex);

                    int nRead = fread((char*)bankbuf + bankptr, 1, nSize, fp);

                    lSoundBytesRead += nRead;
                    bankptr += nSize;

                    assert(nSize == nRead);
                    assert(bankptr <= kSampleRate);

                    if (bankptr >= kSampleRate) {
                        bankptr -= kSampleRate; // loop back to start
                    }

                    mutex_unlock(&mutex);
                }

                continue;
            }
            case kFrameImage:
            {
                if (nSize == 0) {
                    continue;
                }

                uint8_t *pFrame = CurFrame;

                int nRead = fread(&yOffset, 1, sizeof(yOffset), fp);
                nSize -= nRead;

                pFrame += yOffset * 200; // row position

                while (nSize > 0)
                {
                    fread(&xOffset, sizeof(xOffset), 1, fp);
                    fread(&nPixels, sizeof(nPixels), 1, fp);
                    nSize -= 2;

                    pFrame += xOffset;

                    if (nPixels)
                    {
                        int nRead = fread(pFrame, 1, nPixels, fp);
                        pFrame += nRead;
                        nSize -= nRead;
                    }
                }

                tileInvalidate(kMovieTile, -1, -1);
                break;
            }
            case kFrameDone:
            {
                return 1;
                break;
            }
        }
    }
}

static void ServeSample(const char** ptr, uint32_t* length)
{
    mutex_lock(&mutex);

    *ptr = (char*)bankbuf + banktail;
    *length = kSampleSize;

    banktail += kSampleSize;
    if (banktail >= kSampleRate) {
        banktail -= kSampleRate; // rotate back to start
    }

    lSoundBytesUsed += kSampleSize;
    bServedSample = true;

    mutex_unlock(&mutex);
}

void PlayMovie(const char* fileName)
{
    int bDoFade = kTrue;
    int hFx = -1;

    tileLoad(kMovieTile);
    CurFrame = (uint8_t*)waloff[kMovieTile];

    FILE* fp = fopen(fileName, "rb");
    if (fp == NULL)
    {
        DebugOut("Can't open movie file %s\n", fileName);
        return;
    }

    fread(lh, sizeof(lh), 1, fp);

    // sound stuff
    mutex_init(&mutex);
    bankptr = 0;
    banktail = 0;

    // clear keys
    KB_FlushKeyboardQueue();
    KB_ClearKeysDown();

    if (bDoFade) {
        StartFadeIn();
    }

    int angle = 1536;
    int z = 0;
    int f = 255;

    videoSetPalette(0, ANIMPAL, 2 + 8);

    // Read a frame in first
    if (ReadFrame(fp))
    {
        // start audio playback
        hFx = FX_StartDemandFeedPlayback(ServeSample, 8, 1, kSampleRate, 0, gMusicVolume, gMusicVolume, gMusicVolume, FX_MUSIC_PRIORITY, fix16_one, -1);

        while (!KB_KeyWaiting())
        {
            HandleAsync();

            // audio is king for sync - if the backend doesn't need any more samples yet, 
            // don't process any more movie file data.
            if (!bServedSample) {
                continue;
            }

            bServedSample = false;

            if (z < 65536) { // Zoom - normal zoom is 65536.
                z += 2048;
            }
            if (angle != 0) {
                angle += 16;
                if (angle == 2048) {
                    angle = 0;
                }
            }

            videoClearViewableArea(blackcol);
            rotatesprite(160 << 16, 100 << 16, z, angle, kMovieTile, 0, 1, 2, 0, 0, xdim - 1, ydim - 1);

            if (videoGetRenderMode() == REND_CLASSIC)
            {
                if (bDoFade) {
                    bDoFade = DoFadeIn();
                }
            }
#ifdef USE_OPENGL
            else
            {
                if (f >= 0)
                {
                    fullscreen_tint_gl(0, 0, 0, f);
                    f -= 8;
                }
            }
#endif

            videoNextPage();

            if (ReadFrame(fp) == 0) {
                break;
            }
        }
    }

    if (hFx > 0) {
        FX_StopSound(hFx);
    }

    if (KB_KeyWaiting()) {
        KB_GetCh();
    }

    mutex_destroy(&mutex);
    fclose(fp);

#ifdef USE_OPENGL
    // need to do OpenGL fade out here
    f = 0;

    while (f <= 255)
    {
        HandleAsync();

        rotatesprite(160 << 16, 100 << 16, z, angle, kMovieTile, 0, 1, 2, 0, 0, xdim - 1, ydim - 1);

        fullscreen_tint_gl(0, 0, 0, f);
        f += 4;

        WaitTicks(2);

        videoNextPage();
    }
#endif
}
