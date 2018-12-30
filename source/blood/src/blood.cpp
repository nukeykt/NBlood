#include "build.h"
#include "mmulti.h"
#include "compat.h"
#include "renderlayer.h"
#include "common.h"
#include "common_game.h"
#include "gamedefs.h"

#include "asound.h"
#include "db.h"
#include "blood.h"
#include "choke.h"
#include "config.h"
#include "controls.h"
#include "credits.h"
#include "demo.h"
#include "dude.h"
#include "endgame.h"
#include "eventq.h"
#include "fire.h"
#include "fx.h"
#include "getopt.h"
#include "globals.h"
#include "levels.h"
#include "loadsave.h"
#include "menu.h"
#include "mirrors.h"
#include "music.h"
#include "network.h"
#include "osdcmds.h"
#include "replace.h"
#include "resource.h"
#include "qheap.h"
#include "screen.h"
#include "sectorfx.h"
#include "seq.h"
#include "sfx.h"
#include "sound.h"
#include "tile.h"
#include "trig.h"
#include "triggers.h"
#include "view.h"
#include "warp.h"
#include "weapon.h"

#ifdef _WIN32
# include <shellapi.h>
# define UPDATEINTERVAL 604800 // 1w
# include "winbits.h"
#else
# ifndef GEKKO
#  include <sys/ioctl.h>
# endif
#endif /* _WIN32 */

const char* AppProperName = APPNAME;
const char* AppTechnicalName = APPBASENAME;

// NUKE-TODO: make portable
EDUKE32_STATIC_ASSERT(sizeof(SPRITE) == sizeof(spritetype));
EDUKE32_STATIC_ASSERT(sizeof(SECTOR) == sizeof(sectortype));
EDUKE32_STATIC_ASSERT(sizeof(WALL) == sizeof(walltype));
EDUKE32_STATIC_ASSERT(sizeof(XSPRITE) == 56);
EDUKE32_STATIC_ASSERT(sizeof(XSECTOR) == 60);
EDUKE32_STATIC_ASSERT(sizeof(XWALL) == 24);


SECTOR *qsector;
SPRITE *qsprite, *qtsprite;
WALL *qwall;
PICANM *qpicanm;

ud_setup_t gSetup;
char SetupFilename[BMAX_PATH] = SETUPFILENAME;
int32_t gNoSetup = 0;

Resource gSysRes, gGuiRes;

INPUT_MODE gInputMode;

unsigned int nMaxAlloc = 0x2000000;

bool bCustomName = false;
char bAddUserMap = false;
bool bNoDemo = false;
bool bQuickStart = true;

char gUserMapFilename[BMAX_PATH];
char gPName[MAXPLAYERNAME];

short BloodVersion = 0x115;

int gNetPlayers;

char *pUserTiles = NULL;
char *pUserSoundRFF = NULL;
char *pUserRFF = NULL;

void app_crashhandler(void)
{
    // NUKE-TODO:
}

void G_Polymer_UnInit(void)
{
    // NUKE-TODO:
}

void M32RunScript(const char UNUSED(*s))
{
}

static const char *_module;
static int _line;

void _SetErrorLoc(const char *pzFile, int nLine)
{
    _module = pzFile;
    _line = nLine;
}

void _ThrowError(const char *pzFormat, ...)
{
    char buffer[256];
    va_list args;
    va_start(args, pzFormat);
    vsprintf(buffer, pzFormat, args);
    initprintf("%s(%i): %s\n", _module, _line, buffer);

    char titlebuf[256];
    Bsprintf(titlebuf, APPNAME " %s", s_buildRev);
    wm_msgbox(titlebuf, "%s(%i): %s\n", _module, _line, buffer);

    Bfflush(NULL);
    exit(0);
}

void __dassert(const char * pzExpr, const char * pzFile, int nLine)
{
    initprintf("Assertion failed: %s in file %s at line %i\n", pzExpr, pzFile, nLine);

    char titlebuf[256];
    Bsprintf(titlebuf, APPNAME " %s", s_buildRev);
    wm_msgbox(titlebuf, "Assertion failed: %s in file %s at line %i\n", pzExpr, pzFile, nLine);

    Bfflush(NULL);
    exit(0);
}



void sub_1053c(SPRITE *pSprite)
{
	DUDEINFO *pDudeInfo = &dudeInfo[pSprite->type-kDudeBase];
	seqPreloadId(pDudeInfo->seqStartID);
	seqPreloadId(pDudeInfo->seqStartID+5);
	seqPreloadId(pDudeInfo->seqStartID+1);
	seqPreloadId(pDudeInfo->seqStartID+2);
	switch (pSprite->type)
	{
	case 201:
	case 202:
	case 247:
	case 248:
		seqPreloadId(pDudeInfo->seqStartID+6);
		seqPreloadId(pDudeInfo->seqStartID+7);
		seqPreloadId(pDudeInfo->seqStartID+8);
		seqPreloadId(pDudeInfo->seqStartID+9);
		seqPreloadId(pDudeInfo->seqStartID+13);
		seqPreloadId(pDudeInfo->seqStartID+14);
		seqPreloadId(pDudeInfo->seqStartID+15);
		break;
	case 204:
	case 217:
		seqPreloadId(pDudeInfo->seqStartID+6);
		seqPreloadId(pDudeInfo->seqStartID+7);
		seqPreloadId(pDudeInfo->seqStartID+8);
		seqPreloadId(pDudeInfo->seqStartID+9);
		seqPreloadId(pDudeInfo->seqStartID+10);
		seqPreloadId(pDudeInfo->seqStartID+11);
		break;
	case 208:
	case 209:
		seqPreloadId(pDudeInfo->seqStartID+6);
		seqPreloadId(pDudeInfo->seqStartID+6);
	case 206:
	case 207:
		seqPreloadId(pDudeInfo->seqStartID+6);
		seqPreloadId(pDudeInfo->seqStartID+7);
		seqPreloadId(pDudeInfo->seqStartID+8);
		seqPreloadId(pDudeInfo->seqStartID+9);
		break;
	case 210:
	case 211:
	case 213:
	case 214:
	case 215:
	case 216:
	case 229:
		seqPreloadId(pDudeInfo->seqStartID+6);
		seqPreloadId(pDudeInfo->seqStartID+7);
		seqPreloadId(pDudeInfo->seqStartID+8);
		break;
	case 227:
		seqPreloadId(pDudeInfo->seqStartID+6);
		seqPreloadId(pDudeInfo->seqStartID+7);
	case 212:
	case 218:
	case 219:
	case 220:
		seqPreloadId(pDudeInfo->seqStartID+6);
		seqPreloadId(pDudeInfo->seqStartID+7);
		break;
	case 249:
        seqPreloadId(pDudeInfo->seqStartID+6);
		break;
	case 205:
        seqPreloadId(pDudeInfo->seqStartID+12);
        seqPreloadId(pDudeInfo->seqStartID+9);
	case 244:
        seqPreloadId(pDudeInfo->seqStartID+10);
	case 203:
        seqPreloadId(pDudeInfo->seqStartID+6);
        seqPreloadId(pDudeInfo->seqStartID+7);
        seqPreloadId(pDudeInfo->seqStartID+8);
        seqPreloadId(pDudeInfo->seqStartID+11);
        seqPreloadId(pDudeInfo->seqStartID+13);
        seqPreloadId(pDudeInfo->seqStartID+14);
		break;
	}
}

