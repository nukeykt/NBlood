#include <stdio.h>
#include <string.h>
#include "common.h"
#include "common_game.h"
#include "mmulti.h"
#ifdef WITHKPLIB
#include "kplib.h"
#endif

#include "blood.h"
#include "controls.h"
#include "demo.h"
#include "fire.h"
#include "gamemenu.h"
#include "globals.h"
#include "menu.h"
#include "messages.h"
#include "misc.h"
#include "network.h"
#include "player.h"
#include "screen.h"
#include "view.h"


int nBuild = 0;

CDemo gDemo;

CDemo::CDemo()
{
    nBuild = 4;
    at0 = 0;
    at1 = 0;
    at3 = 0;
    at7 = NULL;
    atb = 0;
    at59ef = 0;
    at59eb = 0;
    at2 = 0;
    memset(&atf, 0, sizeof(atf));
}

CDemo::~CDemo()
{
    at0 = 0;
    at1 = 0;
    at3 = 0;
    atb = 0;
    memset(&atf, 0, sizeof(atf));
    if (at7 != NULL)
    {
        fclose(at7);
        at7 = NULL;
    }
}

bool CDemo::Create(const char *pzFile)
{
    char buffer[13] = "";
    char vc = 0;
    if (at0 || at1)
        ThrowError("CDemo::Create called during demo record/playback process.");
    if (!pzFile)
    {
        for (int i = 0; i < 8 && !vc; i++)
        {
            sprintf(buffer, "BLOOD0%02d.DEM", i);
            if (!access(buffer, F_OK))
                vc = 1;
        }
        if (vc == 1)
        {
            at7 = fopen(buffer, "wb");
            if (at7 == NULL)
                return false;
        }
    }
    else
    {
        at7 = fopen(pzFile, "wb");
        if (at7 == NULL)
            return false;
    }
    at0 = 1;
    atb = 0;
    return true;
}

void CDemo::Write(GINPUT *pPlayerInputs)
{
    dassert(pPlayerInputs != NULL);
    if (!at0)
        return;
    if (atb == 0)
    {
        atf.signature = '\x1aMED';
        atf.nVersion = BloodVersion;
        atf.nBuild = nBuild;
        atf.nInputCount = 0;
        atf.nNetPlayers = gNetPlayers;
        atf.nMyConnectIndex = myconnectindex;
        atf.nConnectHead = connecthead;
        memcpy(atf.connectPoints, connectpoint2, sizeof(atf.connectPoints));
        memcpy(&atf.gameOptions, &gGameOptions, sizeof(gGameOptions));
        fwrite(&atf, sizeof(DEMOHEADER), 1, at7);
    }
    for (int p = connecthead; p >= 0; p = connectpoint2[p])
    {
        memcpy(&at1aa[atb&1023], &pPlayerInputs[p], sizeof(INPUT));
        atb++;
        if((atb&1023)==0)
            fwrite(at1aa, sizeof(GINPUT), 1024, at7);
    }
}

void CDemo::Close(void)
{
    if (at0)
    {
        if (atb&1023)
            fwrite(at1aa, sizeof(INPUT), (atb&1023), at7);
        atf.nInputCount = atb;
        fseek(at7, 0, SEEK_SET);
        fwrite(&atf, sizeof(DEMOHEADER), 1, at7);
    }
    if (at7 != NULL)
    {
        fclose(at7);
        at7 = NULL;
    }
    at0 = 0;
    at1 = 0;
}

bool CDemo::SetupPlayback(const char *pzFile)
{
    at0 = 0;
    at1 = 0;
    if (pzFile)
    {
        at7 = fopen(pzFile, "rb");
        if (at7 == NULL)
            return false;
    }
    else
    {
        at7 = fopen(at59aa[at59eb], "rb");
        if (at7 == NULL)
            return false;
    }
    fread(&atf, sizeof(DEMOHEADER), 1, at7);
    if (atf.signature != '\x1aMED')
        return 0;
    if (BloodVersion != atf.nVersion && BloodVersion == 0x10b && atf.nVersion != 0x10a)
        return 0;
    if (nBuild != atf.nBuild)
        return 0;
    at0 = 0;
    at1 = 1;
    return 1;
}

