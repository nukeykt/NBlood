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

#include "save.h"
#include "sound.h"
#include "status.h"
#include "init.h"
#include "menu.h"
#include <stdio.h>
#include <assert.h>
#include "exhumed.h"

const char kSaveSig[] = "EXSV";
const uint16_t kSaveVersion = 9999;


void testsave()
{
    LoadSave::SaveGame("exsave");
    StatusMessage(300, "Saved game");
}

void testload()
{
    LoadSave::LoadGame("exsave");
    StatusMessage(300, "Loaded game");
}

LoadSave LoadSave::head(123);

buildvfs_fd LoadSave::hSFile = buildvfs_fd_invalid;
buildvfs_fd LoadSave::hLFile = buildvfs_fd_invalid;

void LoadSave::Save()
{
    bail2dos("Pure virtual function called");
}

void LoadSave::Load()
{
    bail2dos("Pure virtual function called");
}

void LoadSave::Read(void* pData, int nSize)
{
    assert(hLFile != buildvfs_fd_invalid);

    int nActualRead = buildvfs_read(hLFile, pData, nSize);
    if (nActualRead != nSize)
        bail2dos("Error reading save file.");
  //  if (buildvfs_read(hLFile, pData, nSize) != nSize)
   //     bail2dos("Error reading save file.");
}

void LoadSave::Write(void* pData, int nSize)
{
    assert(hSFile != buildvfs_fd_invalid);

    if (buildvfs_write(hSFile, pData, nSize) != nSize)
        bail2dos("Error writing save file");
}

int savegame(int nSlot)
{
    char filename[BMAX_PATH];

    if (nSlot < 0 || nSlot >= 10) {
        return 0;
    }

    sprintf(filename, "save%1d.gam", nSlot);

    buildvfs_fd hFile = buildvfs_open_write(filename);
    if (hFile != -1)
    {
#if 1 // TODO
        buildvfs_write(hFile, &numsectors, sizeof(numsectors));
        buildvfs_write(hFile, sector, sizeof(SECTOR) * numsectors);
        buildvfs_write(hFile, &numwalls, sizeof(numwalls));
        buildvfs_write(hFile, wall, sizeof(WALL) * numwalls);
        buildvfs_write(hFile, sprite, sizeof(SPRITE) * kMaxSprites);
        buildvfs_write(hFile, headspritesect, sizeof(headspritesect));
        buildvfs_write(hFile, prevspritesect, sizeof(prevspritesect));
        buildvfs_write(hFile, nextspritesect, sizeof(nextspritesect));
        buildvfs_write(hFile, headspritestat, sizeof(headspritestat));
        buildvfs_write(hFile, prevspritestat, sizeof(prevspritestat));
        buildvfs_write(hFile, nextspritestat, sizeof(nextspritestat));
        buildvfs_write(hFile, startumost, sizeof(startumost));
        buildvfs_write(hFile, startdmost, sizeof(startdmost));
/*
        buildvfs_write(hFile, &brightness, 2);
        buildvfs_write(hFile, &visibility, 4);
        buildvfs_write(hFile, &parallaxtype, 1);
        buildvfs_write(hFile, &parallaxyoffs, 4);
        buildvfs_write(hFile, pskyoff, 512);
        buildvfs_write(hFile, &pskybits, 2);
        buildvfs_write(hFile, &inita, 2);
        buildvfs_write(hFile, &initsect, 2);
        buildvfs_write(hFile, &initx, 4);
        buildvfs_write(hFile, &inity, 4);
        buildvfs_write(hFile, &initz, 4);
        buildvfs_write(hFile, &levelnum, 2);
*/
#endif
        buildvfs_close(hFile);
    }

    return 1; // CHECKME
}