void sub_1081c(SPRITE *pSprite)
{
	switch (pSprite->type)
	{
	case 406:
	case 407:
        seqPreloadId(12);
		break;
	case 410:
        seqPreloadId(15);
		break;
	case 411:
        seqPreloadId(21);
		break;
	case 412:
        seqPreloadId(25);
        seqPreloadId(26);
		break;
	case 413:
        seqPreloadId(38);
        seqPreloadId(40);
        seqPreloadId(28);
		break;
	case 416:
		break;
	default:
		tilePreloadTile(pSprite->picnum);
		break;
	}
    seqPreloadId(3);
    seqPreloadId(4);
    seqPreloadId(5);
    seqPreloadId(9);
}

void PreloadTiles(void)
{
	int skyTile = -1;
	memset(gotpic,0,sizeof(gotpic));
	for (int i = 0; i < numsectors; i++)
	{
		tilePreloadTile2(sector[i].floorpicnum);
		tilePreloadTile2(sector[i].ceilingpicnum);
		if ((sector[i].ceilingstat&1) != 0 && skyTile == -1)
			skyTile = sector[i].ceilingpicnum;
	}
	for (int i = 0; i < numwalls; i++)
	{
		tilePreloadTile2(wall[i].picnum);
		if (wall[i].overpicnum >= 0)
			tilePreloadTile2(wall[i].overpicnum);
	}
	for (int i = 0; i < kMaxSprites; i++)
	{
		if (sprite[i].statnum < kMaxStatus)
		{
			SPRITE *pSprite = &qsprite[i];
			switch (pSprite->statnum)
			{
			case 6:
				sub_1053c(pSprite);
				break;
			case 4:
				sub_1081c(pSprite);
				break;
			default:
				tilePreloadTile2(pSprite->picnum);
				break;
			}
		}
	}
	if (numplayers > 1)
	{
		seqPreloadId(dudeInfo[31].seqStartID+6);
		seqPreloadId(dudeInfo[31].seqStartID+7);
		seqPreloadId(dudeInfo[31].seqStartID+8);
		seqPreloadId(dudeInfo[31].seqStartID+9);
		seqPreloadId(dudeInfo[31].seqStartID+10);
		seqPreloadId(dudeInfo[31].seqStartID+14);
		seqPreloadId(dudeInfo[31].seqStartID+15);
		seqPreloadId(dudeInfo[31].seqStartID+12);
		seqPreloadId(dudeInfo[31].seqStartID+16);
		seqPreloadId(dudeInfo[31].seqStartID+17);
		seqPreloadId(dudeInfo[31].seqStartID+18);
	}
	if (skyTile > -1 && skyTile < kMaxTiles)
	{
		for (int i = 1; i < gSkyCount; i++)
			tilePreloadTile2(skyTile+i);
	}
	netGetPackets();
}

void PreloadCache(void)
{
	PreloadTiles();
	for (int i = 0; i < kMaxTiles; i++)
	{
		if (gotpic[i>>3]&(1<<(i&7)))
		{
			tileLoadTile(i);
			netGetPackets();
		}
	}
	memset(gotpic,0,sizeof(gotpic));
}

void EndLevel(void)
{
	gViewPos = VIEWPOS_0;
	gGameMessageMgr.Clear();
	sndKillAllSounds();
	sfxKillAllSounds();
	ambKillAll();
	seqKillAll();
    // PORT-TODO:
	//if (gRedBookInstalled)
	//	Redbook.StopSong();
}

PLAYER gPlayerTemp[kMaxPlayers];
int gHealthTemp[kMaxPlayers];

vec3_t startpos;
int16_t startang, startsectnum;

