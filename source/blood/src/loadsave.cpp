#include "build.h"
#include "compat.h"
#include "common_game.h"
#include "config.h"
#include "ai.h"
#include "asound.h"
#include "blood.h"
#include "globals.h"
#include "db.h"
#include "messages.h"
#include "menu.h"
#include "network.h"
#include "loadsave.h"
#include "resource.h"
#include "screen.h"
#include "sectorfx.h"
#include "seq.h"
#include "sfx.h"
#include "sound.h"
#include "view.h"

// NUKE-TODO:
#pragma init_seg(lib)

GAMEOPTIONS gSaveGameOptions[10];
char *gSaveGamePic[10];
unsigned int gSavedOffset = 0;

unsigned int dword_27AA38 = 0;
unsigned int dword_27AA3C = 0;
unsigned int dword_27AA40 = 0;
void *dword_27AA44 = NULL;

LoadSave LoadSave::head(123);
int LoadSave::hFile = -1;

short word_27AA54 = 0;

void sub_76FD4(void)
{
    if (!dword_27AA44)
        dword_27AA44 = Resource::Alloc(0x186a0);
}

void LoadSave::Save(void)
{
    ThrowError("Pure virtual function called");
}

void LoadSave::Load(void)
{
    ThrowError("Pure virtual function called");
}

void LoadSave::Read(void *pData, int nSize)
{
    dword_27AA38 += nSize;
    dassert(hFile != -1);
    if (read(hFile, pData, nSize) == -1)
        ThrowError("File error #%d reading save file.", errno);
}

void LoadSave::Write(void *pData, int nSize)
{
    dword_27AA38 += nSize;
    dword_27AA3C += nSize;
    dassert(hFile != -1);
    if (write(hFile, pData, nSize) == -1)
        ThrowError("File error #%d writing save file.", errno);
}

void LoadSave::LoadGame(char *pzFile)
{
    sndKillAllSounds();
    sfxKillAllSounds();
    ambKillAll();
    seqKillAll();
    // PORT-TODO:
    //if (gRedBookInstalled)
    //    Redbook.StopSong();
    if (!gGameStarted)
    {
        memset(xsprite, 0, sizeof(xsprite));
        memset(qsprite, 0, sizeof(SPRITE)*kMaxSprites);
        automapping = 1;
    }
    hFile = open(pzFile, 0x200);
    if (hFile == -1)
        ThrowError("File error #%d loading save file.", errno);
    LoadSave *rover = head.next;
    while (rover != &head)
    {
        rover->Load();
        rover = rover->next;
    }
    close(hFile);
    hFile = -1;
    if (!gGameStarted)
        scrLoadPLUs();
    InitSectorFX();
    viewInitializePrediction();
    PreloadCache();
    ambInit();
    memset(myMinLag, 0, sizeof(myMinLag));
    otherMinLag = 0;
    myMaxLag = 0;
    gNetFifoClock = 0;
    gNetFifoTail = 0;
    memset(gNetFifoHead, 0, sizeof(gNetFifoHead));
    gPredictTail = 0;
    gNetFifoMasterTail = 0;
    memset(gFifoInput, 0, sizeof(gFifoInput));
    memset(gChecksum, 0, sizeof(gChecksum));
    memset(gCheckFifo, 0, sizeof(gCheckFifo));
    memset(gCheckHead, 0, sizeof(gCheckHead));
    gSendCheckTail = 0;
    gCheckTail = 0;
    gBufferJitter = 0;
    bOutOfSync = 0;
    viewSetMessage("");
    viewSetErrorMessage("");
    if (!gGameStarted)
    {
        netWaitForEveryone(0);
        memset(gPlayerReady, 0, sizeof(gPlayerReady));
    }
    gFrameTicks = 0;
    gFrame = 0;
    gCacheMiss = 0;
    gFrameRate = 0;
    gGameClock = 0;
    gPaused = 0;
    gGameStarted = 1;
    // PORT-TODO:
    //if (!bNoCDAudio && gRedbookInstalled && Redbook.preprocess())
    //{
    //    Redbook.play_song(gGameOptions.nTrackNumber);
    //    Redbook.cd_status();
    //    Redbook.sub_82bb4();
    //}
    //else
        sndPlaySong(gGameOptions.zLevelSong, 1);
}