int loadgame(int nSlot)
{
    char filename[BMAX_PATH];

    if (nSlot < 0 || nSlot >= 10) {
        return 0;
    }

    sprintf(filename, "save%1d.gam", nSlot);

    buildvfs_fd hFile = buildvfs_open_read(filename);
    if (hFile != -1)
    {
#if 1 // TODO
        buildvfs_read(hFile, &numsectors, sizeof(numsectors));
        buildvfs_read(hFile, sector, sizeof(SECTOR) * numsectors);
        buildvfs_read(hFile, &numwalls, sizeof(numwalls));
        buildvfs_read(hFile, wall, sizeof(WALL) * numwalls);
        buildvfs_read(hFile, sprite, sizeof(SPRITE) * kMaxSprites);
        buildvfs_read(hFile, headspritesect, sizeof(headspritesect));
        buildvfs_read(hFile, prevspritesect, sizeof(prevspritesect));
        buildvfs_read(hFile, nextspritesect, sizeof(nextspritesect));
        buildvfs_read(hFile, headspritestat, sizeof(headspritestat));
        buildvfs_read(hFile, prevspritestat, sizeof(prevspritestat));
        buildvfs_read(hFile, nextspritestat, sizeof(nextspritestat));
        buildvfs_read(hFile, startumost, sizeof(startumost));
        buildvfs_read(hFile, startdmost, sizeof(startdmost));

        #if 0
        buildvfs_read(hFile, &brightness, 2);
        buildvfs_read(hFile, &visibility, 4);
        buildvfs_read(hFile, &parallaxtype, 1);
        buildvfs_read(hFile, &parallaxyoffs, 4);
        buildvfs_read(hFile, pskyoff, 512);
        buildvfs_read(hFile, &pskybits, 2);
        buildvfs_read(hFile, &inita, 2);
        buildvfs_read(hFile, &initsect, 2);
        buildvfs_read(hFile, &initx, 4);
        buildvfs_read(hFile, &inity, 4);
        buildvfs_read(hFile, &initz, 4);
        buildvfs_read(hFile, &levelnum, 2);

        lPlayerXVel = 0;
        lPlayerYVel = 0;
        nPlayerDAng = 0;
        #endif
#endif
        close(hFile);
    }

    return 1; // CHECKME
}

class MyLoadSave : public LoadSave
{
public:
    virtual void Load();
    virtual void Save();
};

void LoadSave::LoadGame(const char* pzFile)
{
    int nSlot = 3; // TEMP
    char filename[BMAX_PATH];

    sprintf(filename, "save%1d.gam", nSlot);

    hLFile = buildvfs_open_read(filename);
    if (hLFile == buildvfs_fd_invalid) {
        bail2dos("Error loading save file %s.", filename);
    }

    char signature[4];
    buildvfs_read(hLFile, &signature, sizeof(signature));
    if (memcmp(&signature, kSaveSig, 4) != 0) {
        bail2dos("Invalid save file signature");
    }

    uint16_t version;
    buildvfs_read(hLFile, &version, 2);
    if (version != kSaveVersion) {
        bail2dos("Invalid save file version");
    }

    StopAllSounds();
    memset(sprite, 0, sizeof(spritetype) * kMaxSprites);
    automapping = 1;

    ClearStatusMessage();

    LoadSave* rover = head.next;
    while (rover != &head)
    {
        rover->Load();
        rover = rover->next;
    }

    buildvfs_close(hLFile);
    hLFile = buildvfs_fd_invalid;

#ifdef USE_STRUCT_TRACKERS
    Bmemset(sectorchanged, 0, sizeof(sectorchanged));
    Bmemset(spritechanged, 0, sizeof(spritechanged));
    Bmemset(wallchanged, 0, sizeof(wallchanged));
#endif

#ifdef USE_OPENGL
    Polymost_prepare_loadboard();
#endif

#ifdef POLYMER
    if (videoGetRenderMode() == REND_POLYMER)
        polymer_loadboard();

    // this light pointer nulling needs to be outside the videoGetRenderMode check
    // because we might be loading the savegame using another renderer but
    // change to Polymer later
    /*
    for (int i = 0; i < kMaxSprites; i++)
    {
        gPolymerLight[i].lightptr = NULL;
        gPolymerLight[i].lightId = -1;
    }
    */
#endif
}

void LoadSave::SaveGame(const char* pzFile)
{
    int nSlot = 3; // TEMP
    char filename[BMAX_PATH];

    sprintf(filename, "save%1d.gam", nSlot);

    hSFile = buildvfs_open_write(filename);
    if (hSFile == buildvfs_fd_invalid) {
        bail2dos("Error creating save file");
    }

    buildvfs_write(hSFile, kSaveSig, 4);
    buildvfs_write(hSFile, &kSaveVersion, 2);

    LoadSave* rover = head.next;
    while (rover != &head)
    {
        rover->Save();
        rover = rover->next;
    }

    buildvfs_close(hSFile);
    hSFile = buildvfs_fd_invalid;
}