void StartLevel(GAMEOPTIONS *gameOptions)
{
	EndLevel();
	gStartNewGame = 0;
	ready2send = 0;
	if (gDemo.at0 && gGameStarted)
		gDemo.Close();
	netWaitForEveryone(0);
	if (gGameOptions.nGameType == 0)
	{
		if (!(gGameOptions.uGameFlags&1))
			levelSetupOptions(gGameOptions.nEpisode, gGameOptions.nLevel);
		if (gEpisodeInfo[gGameOptions.nEpisode].cutALevel == gGameOptions.nLevel
			&& gEpisodeInfo[gGameOptions.nEpisode].at8f08)
			gGameOptions.uGameFlags |= 4;
		if ((gGameOptions.uGameFlags&4) && gDemo.at1 == 0)
			levelPlayIntroScene(gGameOptions.nEpisode);
	}
	else if (gGameOptions.nGameType > 0 && !(gGameOptions.uGameFlags&1))
	{
		gGameOptions.nEpisode = gPacketStartGame.episodeId;
		gGameOptions.nLevel = gPacketStartGame.levelId;
		gGameOptions.nGameType = gPacketStartGame.gameType;
		gGameOptions.nDifficulty = gPacketStartGame.difficulty;
		gGameOptions.nMonsterSettings = gPacketStartGame.monsterSettings;
		gGameOptions.nWeaponSettings = gPacketStartGame.weaponSettings;
		gGameOptions.nItemSettings = gPacketStartGame.itemSettings;
		gGameOptions.nRespawnSettings = gPacketStartGame.respawnSettings;
		if (gPacketStartGame.userMap)
			levelAddUserMap(gPacketStartGame.userMapName);
		else
			levelSetupOptions(gGameOptions.nEpisode, gGameOptions.nLevel);
	}
	if (gameOptions->uGameFlags&1)
	{
		for (int i = connecthead; i >= 0; i = connectpoint2[i])
		{
			memcpy(&gPlayerTemp[i],&gPlayer[i],sizeof(PLAYER));
			gHealthTemp[i] = xsprite[gPlayer[i].pSprite->extra].health;
		}
	}
	memset(xsprite,0,sizeof(xsprite));
	memset(sprite,0,sizeof(sprite));
	sub_5A828();
	dbLoadMap(gameOptions->zLevelName,(long*)&startpos.x,(long*)&startpos.y,(long*)&startpos.z,&startang,&startsectnum,(unsigned long*)&gameOptions->uMapCRC);
	srand(gameOptions->uMapCRC);
	gKillMgr.Clear();
	gSecretMgr.Clear();
	automapping = 1;
	for (int i = 0; i < kMaxSprites; i++)
	{
		SPRITE *pSprite = &qsprite[i];
		if (pSprite->statnum < kMaxStatus && pSprite->extra > 0)
		{
			XSPRITE *pXSprite = &xsprite[pSprite->extra];
			if ((pXSprite->ate_7&(1<<gameOptions->nDifficulty))
			|| (pXSprite->atf_4 && gameOptions->nGameType == 0)
			|| (pXSprite->atf_5 && gameOptions->nGameType == 2)
			|| (pXSprite->atb_7 && gameOptions->nGameType == 3)
			|| (pXSprite->atf_6 && gameOptions->nGameType == 1))
				DeleteSprite(i);
		}
	}
	scrLoadPLUs();
	startpos.z = getflorzofslope(startpos.x,startpos.y,startsectnum);
	for (int i = 0; i < kMaxPlayers; i++)
	{
		gStartZone[i].x = startpos.x;
		gStartZone[i].y = startpos.y;
		gStartZone[i].z = startpos.z;
		gStartZone[i].sectnum = startsectnum;
		gStartZone[i].ang = startang;
	}
	InitSectorFX();
	warpInit();
	actInit();
	evInit();
	for (int i = connecthead; i >= 0; i = connectpoint2[i])
	{
		if (!(gameOptions->uGameFlags&1))
		{
			if (numplayers == 1)
			{
				gProfile[i].skill = gSkill;
				gProfile[i].at0 = 1; //byte_13b838;
			}
			playerInit(i,0);
		}
		playerStart(i);
	}
	if (gameOptions->uGameFlags&1)
	{
		for (int i = connecthead; i >= 0; i = connectpoint2[i])
		{
			PLAYER *pPlayer = &gPlayer[i];
			pPlayer->pXSprite->health &= 0xf000;
			pPlayer->pXSprite->health |= gHealthTemp[i];
			pPlayer->at26 = gPlayerTemp[i].at26;
			pPlayer->atbd = gPlayerTemp[i].atbd;
			pPlayer->atc3 = gPlayerTemp[i].atc3;
			pPlayer->atc7 = gPlayerTemp[i].atc7;
			pPlayer->at2a = gPlayerTemp[i].at2a;
			pPlayer->at1b1 = gPlayerTemp[i].at1b1;
			pPlayer->atbf = gPlayerTemp[i].atbf;
			pPlayer->atbe = gPlayerTemp[i].atbe;
		}
	}
	gameOptions->uGameFlags &= ~3;
	scrSetDac();
	PreloadCache();
	InitMirrors();
	gFrameClock = 0;
	trInit();
	ambInit();
	sub_79760();
	gCacheMiss = 0;
	gFrame = 0;
	if (!gDemo.at1)
		gGameMenuMgr.Deactivate();
    // PORT-TODO;
	//if (!gRedBookInstalled)
		sndPlaySong(gGameOptions.zLevelSong,1);
	//if (!bNoCDAudio && gRedBookInstalled && Redbook.preprocess() && !gDemo.at1)
	//{
	//	Redbook.playsong(gGameOptions.at12a);
	//	Redbook.cd_status();
	//	Redbook.sub_82bb4();
	//}
	viewSetMessage("");
	viewSetErrorMessage("");
	viewResizeView(gViewSize);
	if (gGameOptions.nGameType == 3)
		gGameMessageMgr.SetCoordinates(gViewX0S+1,gViewY0S+15);
	netWaitForEveryone(0);
	gGameClock = 0;
	gPaused = 0;
	gGameStarted = 1;
	ready2send = 1;
}

void StartNetworkLevel(void)
{
	if (gDemo.at0)
		gDemo.Close();
	if (!(gGameOptions.uGameFlags&1))
	{
		gGameOptions.nEpisode = gPacketStartGame.episodeId;
		gGameOptions.nLevel = gPacketStartGame.levelId;
		gGameOptions.nGameType = gPacketStartGame.gameType;
		gGameOptions.nDifficulty = gPacketStartGame.difficulty;
		gGameOptions.nMonsterSettings = gPacketStartGame.monsterSettings;
		gGameOptions.nWeaponSettings = gPacketStartGame.weaponSettings;
		gGameOptions.nItemSettings = gPacketStartGame.itemSettings;
		gGameOptions.nRespawnSettings = gPacketStartGame.respawnSettings;
		if (gPacketStartGame.userMap)
			levelAddUserMap(gPacketStartGame.userMapName);
		else
			levelSetupOptions(gGameOptions.nEpisode, gGameOptions.nLevel);
	}
	StartLevel(&gGameOptions);
}