void LoadSave::SaveGame(char *pzFile)
{
    hFile = open(pzFile, 0x261, 0x1b0);
    if (hFile == -1)
        ThrowError("File error #%d creating save file.", errno);
    dword_27AA38 = 0;
    dword_27AA40 = 0;
    LoadSave *rover = head.next;
    while (rover != &head)
    {
        rover->Save();
        if (dword_27AA38 > dword_27AA40)
            dword_27AA40 = dword_27AA38;
        dword_27AA38 = 0;
        rover = rover->next;
    }
    close(hFile);
    hFile = -1;
}

class MyLoadSave : public LoadSave
{
public:
    virtual void Load(void);
    virtual void Save(void);
};

void MyLoadSave::Load(void)
{
    int id;
    Read(&id, sizeof(id));
    if (id != 'DULB')
        ThrowError("Old saved game found");
    short version;
    Read(&version, sizeof(version));
    if (version != BloodVersion)
        ThrowError("Incompatible version of saved game found!");
    int release;
    Read(&release, sizeof(release));
    id = 4;
    if (release != id)
        ThrowError("Saved game is from another release of Blood");
    Read(&gGameOptions, sizeof(gGameOptions));
    Read(&numsectors, sizeof(numsectors));
    Read(&numwalls, sizeof(numwalls));
    Read(&numsectors, sizeof(numsectors));
    int nNumSprites;
    Read(&nNumSprites, sizeof(nNumSprites));
    memset(sector, 0, sizeof(sector));
    memset(wall, 0, sizeof(wall));
    memset(sprite, 0, sizeof(sprite));
    Read(sector, sizeof(sector[0])*numsectors);
    Read(wall, sizeof(wall[0])*numwalls);
    Read(sprite, sizeof(sprite));
    Read(&randomseed, sizeof(randomseed));
    Read(&parallaxtype, sizeof(parallaxtype));
    Read(&showinvisibility, sizeof(showinvisibility));
    // PORT-TODO:
    // Read(&parallaxyoffs, sizeof(parallaxyoffs));
    // Read(&parallaxyscale, sizeof(parallaxyscale));
    Read(&g_visibility, sizeof(g_visibility));
    Read(&parallaxvisibility, sizeof(parallaxvisibility));
    // PORT-TODO:
    // Read(pskyoff, sizeof(pskyoff));
    // Read(&pskybits, sizeof(pskybits));
    Read(headspritesect, sizeof(headspritesect));
    Read(headspritestat, sizeof(headspritestat));
    Read(prevspritesect, sizeof(prevspritesect));
    Read(prevspritestat, sizeof(prevspritestat));
    Read(nextspritesect, sizeof(nextspritesect));
    Read(nextspritestat, sizeof(nextspritestat));
    Read(show2dsector, sizeof(show2dsector));
    Read(show2dwall, sizeof(show2dwall));
    Read(show2dsprite, sizeof(show2dsprite));
    Read(&automapping, sizeof(automapping));
    Read(gotpic, sizeof(gotpic));
    Read(gotsector, sizeof(gotsector));
    Read(&gFrameClock, sizeof(gFrameClock));
    Read(&gFrameTicks, sizeof(gFrameTicks));
    Read(&gFrame, sizeof(gFrame));
    int nGameClock;
    Read(&nGameClock, sizeof(nGameClock));
    gGameClock = nGameClock;
    Read(&gPaused, sizeof(gPaused));
    Read(&gbAdultContent, sizeof(gbAdultContent));
    Read(baseWall, sizeof(baseWall[0])*numwalls);
    Read(baseSprite, sizeof(baseSprite[0])*nNumSprites);
    Read(baseFloor, sizeof(baseFloor[0])*numsectors);
    Read(baseCeil, sizeof(baseCeil[0])*numsectors);
    Read(velFloor, sizeof(velFloor[0])*numsectors);
    Read(velCeil, sizeof(velCeil[0])*numsectors);
    Read(&gHitInfo, sizeof(gHitInfo));
    Read(&byte_1A76C6, sizeof(byte_1A76C6));
    Read(&byte_1A76C8, sizeof(byte_1A76C8));
    Read(&byte_1A76C7, sizeof(byte_1A76C7));
    Read(&byte_19AE44, sizeof(byte_19AE44));
    Read(gStatCount, sizeof(gStatCount));
    Read(nextXSprite, sizeof(nextXSprite));
    Read(nextXWall, sizeof(nextXWall));
    Read(nextXSector, sizeof(nextXSector));
    memset(xsprite, 0, sizeof(xsprite));
    for (int nSprite = 0; nSprite < kMaxSprites; nSprite++)
    {
        if (sprite[nSprite].statnum < kMaxStatus)
        {
            int nXSprite = sprite[nSprite].extra;
            if (nXSprite > 0)
                Read(&xsprite[nXSprite], sizeof(XSPRITE));
        }
    }
    memset(xwall, 0, sizeof(xwall));
    for (int nWall = 0; nWall < numwalls; nWall++)
    {
        int nXWall = wall[nWall].extra;
        if (nXWall > 0)
            Read(&xwall[nXWall], sizeof(XWALL));
    }
    memset(xsector, 0, sizeof(xsector));
    for (int nSector = 0; nSector < numwalls; nSector++)
    {
        int nXSector = sector[nSector].extra;
        if (nXSector > 0)
            Read(&xsector[nXSector], sizeof(XSECTOR));
    }
    Read(xvel, nNumSprites*sizeof(xvel[0]));
    Read(yvel, nNumSprites*sizeof(yvel[0]));
    Read(zvel, nNumSprites*sizeof(zvel[0]));
    Read(&gMapRev, sizeof(gMapRev));
    Read(&gSongId, sizeof(gSkyCount));
    Read(&gFogMode, sizeof(gFogMode));
    gCheatMgr.sub_5BCF4();
}