void CDemo::ProcessKeys(void)
{
    switch (gInputMode)
    {
    case INPUT_MODE_1:
        gGameMenuMgr.Process();
        break;
    case INPUT_MODE_2:
        gPlayerMsg.ProcessKeys();
        break;
    case INPUT_MODE_0:
    {
        char nKey;
        while ((nKey = keyGetScan()) != 0)
        {
	        char alt = keystatus[0x38] | keystatus[0xb8];
	        char ctrl = keystatus[0x1d] | keystatus[0x9d];
            switch (nKey)
            {
            case 1:
                if (!CGameMenuMgr::m_bActive)
                {
                    gGameMenuMgr.Push(&menuMain, -1);
                    at2 = 1;
                }
                break;
            case 0x58:
                gViewIndex = connectpoint2[gViewIndex];
                if (gViewIndex == -1)
                    gViewIndex = connecthead;
                gView = &gPlayer[gViewIndex];
                break;
            }
        }
        break;
    }
    }
}

void CDemo::Playback(void)
{
    ready2send = 0;
    int v4 = 0;
    if (!CGameMenuMgr::m_bActive)
    {
        gGameMenuMgr.Push(&menuMain, -1);
        at2 = 1;
    }
    gNetFifoClock = gGameClock;
    gViewMode = 3;
_DEMOPLAYBACK:
    while (at1 && !gQuitGame)
    {
        while (gGameClock >= gNetFifoClock && !gQuitGame)
        {
            if (!v4)
            {
                viewResizeView(gViewSize);
                viewSetMessage("");
                gNetPlayers = atf.nNetPlayers;
                atb = atf.nInputCount;
                myconnectindex = atf.nMyConnectIndex;
                connecthead = atf.nConnectHead;
                memcpy(connectpoint2, atf.connectPoints, sizeof(atf.connectPoints));
                memcpy(&gGameOptions, &atf.gameOptions, sizeof(GAMEOPTIONS));
                gSkill = gGameOptions.nDifficulty;
                for (int i = 0; i < 8; i++)
                    playerInit(i, 0);
                StartLevel(&gGameOptions);
            }
            ProcessKeys();
            for (int p = connecthead; p >= 0; p = connectpoint2[p])
            {
                if ((v4&1023) == 0)
                {
                    unsigned int nSize = sizeof(INPUT)*(atb-v4);
                    if (nSize > sizeof(at1aa))
                        nSize = sizeof(at1aa);
                    fread(at1aa, 1, nSize, at7);
                }
                memcpy(&gFifoInput[gNetFifoHead[p]&255], &at1aa[v4&1023], sizeof(INPUT));
                gNetFifoHead[p]++;
                v4++;
                if (v4 >= atf.nInputCount)
                {
                    ready2send = 0;
                    if (at59ef != 1)
                    {
                        v4 = 0;
                        Close();
                        NextDemo();
                        gNetFifoClock = gGameClock;
                        goto _DEMOPLAYBACK;
                    }
                    else
                    {
                        fseek(at7, sizeof(DEMOHEADER), SEEK_SET);
                        v4 = 0;
                    }
                }
            }
            gNetFifoClock += 4;
            if (!gQuitGame)
                ProcessFrame();
            ready2send = 0;
        }
        viewDrawScreen();
        if (gInputMode == INPUT_MODE_1 && CGameMenuMgr::m_bActive)
            gGameMenuMgr.Draw();
        scrNextPage();
        if (TestBitString(gotpic, 2342))
        {
            FireProcess();
            ClearBitString(gotpic, 2342);
        }
    }
    Close();
}

void CDemo::StopPlayback(void)
{
    at1 = 0;
}

void CDemo::LoadDemoInfo(void)
{
    at59ef = 0;
    // PORT-TODO:
#if 0
    struct find_t find;
    int status = _dos_findfirst("BLOOD*.DEM", &find);
    while (!status && at59ef < 5)
    {
        int hFile2 = open(find.name, 0x200);
        if (hFile2 == -1)
            ThrowError("File error #%d loading demo file header.", errno);
        read(hFile2, &atf, sizeof(atf));
        close(hFile2);
        if (atf.signature == '\x1aMED' && (atf.nVersion == BloodVersion || BloodVersion != 0x10b || atf.nVersion == 0x10a) && nBuild == atf.nBuild)
        {
            strcpy(at59aa[at59ef], find.name);
            at59ef++;
        }
        status = _dos_findnext(&find);
    }
#endif
}

void CDemo::NextDemo(void)
{
    at59eb++;
    if (at59eb >= at59ef)
        at59eb = 0;
    SetupPlayback(NULL);
}