void LocalKeys(void)
{
    char buffer[128];
	static char name[16];
	static int timer = 0;
	char alt = keystatus[sc_LeftAlt] | keystatus[sc_RightAlt];
	char ctrl = keystatus[sc_LeftControl] | keystatus[sc_RightControl];
	char shift = keystatus[sc_LeftShift] | keystatus[sc_RightShift];
	if (BUTTON(gamefunc_See_Chase_View) && !alt && !shift)
	{
		CONTROL_ClearButton(gamefunc_See_Chase_View);
		if (gViewPos > VIEWPOS_0)
			gViewPos = VIEWPOS_0;
		else
			gViewPos = VIEWPOS_1;
	}
	if (BUTTON(gamefunc_See_Coop_View))
	{
		CONTROL_ClearButton(gamefunc_See_Coop_View);
		if (gGameOptions.nGameType == 1)
		{
			gViewIndex = connectpoint2[gViewIndex];
			if (gViewIndex == -1)
				gViewIndex = connecthead;
			gView = &gPlayer[gViewIndex];
		}
		else if (gGameOptions.nGameType == 3)
		{
			int oldViewIndex = gViewIndex;
			do
			{
				gViewIndex = connectpoint2[gViewIndex];
				if (gViewIndex == -1)
					gViewIndex = connecthead;
				if (oldViewIndex == gViewIndex || gMe->at2ea == gPlayer[gViewIndex].at2ea)
					break;
			} while (oldViewIndex != gViewIndex);
			gView = &gPlayer[gViewIndex];
		}
	}
	char key;
	if (key = keyGetScan())
	{
		if ((alt || shift) && gGameOptions.nGameType > 0 && key >= 0x3b && key <= 0x44)
		{
			char fk = key - 0x3b;
			if (alt)
			{
				netBroadcastTaunt(myconnectindex, fk);
			}
			else
			{
				gPlayerMsg.Set(CommbatMacro[fk]);
				gPlayerMsg.Send();
			}
			keyFlushScans();
			keystatus[key] = 0;
			CONTROL_ClearButton(41);
			return;
		}
		switch (key)
		{
        case 0x53:
		case 0xd3:
			if (ctrl && alt)
			{
				gQuitGame = 1;
				return;
			}
			break;
		case 0x01:
            keyFlushScans();
			if (gGameStarted && gPlayer[myconnectindex].pXSprite->health != 0)
			{
				if (!gGameMenuMgr.m_bActive)
					gGameMenuMgr.Push(&menuMainWithSave,-1);
			}
			else
			{
				if (!gGameMenuMgr.m_bActive)
					gGameMenuMgr.Push(&menuMain,-1);
			}
			return;
		case 0x3b:
            keyFlushScans();
			if (gGameOptions.nGameType == 0)
				gGameMenuMgr.Push(&menuOrder,-1);
			break;
		case 0x3c:
            keyFlushScans();
			if (!gGameMenuMgr.m_bActive && gGameOptions.nGameType == 0)
				gGameMenuMgr.Push(&menuSaveGame,-1);
			break;
		case 0x3d:
            keyFlushScans();
			if (!gGameMenuMgr.m_bActive && gGameOptions.nGameType == 0)
				gGameMenuMgr.Push(&menuLoadGame,-1);
			break;
		case 0x3e:
            keyFlushScans();
			if (!gGameMenuMgr.m_bActive)
				gGameMenuMgr.Push(&menuSounds,-1);
			return;
		case 0x3f:
            keyFlushScans();
			if (!gGameMenuMgr.m_bActive)
				gGameMenuMgr.Push(&menuOptions,-1);
			return;
		case 0x40:
            keyFlushScans();
			if (gGameStarted && !gGameMenuMgr.m_bActive && gPlayer[myconnectindex].pXSprite->health != 0)
			{
				if (gQuickSaveSlot != -1)
				{
					QuickSaveGame();
					return;
				}
				gGameMenuMgr.Push(&menuSaveGame,-1);
			}
			break;
		case 0x42:
            keyFlushScans();
			gGameMenuMgr.Push(&menuOptions,-1);
			break;
		case 0x43:
            keyFlushScans();
			if (!gGameMenuMgr.m_bActive)
			{
				if (gQuickLoadSlot != -1)
				{
					QuickLoadGame();
					return;
				}
				if (gQuickLoadSlot == -1 && gQuickSaveSlot != -1)
				{
					gQuickLoadSlot = gQuickSaveSlot;
					QuickLoadGame();
					return;
				}
				gGameMenuMgr.Push(&menuLoadGame,-1);
			}
			break;
		case 0x44:
            keyFlushScans();
			if (!gGameMenuMgr.m_bActive)
				gGameMenuMgr.Push(&menuQuit,-1);
			break;
		case 0x57:
			if (gGamma == gGammaLevels-1)
				gGamma = 0;
			else
				gGamma = ClipHigh(gGamma+1,gGammaLevels-1);
			scrSetGamma(gGamma);

			sprintf(buffer,"Gamma correction level %i",gGamma);
			viewSetMessage(buffer);
			break;
		case 0x58:
			if (name[0] == 0)
			{
				int i = 0;
				do
				{
					sprintf(name,"SS%02d0000.PCX",i);
					if (access(name, F_OK) == -1)
						break;
					i++;
				} while (1);
			}
            videoCaptureScreen(name, 0);
			break;
		}
	}
	int powerCount;
	if (powerCount = powerupCheck(gView,28))
	{
		int tilt1 = 170, tilt2 = 170, pitch = 20;
		timer += 4;
		if (powerCount < 512)
		{
			int powerScale = (powerCount<<16) / 512;
			tilt1 = mulscale16(tilt1, powerScale);
			tilt2 = mulscale16(tilt2, powerScale);
			pitch = mulscale16(pitch, powerScale);
		}
		int sin2 = costable[(2*timer-512)&2047] / 2;
		int sin3 = costable[(3*timer-512)&2047] / 2;
		gScreenTilt = mulscale30(sin2+sin3,tilt1);
		int sin4 = costable[(4*timer-512)&2047] / 2;
		deliriumTurn = mulscale30(sin3+sin4,tilt2);
		int sin5 = costable[(5*timer-512)&2047] / 2;
		deliriumPitch = mulscale30(sin4+sin5,pitch);
		return;
	}
	gScreenTilt = ((gScreenTilt+1024)&2047)-1024;
	if (gScreenTilt > 0)
	{
		gScreenTilt -= 8;
		if (gScreenTilt < 0)
			gScreenTilt = 0;
	}
	else if (gScreenTilt < 0)
	{
		gScreenTilt += 8;
		if (gScreenTilt >= 0)
			gScreenTilt = 0;
	}
}