void MyLoadSave::Save(void)
{
    int nNumSprites = 0;
    int id = 'DULB';
    Write(&id, sizeof(id));
    short version = BloodVersion;
    Write(&version, sizeof(version));
    int release = 4;
    Write(&release, sizeof(release));
    for (int nSprite = 0; nSprite < kMaxSprites; nSprite++)
    {
        if (sprite[nSprite].statnum < kMaxStatus && nSprite > nNumSprites)
            nNumSprites = nSprite;
    }
    nNumSprites += 2;
    Write(&gGameOptions, sizeof(gGameOptions));
    Write(&numsectors, sizeof(numsectors));
    Write(&numwalls, sizeof(numwalls));
    Write(&numsectors, sizeof(numsectors));
    Write(&nNumSprites, sizeof(nNumSprites));
    Write(sector, sizeof(sector[0])*numsectors);
    Write(wall, sizeof(wall[0])*numwalls);
    Write(sprite, sizeof(sprite));
    Write(&randomseed, sizeof(randomseed));
    Write(&parallaxtype, sizeof(parallaxtype));
    Write(&showinvisibility, sizeof(showinvisibility));
    // PORT-TODO:
    // Write(&parallaxyoffs, sizeof(parallaxyoffs));
    // Write(&parallaxyscale, sizeof(parallaxyscale));
    Write(&g_visibility, sizeof(g_visibility));
    Write(&parallaxvisibility, sizeof(parallaxvisibility));
    // PORT-TODO:
    // Write(pskyoff, sizeof(pskyoff));
    // Write(&pskybits, sizeof(pskybits));
    Write(headspritesect, sizeof(headspritesect));
    Write(headspritestat, sizeof(headspritestat));
    Write(prevspritesect, sizeof(prevspritesect));
    Write(prevspritestat, sizeof(prevspritestat));
    Write(nextspritesect, sizeof(nextspritesect));
    Write(nextspritestat, sizeof(nextspritestat));
    Write(show2dsector, sizeof(show2dsector));
    Write(show2dwall, sizeof(show2dwall));
    Write(show2dsprite, sizeof(show2dsprite));
    Write(&automapping, sizeof(automapping));
    Write(gotpic, sizeof(gotpic));
    Write(gotsector, sizeof(gotsector));
    Write(&gFrameClock, sizeof(gFrameClock));
    Write(&gFrameTicks, sizeof(gFrameTicks));
    Write(&gFrame, sizeof(gFrame));
    int nGameClock = gGameClock;
    Write(&nGameClock, sizeof(nGameClock));
    Write(&gPaused, sizeof(gPaused));
    Write(&gbAdultContent, sizeof(gbAdultContent));
    Write(baseWall, sizeof(baseWall[0])*numwalls);
    Write(baseSprite, sizeof(baseSprite[0])*nNumSprites);
    Write(baseFloor, sizeof(baseFloor[0])*numsectors);
    Write(baseCeil, sizeof(baseCeil[0])*numsectors);
    Write(velFloor, sizeof(velFloor[0])*numsectors);
    Write(velCeil, sizeof(velCeil[0])*numsectors);
    Write(&gHitInfo, sizeof(gHitInfo));
    Write(&byte_1A76C6, sizeof(byte_1A76C6));
    Write(&byte_1A76C8, sizeof(byte_1A76C8));
    Write(&byte_1A76C7, sizeof(byte_1A76C7));
    Write(&byte_19AE44, sizeof(byte_19AE44));
    Write(gStatCount, sizeof(gStatCount));
    Write(nextXSprite, sizeof(nextXSprite));
    Write(nextXWall, sizeof(nextXWall));
    Write(nextXSector, sizeof(nextXSector));
    for (int nSprite = 0; nSprite < kMaxSprites; nSprite++)
    {
        if (sprite[nSprite].statnum < kMaxStatus)
        {
            int nXSprite = sprite[nSprite].extra;
            if (nXSprite > 0)
                Write(&xsprite[nXSprite], sizeof(XSPRITE));
        }
    }
    for (int nWall = 0; nWall < numwalls; nWall++)
    {
        int nXWall = wall[nWall].extra;
        if (nXWall > 0)
            Write(&xwall[nXWall], sizeof(XWALL));
    }
    for (int nSector = 0; nSector < numwalls; nSector++)
    {
        int nXSector = sector[nSector].extra;
        if (nXSector > 0)
            Write(&xsector[nXSector], sizeof(XSECTOR));
    }
    Write(xvel, nNumSprites*sizeof(xvel[0]));
    Write(yvel, nNumSprites*sizeof(yvel[0]));
    Write(zvel, nNumSprites*sizeof(zvel[0]));
    Write(&gMapRev, sizeof(gMapRev));
    Write(&gSongId, sizeof(gSkyCount));
    Write(&gFogMode, sizeof(gFogMode));
}