void MyLoadSave::Load()
{
    psky_t* pSky = tileSetupSky(0);
    int numsprites = 0;

    Read(&GameStats, sizeof(GameStats));

    Read(&numsectors, sizeof(numsectors));
    Read(&numwalls,   sizeof(numwalls));

    Read(sector, sizeof(sector[0]) * numsectors);
    Read(wall,   sizeof(wall[0])   * numwalls);
    Read(sprite, sizeof(sprite[0]) * kMaxSprites);

    Read(&parallaxtype, sizeof(parallaxtype));
    Read(&showinvisibility, sizeof(showinvisibility));
    Read(&pSky->horizfrac,  sizeof(pSky->horizfrac));
    Read(&pSky->yoffs,  sizeof(pSky->yoffs));
    Read(&pSky->yscale, sizeof(pSky->yscale));
    Read(&parallaxvisibility, sizeof(parallaxvisibility));
    Read(pSky->tileofs, sizeof(pSky->tileofs));
    Read(&pSky->lognumtiles, sizeof(pSky->lognumtiles));

    Read(headspritesect, sizeof(headspritesect));
    Read(headspritestat, sizeof(headspritestat));
    Read(prevspritesect, sizeof(prevspritesect));
    Read(prevspritestat, sizeof(prevspritestat));
    Read(nextspritesect, sizeof(nextspritesect));
    Read(nextspritestat, sizeof(nextspritestat));
    Read(show2dsector, sizeof(show2dsector));
    Read(show2dwall,   sizeof(show2dwall));
    Read(show2dsprite, sizeof(show2dsprite));
    Read(&automapping, sizeof(automapping));
    Read(gotpic,    sizeof(gotpic));
    Read(gotsector, sizeof(gotsector));

    Read(&initx, sizeof(initx));
    Read(&inity, sizeof(inity));
    Read(&initz, sizeof(initz));
    Read(&inita, sizeof(inita));
    Read(&initsect, sizeof(initsect));

    Read(nBodyGunSprite, sizeof(nBodyGunSprite));
    Read(&movefifoend, sizeof(movefifoend));
    Read(&movefifopos, sizeof(movefifopos));
    Read(&nCurBodyGunNum, sizeof(nCurBodyGunNum));

    Read(&nAmbientChannel, sizeof(nAmbientChannel));
    Read(&nStopSound,  sizeof(nStopSound));
    Read(&nStoneSound, sizeof(nStoneSound));
    Read(&nSwitchSound, sizeof(nSwitchSound));
    Read(&nElevSound, sizeof(nElevSound));
    Read(&nCreepyTimer, sizeof(nCreepyTimer));
    Read(&nCreaturesLeft, sizeof(nCreaturesLeft));

    Read(&nClockVal, sizeof(nClockVal));
    Read(&nRedTicks, sizeof(nRedTicks));
    Read(&nAlarmTicks,  sizeof(nAlarmTicks));
    Read(&nButtonColor, sizeof(nButtonColor));
    Read(&nEnergyChan,  sizeof(nEnergyChan));
    Read(&lCountDown,   sizeof(lCountDown));
    Read(&nEnergyTowers, sizeof(nEnergyTowers));
    Read(&moveframes, sizeof(moveframes));
    Read(&flash, sizeof(flash));
    //Read(&localclock, sizeof(localclock));
    Read(&totalmoves, sizeof(totalmoves));
    Read(&tclocks,  sizeof(tclocks));
    Read(&tclocks2, sizeof(tclocks2));
    Read(&ototalclock, sizeof(ototalclock));
    Read(&totalclock,  sizeof(totalclock));

    Read(&nSnakeCam,  sizeof(nSnakeCam));
    Read(&nBestLevel, sizeof(nBestLevel));
    Read(&levelnew,   sizeof(levelnew));
    Read(&levelnum,   sizeof(levelnum));

#ifdef YAX_ENABLE
    Read(&numyaxbunches, sizeof(numyaxbunches));
#endif
    psky_t skyInfo;
    Read(&skyInfo, sizeof(skyInfo));

    *tileSetupSky(0) = skyInfo;
}