bool gRestartGame = false;

void ProcessFrame(void)
{
    char buffer[128];
	if (gDemo.at0)
		gDemo.Write(gFifoInput[gNetFifoTail&255]);
	for (int i = connecthead; i >= 0; i = connectpoint2[i])
	{
		gPlayer[i].atc.buttonFlags = gFifoInput[gNetFifoTail&255][i].buttonFlags;
		gPlayer[i].atc.keyFlags.word |= gFifoInput[gNetFifoTail&255][i].keyFlags.word;
		gPlayer[i].atc.useFlags.byte |= gFifoInput[gNetFifoTail&255][i].useFlags.byte;
		if (gFifoInput[gNetFifoTail&255][i].newWeapon)
			gPlayer[i].atc.newWeapon = gFifoInput[gNetFifoTail&255][i].newWeapon;
		gPlayer[i].atc.forward = gFifoInput[gNetFifoTail&255][i].forward;
		gPlayer[i].atc.turn = gFifoInput[gNetFifoTail&255][i].turn;
		gPlayer[i].atc.strafe = gFifoInput[gNetFifoTail&255][i].strafe;
		gPlayer[i].atc.mlook = gFifoInput[gNetFifoTail&255][i].mlook;
	}
	gNetFifoTail++;
	if (!(gFrame&((gSyncRate<<3)-1)))
	{
		CalcGameChecksum();
		memcpy(gCheckFifo[gCheckHead[myconnectindex]&255][myconnectindex], gChecksum, sizeof(gChecksum));
		gCheckHead[myconnectindex]++;
	}
	for (int i = connecthead; i >= 0; i = connectpoint2[i])
	{
		if (gPlayer[i].atc.keyFlags.quit && i == myconnectindex)
		{
            gPlayer[i].atc.keyFlags.quit = 0;
			netBroadcastMyLogoff();
			return;
		}
		if (gPlayer[i].atc.keyFlags.restart)
		{
            gPlayer[i].atc.keyFlags.restart = 0;
			levelRestart();
			return;
		}
		if (gPlayer[i].atc.keyFlags.pause)
		{
            gPlayer[i].atc.keyFlags.pause = 0;
			gPaused = !gPaused;
			if (gPaused && gGameOptions.nGameType > 0 && numplayers > 1)
			{
				sprintf(buffer,"%s paused the game",gProfile[i].name);
				viewSetMessage(buffer);
			}
		}
	}
	viewClearInterpolations();
	if (!gDemo.at1)
	{
		if (gPaused || gEndGameMgr.at0 || (gGameOptions.nGameType == 0 && gGameMenuMgr.m_bActive))
			return;
	}
	for (int i = connecthead; i >= 0; i = connectpoint2[i])
	{
		viewBackupView(i);
		playerProcess(&gPlayer[i]);
	}
	trProcessBusy();
	evProcess(gFrameClock);
	seqProcess(4);
	DoSectorPanning();
	actProcessSprites();
	actPostProcess();
	viewCorrectPrediction();
	sndProcess();
	ambProcess();
	sfxUpdate3DSounds();
	gFrame++;
	gFrameClock += 4;
	if ((gGameOptions.uGameFlags&1) != 0 && !gStartNewGame)
	{
		ready2send = 0;
		if (gDemo.at0)
			gDemo.Close();
		sndFadeSong(4000);
		seqKillAll();
		if (gGameOptions.uGameFlags&2)
		{
			if (gGameOptions.nGameType == 0)
			{
				if (gGameOptions.uGameFlags&8)
					levelPlayEndScene(gGameOptions.nEpisode);
				gGameMenuMgr.Deactivate();
				gGameMenuMgr.Push(&menuCredits,-1);
			}
			gGameOptions.uGameFlags &= ~3;
			gRestartGame = 1;
			gQuitGame = 1;
		}
		else
		{
			gEndGameMgr.Setup();
			viewResizeView(gViewSize);
		}
	}
}

SWITCH switches[] = {
    { "?", 0, 0 },
    { "broadcast", 1, 0 },
    { "map", 2, 1 },
    { "masterslave", 3, 0 },
    //{ "net", 4, 1 },
    { "nodudes", 5, 1 },
    { "playback", 6, 1 },
    { "record", 7, 1 },
    { "robust", 8, 0 },
    { "setupfile", 9, 1 },
    { "skill", 10, 1 },
    { "nocd", 11, 0 },
    { "8250", 12, 0 },
    { "ini", 13, 1 },
    { "noaim", 14, 0 },
    { "f", 15, 1 },
    { "control", 16, 1 },
    { "vector", 17, 1 },
    { "quick", 18, 0 },
    //{ "getopt", 19, 1 },
    //{ "auto", 20, 1 },
    { "pname", 21, 1 },
    { "noresend", 22, 0 },
    { "silentaim", 23, 0 },
    { "nodemo", 25, 0 },
    { "art", 26, 1 },
    { "snd", 27, 1 },
    { "rff", 28, 1 },
    { "maxalloc", 29, 1 },
    { "server", 30, 1 },
    { "client", 31, 1 },
    { 0 }
};

