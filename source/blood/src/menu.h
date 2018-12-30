#pragma once
#include "gamemenu.h"

extern CGameMenu menuMain;
extern CGameMenu menuMainWithSave;
extern CGameMenu unk_26B820;
extern CGameMenu unk_26B8B0;
extern CGameMenu unk_26B940;
extern CGameMenu unk_26B9D0;
extern CGameMenu menuOptions;
extern CGameMenu unk_26BAF0;
extern CGameMenu unk_26BB80;
extern CGameMenu unk_26BC10;
extern CGameMenu menuSaveGame;
extern CGameMenu menuLoadGame;
extern CGameMenu unk_26BDC0;
extern CGameMenu menuSounds;
extern CGameMenu menuQuit;
extern CGameMenu menuCredits;
extern CGameMenu menuOrder;
extern CGameMenu unk_26C090;
extern CGameMenu unk_26C120;
extern CGameMenu unk_26C1B0;
extern CGameMenu unk_26C240;
extern short gQuickLoadSlot;
extern short gQuickSaveSlot;
extern char strRestoreGameStrings[][16];
void sub_5A828(void);
void SetupMenus(void);
void QuickSaveGame(void);
void QuickLoadGame(void);