void MyLoadSave::Save()
{
    psky_t* pSky = tileSetupSky(0);

    Write(&GameStats, sizeof(GameStats));

    Write(&numsectors, sizeof(numsectors));
    Write(&numwalls,   sizeof(numwalls));

    Write(sector, sizeof(sector[0]) * numsectors);
    Write(wall,   sizeof(wall[0])   * numwalls);
    Write(sprite, sizeof(sprite[0]) * kMaxSprites);

    Write(&parallaxtype, sizeof(parallaxtype));
    Write(&showinvisibility, sizeof(showinvisibility));
    Write(&pSky->horizfrac,  sizeof(pSky->horizfrac));
    Write(&pSky->yoffs,  sizeof(pSky->yoffs));
    Write(&pSky->yscale, sizeof(pSky->yscale));
    Write(&parallaxvisibility, sizeof(parallaxvisibility));
    Write(pSky->tileofs, sizeof(pSky->tileofs));
    Write(&pSky->lognumtiles, sizeof(pSky->lognumtiles));

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
    Write(gotpic,    sizeof(gotpic));
    Write(gotsector, sizeof(gotsector));

    Write(&initx, sizeof(initx));
    Write(&inity, sizeof(inity));
    Write(&initz, sizeof(initz));
    Write(&inita, sizeof(inita));
    Write(&initsect, sizeof(initsect));

    Write(nBodyGunSprite, sizeof(nBodyGunSprite));
    Write(&movefifoend, sizeof(movefifoend));
    Write(&movefifopos, sizeof(movefifopos));
    Write(&nCurBodyGunNum, sizeof(nCurBodyGunNum));

    Write(&nAmbientChannel, sizeof(nAmbientChannel));
    Write(&nStopSound,   sizeof(nStopSound));
    Write(&nStoneSound,  sizeof(nStoneSound));
    Write(&nSwitchSound, sizeof(nSwitchSound));
    Write(&nElevSound, sizeof(nElevSound));
    Write(&nCreepyTimer, sizeof(nCreepyTimer));
    Write(&nCreaturesLeft, sizeof(nCreaturesLeft));

    Write(&nClockVal, sizeof(nClockVal));
    Write(&nRedTicks, sizeof(nRedTicks));
    Write(&nAlarmTicks,  sizeof(nAlarmTicks));
    Write(&nButtonColor, sizeof(nButtonColor));
    Write(&nEnergyChan,  sizeof(nEnergyChan));
    Write(&lCountDown, sizeof(lCountDown));
    Write(&nEnergyTowers, sizeof(nEnergyTowers));
    Write(&moveframes, sizeof(moveframes));
    Write(&flash, sizeof(flash));
    //Write(&localclock, sizeof(localclock));
    Write(&totalmoves, sizeof(totalmoves));
    Write(&tclocks,  sizeof(tclocks));
    Write(&tclocks2, sizeof(tclocks2));
    Write(&ototalclock, sizeof(ototalclock));
    Write(&totalclock,  sizeof(totalclock));

    Write(&nSnakeCam,  sizeof(nSnakeCam));
    Write(&nBestLevel, sizeof(nBestLevel));
    Write(&levelnew,   sizeof(levelnew));
    Write(&levelnum,   sizeof(levelnum));

#ifdef YAX_ENABLE
    Write(&numyaxbunches, sizeof(numyaxbunches));
#endif
    psky_t skyInfo = *tileSetupSky(0);
    Write(&skyInfo, sizeof(skyInfo));
}

static MyLoadSave* myLoadSave;

/*
AiFunc aiFunctions[kFuncMax] = {
    FuncElev,
    FuncWallFace,
    FuncSlide,

    FuncCreatureChunk,
    FuncObject,
    FuncTrap,
    FuncEnergyBlock,
    FuncSpark,
};
*/

void LoadSaveSetup()
{
    void ObjectLoadSaveConstruct();
    void SwitchLoadSaveConstruct();
    void AnubisLoadSaveConstruct();
    void PlayerLoadSaveConstruct();
    void SpiderLoadSaveConstruct();
    void WaspLoadSaveConstruct();
    void RunListLoadSaveConstruct();
    void RatLoadSaveConstruct();
    void BubbleLoadSaveConstruct();
    void MummyLoadSaveConstruct();
    void LionLoadSaveConstruct();
    void FishLoadSaveConstruct();
    void LavaDudeLoadSaveConstruct();
    void GunLoadSaveConstruct();
    void StatusLoadSaveConstruct();
    void ViewLoadSaveConstruct();
    void AnimsLoadSaveConstruct();
    void BulletLoadSaveConstruct();
    void GrenadeLoadSaveConstruct();
    void RandomLoadSaveConstruct();
    void MoveLoadSaveConstruct();
    void ItemsLoadSaveConstruct();
    void RaLoadSaveConstruct();
    void RexLoadSaveConstruct();
    void RoachLoadSaveConstruct();
    void ScorpLoadSaveConstruct();
    void SetLoadSaveConstruct();
    void QueenLoadSaveConstruct();
    void SnakeLoadSaveConstruct();

    void LightingLoadSaveConstruct();

    myLoadSave = new MyLoadSave();

    ObjectLoadSaveConstruct();
    SwitchLoadSaveConstruct();
    MoveLoadSaveConstruct();
    LightingLoadSaveConstruct();

    RandomLoadSaveConstruct();
    AnimsLoadSaveConstruct();
    StatusLoadSaveConstruct();
    ViewLoadSaveConstruct();
    ItemsLoadSaveConstruct();

    PlayerLoadSaveConstruct();
    GunLoadSaveConstruct();
    BulletLoadSaveConstruct();
    GrenadeLoadSaveConstruct();

    AnubisLoadSaveConstruct();
    SpiderLoadSaveConstruct();
    WaspLoadSaveConstruct();
    RunListLoadSaveConstruct();
    RatLoadSaveConstruct();
    MummyLoadSaveConstruct();
    LionLoadSaveConstruct();
    FishLoadSaveConstruct();
    LavaDudeLoadSaveConstruct();
    RexLoadSaveConstruct();
    RoachLoadSaveConstruct();
    ScorpLoadSaveConstruct();
    SetLoadSaveConstruct();
    QueenLoadSaveConstruct();
    RaLoadSaveConstruct();
    SnakeLoadSaveConstruct();
    BubbleLoadSaveConstruct();
}