void PrintHelp(void)
{
    puts("Blood Command-line Options:");
    // NUKE-TODO:
    puts("-?            This help");
    puts("-8250         Enforce obsolete UART I/O");
    puts("-auto         Automatic Network start. Implies -quick");
    puts("-getopt       Use network game options from file.  Implies -auto");
    puts("-broadcast    Set network to broadcast packet mode");
    puts("-masterslave  Set network to master/slave packet mode");
    puts("-net          Net mode game");
    puts("-noaim        Disable auto-aiming");
    puts("-nocd         Disable CD audio");
    puts("-nodudes      No monsters");
    puts("-nodemo       No Demos");
    puts("-robust       Robust network sync checking");
    puts("-skill        Set player handicap; Range:0..4; Default:2; (NOT difficulty level.)");
    puts("-quick        Skip Intro screens and get right to the game");
    puts("-pname        Override player name setting from config file");
    puts("-map          Specify a user map");
    puts("-playback     Play back a demo");
    puts("-record       Record a demo");
    puts("-art          Specify an art base file name");
    puts("-snd          Specify an RFF Sound file name");
    puts("-RFF          Specify an RFF file for Blood game resources");
    puts("-ini          Specify an INI file name (default is blood.ini)");
    exit(0);
}

void ParseOptions(void)
{
    int option;
    while ((option = GetOptions(switches)) != -1)
    {
        switch (option)
        {
        case -3:
            ThrowError("Invalid argument: %s", OptFull);
        case 29:
            if (OptArgc < 1)
                ThrowError("Missing argument");
            nMaxAlloc = atoi(OptArgv[0]);
            if (!nMaxAlloc)
                nMaxAlloc = 0x2000000;
            break;
        case 0:
            PrintHelp();
            break;
        //case 19:
        //    byte_148eec = 1;
        //case 20:
        //    if (OptArgc < 1)
        //        ThrowError("Missing argument");
        //    strncpy(byte_148ef0, OptArgv[0], 13);
        //    byte_148ef0[12] = 0;
        //    bQuickStart = 1;
        //    byte_148eeb = 1;
        //    if (gGameOptions.gameType == 0)
        //        gGameOptions.gameType = 2;
        //    break;
        case 25:
            bNoDemo = 1;
            break;
        case 18:
            bQuickStart = 1;
            break;
        //case 12:
        //    EightyTwoFifty = 1;
        //    break;
        case 1:
            gPacketMode = PACKETMODE_2;
            break;
        case 21:
            if (OptArgc < 1)
                ThrowError("Missing argument");
            strcpy(gPName, OptArgv[0]);
            bCustomName = 1;
            break;
        case 2:
            if (OptArgc < 1)
                ThrowError("Missing argument");
            strcpy(gUserMapFilename, OptArgv[0]);
            bAddUserMap = 1;
            bNoDemo = 1;
            break;
        case 3:
            if (gSyncRate == 1)
                gPacketMode = PACKETMODE_2;
            else
                gPacketMode = PACKETMODE_1;
            break;
        case 4:
            //if (OptArgc < 1)
            //    ThrowError("Missing argument");
            //if (gGameOptions.nGameType == 0)
            //    gGameOptions.nGameType = 2;
            break;
        case 30:
            if (OptArgc < 1)
                ThrowError("Missing argument");
            gNetPlayers = ClipRange(atoi(OptArgv[0]), 1, kMaxPlayers);
            if (gGameOptions.nGameType == 0)
                gGameOptions.nGameType = 2;
            gNetMode = NETWORK_SERVER;
            break;
        case 31:
            if (OptArgc < 1)
                ThrowError("Missing argument");
            if (gGameOptions.nGameType == 0)
                gGameOptions.nGameType = 2;
            gNetMode = NETWORK_CLIENT;
            strncpy(gNetAddress, OptArgv[0], sizeof(gNetAddress)-1);
            break;
        case 14:
            gAutoAim = 0;
            break;
        case 22:
            bNoResend = 0;
            break;
        case 23:
            bSilentAim = 1;
            break;
        case 5:
            gGameOptions.nMonsterSettings = 0;
            break;
        case 6:
            if (OptArgc < 1)
                gDemo.SetupPlayback(NULL);
            else
                gDemo.SetupPlayback(OptArgv[0]);
            break;
        case 7:
            if (OptArgc < 1)
                gDemo.Create(NULL);
            else
                gDemo.Create(OptArgv[0]);
            break;
        case 8:
            gRobust = 1;
            break;
        case 13:
            if (OptArgc < 1)
                ThrowError("Missing argument");
            sub_269D8(OptArgv[0]);
            bNoDemo = 1;
            break;
        case 26:
            if (OptArgc < 1)
                ThrowError("Missing argument");
            pUserTiles = (char*)malloc(strlen(OptArgv[0])+1);
            if (!pUserTiles)
                return;
            strcpy(pUserTiles, OptArgv[0]);
            break;
        case 27:
            if (OptArgc < 1)
                ThrowError("Missing argument");
            pUserSoundRFF = (char*)malloc(strlen(OptArgv[0])+1);
            if (!pUserSoundRFF)
                return;
            strcpy(pUserSoundRFF, OptArgv[0]);
            break;
        case 28:
            if (OptArgc < 1)
                ThrowError("Missing argument");
            pUserRFF = (char*)malloc(strlen(OptArgv[0])+1);
            if (!pUserRFF)
                return;
            strcpy(pUserRFF, OptArgv[0]);
            break;
        case 9:
            if (OptArgc < 1)
                ThrowError("Missing argument");
            strcpy(SetupFilename, OptArgv[0]);
            break;
        case 10:
            if (OptArgc < 1)
                ThrowError("Missing argument");
            gSkill = strtoul(OptArgv[0], NULL, 0);
            if (gSkill < 0)
                gSkill = 0;
            else if (gSkill > 4)
                gSkill = 4;
            break;
        case 15:
            if (OptArgc < 1)
                ThrowError("Missing argument");
            gSyncRate = ClipRange(strtoul(OptArgv[0], NULL, 0), 1, 4);
            if (gPacketMode == PACKETMODE_1)
                gSyncRate = 1;
            else if (gPacketMode == PACKETMODE_3)
                gSyncRate = 1;
            break;
        case -2:
            strcpy(gUserMapFilename, OptFull);
            bAddUserMap = 1;
            bNoDemo = 1;
            break;
        case 11:
            //bNoCDAudio = 1;
            break;
        }
    }
    if (bAddUserMap)
    {
        char zNode[BMAX_PATH];
        char zDir[BMAX_PATH];
        char zFName[BMAX_PATH];
        _splitpath(gUserMapFilename, zNode, zDir, zFName, NULL);
        strcpy(UserPath, zNode);
        strcat(UserPath, zDir);
        strcpy(gUserMapFilename, zFName);
    }
}