void LoadSavedInfo(void)
{
    // PORT-TODO:
#if 0
    struct find_t find;
    int nStatus = _dos_findfirst("GAME*.SAV", 0, &find);
    int nCount = 0;
    while (!nStatus && nCount < 10)
    {
        int hFile = open(find.name, 0x200);
        if (hFile == -1)
            ThrowError("File error #%d loading save file header.", errno);
        int vc, v8;
        short v4;
        vc = 0;
        v8 = 0;
        v4 = word_27AA54;
        if (read(hFile, &vc, sizeof(vd)) == -1)
        {
            close(hFile);
            nCount++;
            nStatus = _dos_findnext(&find);
            continue;
        }
        if (vc != 'DULB')
        {
            close(hFile);
            nCount++;
            nStatus = _dos_findnext(&find);
            continue;
        }
        read(hFile, &v4, sizeof(v4));
        if (v4 != BloodVersion)
        {
            close(hFile);
            nCount++;
            nStatus = _dos_findnext(&find);
            continue;
        }
        read(hFile, &v8, sizeof(v8));
        vc = 4;
        if (v8 != vc)
        {
            close(hFile);
            nCount++;
            nStatus = _dos_findnext(&find);
            continue;
        }
        if (read(hFile, &gSaveGameOptions[nCount], sizeof(gSaveGameOptions[0])) == -1)
            ThrowError("File error #%d reading save file.", errno);
        strcpy(strRestoreGameStrings[gSaveGameOptions[nCount].nSaveGameSlot], gSaveGameOptions[nCount].szUserGameName);
        close(hFile);
        nCount++;
        nStatus = _dos_findnext(&find);
    }
#endif
}

void UpdateSavedInfo(int nSlot)
{
    strcpy(strRestoreGameStrings[gSaveGameOptions[nSlot].nSaveGameSlot], gSaveGameOptions[nSlot].szUserGameName);
}

static MyLoadSave myLoadSave;
