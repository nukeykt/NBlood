#pragma once
#include "fix16.h"
#include "common_game.h"
#include "actor.h"
#include "blood.h"
#include "controls.h"
#include "db.h"
#include "dude.h"

enum LifeMode {
    kModeHuman = 0,
    kModeBeast,
};

struct PACKINFO {
    char at0;
    int at1;
};

struct PLAYER {
    SPRITE *pSprite;
    XSPRITE *pXSprite;
    DUDEINFO *pDudeInfo;
    GINPUT atc;
    //short atc; // INPUT
    //char at10; // forward
    //short at11; // turn
    //char at13; // strafe
    //int at14; // buttonFlags
    //unsigned int at18; // keyFlags
    //char at1c; // useFlags;
    //char at20; // newWeapon
    //char at21; // mlook
    int at22;
    int at26; // weapon qav
    int at2a; // qav callback
    char at2e; // run
    int at2f; // state
    int at33; // unused?
    int at37;
    int at3b;
    int at3f; // bob height
    int at43; // bob width
    int at47;
    int at4b;
    int at4f; // bob sway y
    int at53; // bob sway x
    int at57; // Connect id
    int at5b; // spritenum
    int at5f; // life mode
    int at63;
    int at67; // view z
    int at6b;
    int at6f; // weapon z
    int at73;
    fix16_t q16look;
    int q16horiz; // horiz
    int q16slopehoriz; // horizoff
    int at83;
    char at87; // underwater
    char at88[8]; // keys
    char at90; // flag capture
    short at91[8];
    int ata1[7];
    char atbd; // weapon
    char atbe; // pending weapon
    int atbf, atc3, atc7;
    char atcb[14]; // hasweapon
    int atd9[14];
    int at111[2][14];
    //int at149[14];
    int at181[12]; // ammo
    char at1b1;
    int at1b2;
    int at1b6;
    int at1ba;
    Aim at1be; // world
    //int at1c6;
    Aim at1ca; // relative
    //int at1ca;
    //int at1ce;
    //int at1d2;
    int at1d6; // aim target sprite
    int at1da;
    short at1de[16];
    int at1fe;
    int at202[kMaxPowerUps];
    int at2c6; // frags
    int at2ca[8];
    int at2ea;
    int at2ee; // killer
    int at2f2;
    int at2f6;
    int at2fa;
    int at2fe;
    int at302;
    int at306;
    int at30a;
    int at30e;
    int at312;
    int at316;
    char at31a; // God mode
    char at31b; // Fall scream
    char at31c;
    int at31d; // pack timer
    int at321; // pack id
    PACKINFO packInfo[5]; // at325
    int at33e[3]; // armor
    //int at342;
    //int at346;
    int voodooTarget; // at34a
    int at34e;
    int at352;
    int at356;
    int at35a; // quake
    int at35e;
    int at362; // light
    int at366;
    int at36a; // blind
    int at36e; // choke
    int at372;
    char at376; // hand
    int at377;
    char at37b; // weapon flash
    int at37f; // quake2
    fix16_t q16ang;
    int angold;
};

struct POSTURE {
    int at0;
    int at4;
    int at8;
    int atc[2];
    int at14;
    int at18;
    int at1c;
    int at20;
    int at24;
    int at28;
    int at2c;
    int at30;
};

struct PROFILE {
    char at0;
    int skill;
    char name[12];
};

struct AMMOINFO {
    int at0;
    signed char at4;
};

extern POSTURE gPosture[][3];

extern PLAYER gPlayer[kMaxPlayers];
extern PLAYER *gMe, *gView;

extern PROFILE gProfile[kMaxPlayers];

extern int dword_21EFB0[kMaxPlayers];
extern int dword_21EFD0[kMaxPlayers];
extern AMMOINFO gAmmoInfo[];

int powerupCheck(PLAYER *pPlayer, int nPowerUp);
char powerupActivate(PLAYER *pPlayer, int nPowerUp);
void powerupDeactivate(PLAYER *pPlayer, int nPowerUp);
void powerupSetState(PLAYER *pPlayer, int nPowerUp, char bState);
void powerupProcess(PLAYER *pPlayer);
void powerupClear(PLAYER *pPlayer);
void powerupInit(void);
int packItemToPowerup(int nPack);
int powerupToPackItem(int nPowerUp);
char packAddItem(PLAYER *pPlayer, unsigned int nPack);
int packCheckItem(PLAYER *pPlayer, int nPack);
char packItemActive(PLAYER *pPlayer, int nPack);
void packUseItem(PLAYER *pPlayer, int nPack);
void packPrevItem(PLAYER *pPlayer);
void packNextItem(PLAYER *pPlayer);
char playerSeqPlaying(PLAYER * pPlayer, int nSeq);
void playerSetRace(PLAYER *pPlayer, int nLifeMode);
void playerSetGodMode(PLAYER *pPlayer, char bGodMode);
void playerResetInertia(PLAYER *pPlayer);
void playerStart(int nPlayer);
void playerReset(PLAYER *pPlayer);
void playerInit(int nPlayer, unsigned int a2);
char sub_3A158(PLAYER *a1, SPRITE *a2);
char PickupItem(PLAYER *pPlayer, SPRITE *pItem);
char PickupAmmo(PLAYER *pPlayer, SPRITE *pAmmo);
char PickupWeapon(PLAYER *pPlayer, SPRITE *pWeapon);
void PickUp(PLAYER *pPlayer, SPRITE *pSprite);
void CheckPickUp(PLAYER *pPlayer);
int ActionScan(PLAYER *pPlayer, int *a2, int *a3);
void ProcessInput(PLAYER *pPlayer);
void playerProcess(PLAYER *pPlayer);
SPRITE *playerFireMissile(PLAYER *pPlayer, int a2, long a3, long a4, long a5, int a6);
SPRITE *playerFireThing(PLAYER *pPlayer, int a2, int a3, int thingType, int a5);
void playerFrag(PLAYER *pKiller, PLAYER *pVictim);
void FragPlayer(PLAYER *pPlayer, int nSprite);
int playerDamageArmor(PLAYER *pPlayer, DAMAGE_TYPE nType, int nDamage);
SPRITE *sub_40A94(PLAYER *pPlayer, int a2);
int playerDamageSprite(int nSource, PLAYER *pPlayer, DAMAGE_TYPE nDamageType, int nDamage);
int UseAmmo(PLAYER *pPlayer, int nAmmoType, int nDec);
void sub_41250(PLAYER *pPlayer);
void playerLandingSound(PLAYER *pPlayer);
void PlayerSurvive(int, int nXSprite);
void PlayerKeelsOver(int, int nXSprite);