void ClockStrobe()
{
    if (handleevents() && quitevent)
    {
        KB_KeyDown[sc_Escape] = 1;
        quitevent = 0;
    }
    MUSIC_Update();
    gGameClock++;
}

int app_main(int argc, char const * const * argv)
{
    char buffer[BMAX_PATH];
    margc = argc;
    margv = argv;
#ifdef _WIN32
    if (!G_CheckCmdSwitch(argc, argv, "-noinstancechecking") && win_checkinstance())
    {
        if (!wm_ynbox(APPNAME, "Another Build game is currently running. "
                      "Do you wish to continue starting this copy?"))
            return 3;
    }

    backgroundidle = 0;

#ifdef DEBUGGINGAIDS
    extern int32_t (*check_filename_casing_fn)(void);
    check_filename_casing_fn = check_filename_casing;
#endif
#endif

    OSD_SetLogFile(APPBASENAME ".log");

    wm_setapptitle(APPNAME);

    initprintf(APPNAME " %s\n", s_buildRev);
    PrintBuildInfo();

    memcpy(&gGameOptions, &gSingleGameOptions, sizeof(GAMEOPTIONS));
    ParseOptions();
    sub_26988();

    // used with binds for fast function lookup
    hash_init(&h_gamefuncs);
    for (bssize_t i=NUMGAMEFUNCTIONS-1; i>=0; i--)
    {
        if (gamefunctions[i][0] == '\0')
            continue;

        char *str = Bstrtolower(Xstrdup(gamefunctions[i]));
        hash_add(&h_gamefuncs,gamefunctions[i],i,0);
        hash_add(&h_gamefuncs,str,i,0);
        Bfree(str);
    }
    
#ifdef STARTUP_SETUP_WINDOW
    int const readSetup =
#endif
    CONFIG_ReadSetup();
    if (bCustomName)
        strcpy(szPlayerName, gPName);

    initprintf("Initializing OSD...\n");

    //Bsprintf(tempbuf, HEAD2 " %s", s_buildRev);
    OSD_SetVersion("Blood", 10, 0);
    OSD_SetParameters(0, 0, 0, 12, 2, 12, OSD_ERROR, OSDTEXT_RED, gamefunctions[gamefunc_Show_Console][0] == '\0' ? OSD_PROTECTED : 0);
    registerosdcommands();

    if (enginePreInit())
    {
        wm_msgbox("Build Engine Initialization Error",
                  "There was a problem initializing the Build engine: %s", engineerrstr);
        ERRprintf("app_main: There was a problem initializing the Build engine: %s\n", engineerrstr);
        Bexit(2);
    }

    if (Bstrcmp(SetupFilename, SETUPFILENAME))
        initprintf("Using config file \"%s\".\n", SetupFilename);

#ifdef STARTUP_SETUP_WINDOW
    if (readSetup < 0 || (/*!g_noSetup && */(configversion != BYTEVERSION || gSetup.forcesetup))/* || g_commandSetup*/)
    {
        if (quitevent || !startwin_run())
        {
            engineUnInit();
            Bexit(0);
        }
    }
#endif

    Resource::heap = new QHeap(nMaxAlloc);
    strcpy(buffer, UserPath);
    strcat(buffer, "*.map");
    if (pUserRFF)
        gSysRes.Init(pUserRFF, buffer);
    else
        gSysRes.Init("BLOOD.RFF", buffer);
    gGuiRes.Init("GUI.RFF", "*.*");


    { // Replace
        void qinitspritelists();
        int32_t qinsertsprite(int16_t nSector, int16_t nStat);
        int32_t qdeletesprite(int16_t nSprite);
        int32_t qchangespritesect(int16_t nSprite, int16_t nSector);
        int32_t qchangespritestat(int16_t nSprite, int16_t nStatus);
        animateoffs_replace = qanimateoffs;
        paletteLoadFromDisk_replace = qloadpalette;
        getpalookup_replace = qgetpalookup;
        initspritelists_replace = qinitspritelists;
        insertsprite_replace = qinsertsprite;
        deletesprite_replace = qdeletesprite;
        changespritesect_replace = qchangespritesect;
        changespritestat_replace = qchangespritestat;
        loadvoxel_replace = qloadvoxel;
        bloodhack = true;
    }

    initprintf("Initializing Build 3D engine\n");
    scrInit();

    qsprite = (SPRITE*)sprite;
    qwall = (WALL*)wall;
    qsector = (SECTOR*)sector;
    qpicanm = (PICANM*)picanm;
    qtsprite = (SPRITE*)tsprite;
    
    initprintf("Loading tiles\n");
	if (pUserTiles)
	{
		strcpy(buffer,pUserTiles);
		strcat(buffer,"000.ART");
		if (!tileInit(0,buffer))
			ThrowError("User specified ART files not found");
	}
	else
	{
		if (!tileInit(0,NULL))
			ThrowError("TILES###.ART files not found");
	}
    powerupInit();
    initprintf("Loading cosine table\n");
    trigInit(gSysRes);
    initprintf("Initializing view subsystem\n");
    viewInit();
    initprintf("Initializing dynamic fire\n");
    FireInit();
    initprintf("Initializing weapon animations\n");
    WeaponInit();
    LoadSavedInfo();
    gDemo.LoadDemoInfo();
    sprintf(buffer, "There are %d demo(s) in the loop\n", gDemo.at59ef);
    initprintf(buffer);
    if (!gDemo.at0 && gDemo.at59ef > 0 && gGameOptions.nGameType == 0 && !bNoDemo)
        gDemo.SetupPlayback(NULL);
    initprintf("Loading control setup\n");
    ctrlInit();
    timerInit(120);
    timerSetCallback(ClockStrobe);
    // PORT-TODO: CD audio init

    initprintf("Initializing network users\n");
    netInitialize();
    gViewIndex = myconnectindex;
    gMe = gView = &gPlayer[myconnectindex];
    // sub_2906C()
    netBroadcastPlayerInfo(myconnectindex);
    initprintf("Waiting for network players!\n");
    netWaitForEveryone(0);
    scrSetGameMode(gSetup.fullscreen, gSetup.xdim, gSetup.ydim, gSetup.bpp);
    scrSetGamma(gGamma);
    viewResizeView(gViewSize);
    initprintf("Initializing sound system\n");
    sndInit();
    sfxInit();
    gChoke.sub_83ff0(518, sub_84230);
    levelLoadDefaults();
    if (bAddUserMap)
    {
        levelAddUserMap(gUserMapFilename);
        gStartNewGame = 1;
    }
    SetupMenus();
    videoSetViewableArea(0, 0, xdim - 1, ydim - 1);
    if (!bQuickStart)
        credLogosDos();
    scrSetDac();
RESTART:
    scrSetGameMode(gSetup.fullscreen, gSetup.xdim, gSetup.ydim, gSetup.bpp);
    scrSetGamma(gGamma);
    gQuitGame = 0;
    gRestartGame = 0;
    if (gGameOptions.nGameType > 0)
    {
        KB_ClearKeysDown();
        KB_FlushKeyboardQueue();
        keyFlushScans();
    }
    else if (gDemo.at1 && !bAddUserMap && !bNoDemo)
        gDemo.Playback();
    if (gDemo.at59ef > 0)
        gGameMenuMgr.Deactivate();
    if (!bAddUserMap && !gGameStarted)
        gGameMenuMgr.Push(&menuMain, -1);
    ready2send = 1;
	while (!gQuitGame)
	{
        // PORT-TODO:
		//if (gRedBookInstalled)
		//	Redbook.preprocess();
        netUpdate();
        CONTROL_BindsEnabled = true;
		switch (gInputMode)
		{
		case INPUT_MODE_1:
			if (gGameMenuMgr.m_bActive)
				gGameMenuMgr.Process();
			break;
		case INPUT_MODE_0:
			LocalKeys();
			break;
		}
		if (gQuitGame)
			continue;
        // PORT-TODO:
		//if (gRedBookInstalled)
		//	Redbook.postprocess();
		if (gGameStarted)
		{
			if (numplayers > 1)
				netGetPackets();
			while (gPredictTail < gNetFifoHead[myconnectindex] && !gPaused)
			{
				viewUpdatePrediction(&gFifoInput[gPredictTail][myconnectindex]);
			}
			if (numplayers == 1)
				gBufferJitter = 0;
			while (gNetFifoHead[myconnectindex]-gNetFifoTail > gBufferJitter && !gStartNewGame && !gQuitGame)
			{
				int i;
				for (i = connecthead; i >= 0; i = connectpoint2[i])
					if (gNetFifoHead[i] == gNetFifoTail)
						break;
				if (i >= 0)
					break;
				faketimerhandler();
				ProcessFrame();
			}
			if (gQuitRequest && gQuitGame)
				videoClearScreen(0);
			else
			{
				netCheckSync();
				viewDrawScreen();
			}
		}
		else
		{
            videoClearScreen(0);
			rotatesprite(160<<16,100<<16,65536,0,2518,0,0,0x4a,0,0,xdim-1,ydim-1);
			netGetPackets();
			if (gQuitRequest && !gQuitGame)
				netBroadcastMyLogoff();
		}
		switch (gInputMode)
		{
		case INPUT_MODE_1:
			if (gGameMenuMgr.m_bActive)
				gGameMenuMgr.Draw();
			break;
		case INPUT_MODE_2:
			gPlayerMsg.ProcessKeys();
			gPlayerMsg.Draw();
			break;
		case INPUT_MODE_3:
			gEndGameMgr.ProcessKeys();
			gEndGameMgr.Draw();
			break;
		}
		scrNextPage();
		if (TestBitString(gotpic, 2342))
		{
			FireProcess();
            ClearBitString(gotpic, 2342);
		}
		//if (byte_148e29 && gStartNewGame)
		//{
		//	gStartNewGame = 0;
		//	gQuitGame = 1;
		//}
		if (gStartNewGame)
			StartLevel(&gGameOptions);
	}
    netDeinitialize();
    ready2send = 0;
    if (gDemo.at0)
        gDemo.Close();
    if (gRestartGame)
    {
		gQuitGame = 0;
		gRestartGame = 0;
		gGameStarted = 0;
		levelSetupOptions(0,0);
		while (gGameMenuMgr.m_bActive)
		{
			gGameMenuMgr.Process();
            videoClearScreen(0);
			netGetPackets();
			gGameMenuMgr.Draw();
			scrNextPage();
		}
		if (gGameOptions.nGameType != 0)
        {
		    if (!gDemo.at0 && gDemo.at59ef > 0 && gGameOptions.nGameType == 0 && !bNoDemo)
			    gDemo.NextDemo();
		    videoSetViewableArea(0,0,xdim-1,ydim-1);
		    if (!bQuickStart)
			    credLogosDos();
		    scrSetDac();
        }
        goto RESTART;
    }
    sndTerm();
    scrUnInit();
    // PORT_TODO: Check argument
    CONFIG_WriteSetup(0);
    if (syncstate)
        printf("A packet was lost! (syncstate)\n");
    for (int i = 0; i < 10; i++)
    {
        if (gSaveGamePic[i])
            Resource::Free(gSaveGamePic[i]);
    }
    // PORT-TODO:
    //if (gRedBookInstalled)
    //{
    //    Redbook.StopSong();
    //    Redbook.cdrom_shutdown();
    //}

    return 0;
}
