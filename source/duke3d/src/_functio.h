//-------------------------------------------------------------------------
/*
Copyright (C) 2010 EDuke32 developers and contributors

This file is part of EDuke32.

EDuke32 is free software; you can redistribute it and/or
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

// _functio.h

// file created by makehead.exe
// these headers contain default key assignments, as well as
// default button assignments and game function names
// axis defaults are also included

#include "_control.h"
#include "control.h"

#ifndef function_private_h_
#define function_private_h_
#ifdef __cplusplus
extern "C" {
#endif
char gamefunctions[NUMGAMEFUNCTIONS][MAXGAMEFUNCLEN] =
   {
   "Move_Forward",
   "Move_Backward",
   "Turn_Left",
   "Turn_Right",
   "Strafe",
   "Fire",
   "Open",
   "Run",
   "Alt_Fire",
   "Jump",
   "Crouch",
   "Look_Up",
   "Look_Down",
   "Look_Left",
   "Look_Right",
   "Strafe_Left",
   "Strafe_Right",
   "Aim_Up",
   "Aim_Down",
   "Weapon_1",
   "Weapon_2",
   "Weapon_3",
   "Weapon_4",
   "Weapon_5",
   "Weapon_6",
   "Weapon_7",
   "Weapon_8",
   "Weapon_9",
   "Weapon_10",
   "Inventory",
   "Inventory_Left",
   "Inventory_Right",
#ifndef EDUKE32_STANDALONE
   "Holo_Duke",
   "Jetpack",
   "NightVision",
   "MedKit",
#else
   "",
   "",
   "",
   "",
#endif
   "TurnAround",
   "SendMessage",
   "Map",
   "Shrink_Screen",
   "Enlarge_Screen",
   "Center_View",
   "Holster_Weapon",
   "Show_Opponents_Weapon",
   "Map_Follow_Mode",
   "See_Coop_View",
   "Mouse_Aiming",
   "Toggle_Crosshair",
#ifndef EDUKE32_STANDALONE
   "Steroids",
   "Quick_Kick",
#else
   "",
   "",
#endif
   "Next_Weapon",
   "Previous_Weapon",
   "Show_Console",
   "Show_Scoreboard",
   "Dpad_Select",
   "Dpad_Aiming",
   "AutoRun",
   "Last_Used_Weapon",
   "Quick_Save",
   "Quick_Load",
   "Alt_Weapon",
   "Third_Person_View",
   "Toggle_Crouch",
   };

// note: internal ordering is important, must not be changed
const char internal_gamefunction_names[NUMGAMEFUNCTIONS][MAXGAMEFUNCLEN] =
   {
   "gamefunc_Move_Forward",
   "gamefunc_Move_Backward",
   "gamefunc_Turn_Left",
   "gamefunc_Turn_Right",
   "gamefunc_Strafe",
   "gamefunc_Fire",
   "gamefunc_Open",
   "gamefunc_Run",
   "gamefunc_Alt_Fire",
   "gamefunc_Jump",
   "gamefunc_Crouch",
   "gamefunc_Look_Up",
   "gamefunc_Look_Down",
   "gamefunc_Look_Left",
   "gamefunc_Look_Right",
   "gamefunc_Strafe_Left",
   "gamefunc_Strafe_Right",
   "gamefunc_Aim_Up",
   "gamefunc_Aim_Down",
   "gamefunc_Weapon_1",
   "gamefunc_Weapon_2",
   "gamefunc_Weapon_3",
   "gamefunc_Weapon_4",
   "gamefunc_Weapon_5",
   "gamefunc_Weapon_6",
   "gamefunc_Weapon_7",
   "gamefunc_Weapon_8",
   "gamefunc_Weapon_9",
   "gamefunc_Weapon_10",
   "gamefunc_Inventory",
   "gamefunc_Inventory_Left",
   "gamefunc_Inventory_Right",
#ifndef EDUKE32_STANDALONE
   "gamefunc_Holo_Duke",
   "gamefunc_Jetpack",
   "gamefunc_NightVision",
   "gamefunc_MedKit",
#else
   "gamefunc_Item_1",
   "gamefunc_Item_2",
   "gamefunc_Item_3",
   "gamefunc_Item_4",
#endif
   "gamefunc_TurnAround",
   "gamefunc_SendMessage",
   "gamefunc_Map",
   "gamefunc_Shrink_Screen",
   "gamefunc_Enlarge_Screen",
   "gamefunc_Center_View",
   "gamefunc_Holster_Weapon",
   "gamefunc_Show_Opponents_Weapon",
   "gamefunc_Map_Follow_Mode",
   "gamefunc_See_Coop_View",
   "gamefunc_Mouse_Aiming",
   "gamefunc_Toggle_Crosshair",
#ifndef EDUKE32_STANDALONE
   "gamefunc_Steroids",
   "gamefunc_Quick_Kick",
#else
   "gamefunc_Item_5",
   "gamefunc_Melee",
#endif
   "gamefunc_Next_Weapon",
   "gamefunc_Previous_Weapon",
   "gamefunc_Show_Console",
   "gamefunc_Show_Scoreboard",
   "gamefunc_Dpad_Select",
   "gamefunc_Dpad_Aiming",
   "gamefunc_AutoRun",
   "gamefunc_Last_Weapon",
   "gamefunc_Quick_Save",
   "gamefunc_Quick_Load",
   "gamefunc_Alt_Weapon",
   "gamefunc_Third_Person_View",
   "gamefunc_Toggle_Crouch",
   };


// classic key order is just integer indices ascending, not listed
const int32_t keybind_order_modern[NUMGAMEFUNCTIONS] =
   {
   // WASD
   gamefunc_Move_Forward,
   gamefunc_Move_Backward,
   gamefunc_Strafe_Left,
   gamefunc_Strafe_Right,
   // attacks
   gamefunc_Fire,
   gamefunc_Alt_Fire,
   gamefunc_Quick_Kick,
   // interact
   gamefunc_Open,
   gamefunc_Jump,
   gamefunc_Crouch,
   gamefunc_Toggle_Crouch,
   gamefunc_Run,
   // inventory
   gamefunc_Inventory,
   gamefunc_Inventory_Left,
   gamefunc_Inventory_Right,
   gamefunc_MedKit,
   gamefunc_Jetpack,
   gamefunc_Steroids,
   gamefunc_NightVision,
   gamefunc_Holo_Duke,
   // weapon selection
   gamefunc_Next_Weapon,
   gamefunc_Previous_Weapon,
   gamefunc_Last_Weapon,
   gamefunc_Weapon_1,
   gamefunc_Weapon_2,
   gamefunc_Weapon_3,
   gamefunc_Weapon_4,
   gamefunc_Weapon_5,
   gamefunc_Weapon_6,
   gamefunc_Weapon_7,
   gamefunc_Weapon_8,
   gamefunc_Weapon_9,
   gamefunc_Weapon_10,
   // important utilities
   gamefunc_Map,
   gamefunc_Map_Follow_Mode,
   gamefunc_Third_Person_View,
   gamefunc_Quick_Save,
   gamefunc_Quick_Load,
   // rarely used or usually replaced
   gamefunc_Holster_Weapon,
   gamefunc_Alt_Weapon,
   gamefunc_Aim_Up,
   gamefunc_Aim_Down,
   gamefunc_Turn_Left,
   gamefunc_Turn_Right,
   gamefunc_Center_View,
   gamefunc_Look_Up,
   gamefunc_Look_Down,
   gamefunc_Look_Left,
   gamefunc_Look_Right,
   gamefunc_TurnAround,
   gamefunc_Dpad_Select,
   gamefunc_Dpad_Aiming,
   gamefunc_Shrink_Screen,
   gamefunc_Enlarge_Screen,
   gamefunc_Strafe,
   // toggles that have corresponding menu options
   gamefunc_Mouse_Aiming,
   gamefunc_Toggle_Crosshair,
   gamefunc_AutoRun,
   // multiplayer
   gamefunc_SendMessage,
   gamefunc_Show_Opponents_Weapon,
   gamefunc_See_Coop_View,
   gamefunc_Show_Scoreboard,
   gamefunc_Show_Console,
   };

int32_t keybind_order_custom[NUMGAMEFUNCTIONS] = {-1};

#ifdef __SETUP__

const char keydefaults[NUMGAMEFUNCTIONS*2][MAXGAMEFUNCLEN] =
   {
   "W", "Kpad8",
   "S", "Kpad2",
   "Left", "Kpad4",
   "Right", "KPad6",
   "LAlt", "RAlt",
   "RCtrl", "",
   "E", "",
   "LShift", "RShift",
   "", "",
   "Space", "/",
   "LCtrl", "",
   "PgUp", "Kpad9",
   "PgDn", "Kpad3",
   "Insert", "Kpad0",
   "Delete", "Kpad.",
   "A", "",
   "D", "",
   "Home", "KPad7",
   "End", "Kpad1",
   "1", "",
   "2", "",
   "3", "",
   "4", "",
   "5", "",
   "6", "",
   "7", "",
   "8", "",
   "9", "",
   "0", "",
   "Enter", "KpdEnt",
   "[", "",
   "]", "",
   "H", "",
   "J", "",
   "N", "",
   "M", "",
   "BakSpc", "",
   "T", "",
   "Tab", "",
   "-", "Kpad-",
   "=", "Kpad+",
   "KPad5", "",
   "ScrLck", "",
   "Y", "",
   "F", "",
   "K", "",
   "", "",
   "", "",
   "R", "",
   "Q", "",
   "'", "",
   ";", "",
   "`", "",
   "", "",
   "", "",
   "", "",
   "CapLck", "",
   "X", "",
   "F6", "",
   "F9", "",
   "", "",
   "F7", "",
   "C", "",
   };

const char oldkeydefaults[NUMGAMEFUNCTIONS*2][MAXGAMEFUNCLEN] =
   {
   "Up", "Kpad8",
   "Down", "Kpad2",
   "Left", "Kpad4",
   "Right", "KPad6",
   "LAlt", "RAlt",
   "LCtrl", "RCtrl",
   "Space", "",
   "LShift", "RShift",
   "", "",
   "A", "/",
   "Z", "",
   "PgUp", "Kpad9",
   "PgDn", "Kpad3",
   "Insert", "Kpad0",
   "Delete", "Kpad.",
   ",", "",
   ".", "",
   "Home", "KPad7",
   "End", "Kpad1",
   "1", "",
   "2", "",
   "3", "",
   "4", "",
   "5", "",
   "6", "",
   "7", "",
   "8", "",
   "9", "",
   "0", "",
   "Enter", "KpdEnt",
   "[", "",
   "]", "",
   "H", "",
   "J", "",
   "N", "",
   "M", "",
   "BakSpc", "",
   "T", "",
   "Tab", "",
   "-", "Kpad-",
   "=", "Kpad+",
   "KPad5", "",
   "ScrLck", "",
   "W", "",
   "F", "",
   "K", "",
   "U", "",
   "I", "",
   "R", "",
   "`", "",
   "'", "",
   ";", "",
   "C", "",
   "", "",
   "", "",
   "", "",
   "CapLck", "",
   "", "",
   "F6", "",
   "F9", "",
   "", "",
   "F7", "",
   "", "",
   };

static const char * mousedefaults[MAXMOUSEBUTTONS] =
   {
   "Fire",
   "Alt_Fire",
   "MedKit",
   "",
   "Previous_Weapon",
   "Next_Weapon",
   };


static const char * mouseclickeddefaults[MAXMOUSEBUTTONS] =
   {
   };


#if defined GEKKO
static const char * joystickdefaults[MAXJOYBUTTONSANDHATS] =
   {
   "Open", // A
   "Fire", // B
   "Run", // 1
   "Map", // 2
   "Previous_Weapon", // -
   "Next_Weapon", // +
   "", // Home
   "Jump", // Z
   "Crouch", // C
   "Map", // X
   "Run", // Y
   "Jump", // L
   "Quick_Kick", // R
   "Crouch", // ZL
   "Fire", // ZR
   "Quick_Kick", // D-Pad Up
   "Inventory_Right", // D-Pad Right
   "Inventory", // D-Pad Down
   "Inventory_Left", // D-Pad Left
   };


static const char * joystickclickeddefaults[MAXJOYBUTTONSANDHATS] =
   {
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "Inventory",
   };


static const char * joystickanalogdefaults[MAXJOYAXES] =
   {
   "analog_strafing",
   "analog_moving",
   "analog_turning",
   "analog_lookingupanddown",
   };


static const char * joystickdigitaldefaults[MAXJOYDIGITAL] =
   {
   };
#endif

#endif
#ifdef __cplusplus
}
#endif
#endif
