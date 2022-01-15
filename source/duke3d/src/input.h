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

#ifndef input_h_
#define input_h_

extern int32_t I_CheckAllInput(void);
extern void I_ClearAllInput(void);
extern void I_ClearLast(void);

// Advance = Selecting a menu option || Saying "Yes" || Going forward in Help/Credits
// Return = Closing a sub-menu || Saying "No"
// General = Advance + Return = Skipping screens
// Escape = Opening the menu in-game (should not be any gamefuncs)

extern int32_t I_AdvanceTrigger(void);
extern int32_t I_GeneralTrigger(void);
extern int32_t I_ReturnTrigger(void);
extern int32_t I_EscapeTrigger(void);

extern void I_AdvanceTriggerClear(void);
extern void I_GeneralTriggerClear(void);

#define I_ReturnTriggerClear I_ClearLast
#define I_EscapeTriggerClear I_ClearLast

extern int32_t I_MenuUp(void);
extern int32_t I_MenuDown(void);
extern int32_t I_MenuLeft(void);
extern int32_t I_MenuRight(void);

#define I_MenuUpClear    I_ClearLast
#define I_MenuDownClear  I_ClearLast
#define I_MenuLeftClear  I_ClearLast
#define I_MenuRightClear I_ClearLast

extern int32_t I_PanelUp(void);
extern int32_t I_PanelDown(void);

extern void I_PanelUpClear(void);
extern void I_PanelDownClear(void);

extern int32_t I_SliderLeft(void);
extern int32_t I_SliderRight(void);

#define I_SliderLeftClear  I_ClearLast
#define I_SliderRightClear I_ClearLast

enum EnterTextFlags_t {
    INPUT_NUMERIC        = 0x00000001,
};

extern int32_t I_EnterText(char *t, int32_t maxlength, int32_t flags);

#define FF_WEAPON_MAX_TIME 66
#define FF_WEAPON_DMG_MIN  10
#define FF_WEAPON_DMG_MAX  50

#define FF_WEAPON_DMG_SCALE  8
#define FF_WEAPON_TIME_SCALE 3

#define FF_PLAYER_DMG_SCALE  10
#define FF_PLAYER_TIME_SCALE 4

static FORCE_INLINE void I_AddForceFeedback(int const lo, int const hi, int const time)
{
    if (!joystick.hasRumble || !ud.config.controllerRumble)
        return;

    joystick.rumbleHigh = min<int>(UINT16_MAX, joystick.rumbleHigh + hi);
    joystick.rumbleLow  = min<int>(UINT16_MAX, joystick.rumbleLow + lo);
    joystick.rumbleTime = max<int>(time, joystick.rumbleTime);
}

#endif
