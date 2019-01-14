#pragma once
#include "gamemenu.h"

extern CGameMenu menuMain;
extern CGameMenu menuMainWithSave;
extern CGameMenu menuNetMain;
extern CGameMenu menuNetStart;
extern CGameMenu menuEpisode;
extern CGameMenu menuDifficulty;
extern CGameMenu menuOptions;
extern CGameMenu menuControls;
extern CGameMenu menuMessages;
extern CGameMenu menuKeys;
extern CGameMenu menuSaveGame;
extern CGameMenu menuLoadGame;
extern CGameMenu menuLoading;
extern CGameMenu menuSounds;
extern CGameMenu menuQuit;
extern CGameMenu menuCredits;
extern CGameMenu menuOrder;
extern CGameMenu menuPlayOnline;
extern CGameMenu menuParentalLock;
extern CGameMenu menuSorry;
extern CGameMenu menuSorry2;
extern short gQuickLoadSlot;
extern short gQuickSaveSlot;
extern char strRestoreGameStrings[][16];
void sub_5A828(void);
void SetupMenus(void);
void QuickSaveGame(void);
void QuickLoadGame(void);