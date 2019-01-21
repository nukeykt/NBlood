//-------------------------------------------------------------------------
/*
Copyright (C) 2010-2019 EDuke32 developers and contributors
Copyright (C) 2019 Nuke.YKT

This file is part of NBlood.

NBlood is free software; you can redistribute it and/or
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
#include "build.h"
#include "compat.h"
#include "mouse.h"
#include "common_game.h"
#include "blood.h"
#include "config.h"
#include "gamemenu.h"
#include "globals.h"
#include "inifile.h"
#include "levels.h"
#include "menu.h"
#include "qav.h"
#include "resource.h"
#include "view.h"

CMenuTextMgr gMenuTextMgr;
CGameMenuMgr gGameMenuMgr;

extern CGameMenuItemPicCycle itemSorryPicCycle;
extern CGameMenuItemQAV itemBloodQAV;

CMenuTextMgr::CMenuTextMgr()
{
    at0 = -1;
}

static char buffer[21][45];

void CMenuTextMgr::DrawText(const char *pString, int nFont, int x, int y, int nShade, int nPalette, bool shadow )
{
    viewDrawText(nFont, pString, x, y, nShade, nPalette, 0, shadow);
}

void CMenuTextMgr::GetFontInfo(int nFont, const char *pString, int *pXSize, int *pYSize)
{
    if (nFont < 0 || nFont >= 5)
        return;
    viewGetFontInfo(nFont, pString, pXSize, pYSize);
}

bool CGameMenuMgr::m_bInitialized = false;
bool CGameMenuMgr::m_bActive = false;

CGameMenuMgr::CGameMenuMgr()
{
    dassert(!m_bInitialized);
    m_bInitialized = true;
    Clear();
}

CGameMenuMgr::~CGameMenuMgr()
{
    m_bInitialized = false;
    Clear();
}

void CGameMenuMgr::InitializeMenu(void)
{
    if (pActiveMenu)
    {
        CGameMenuEvent event;
        event.at0 = kMenuEventInit;
        event.at2 = 0;
        pActiveMenu->Event(event);
    }
}

void CGameMenuMgr::DeInitializeMenu(void)
{
    if (pActiveMenu)
    {
        CGameMenuEvent event;
        event.at0 = kMenuEventDeInit;
        event.at2 = 0;
        pActiveMenu->Event(event);
    }
}

bool CGameMenuMgr::Push(CGameMenu *pMenu, int nItem)
{
    if (nMenuPointer == 0)
    {
        mouseReadAbs(&m_prevmousepos, &g_mouseAbs);
        m_mouselastactivity = -M_MOUSETIMEOUT;
        m_mousewake_watchpoint = 0;
        mouseLockToWindow(0);
    }
    dassert(pMenu != NULL);
    if (nMenuPointer == 8)
        return false;
    pActiveMenu = pMenuStack[nMenuPointer] = pMenu;
    nMenuPointer++;
    if (nItem >= 0)
        pMenu->SetFocusItem(nItem);
    m_bActive = true;
    gInputMode = INPUT_MODE_1;
    InitializeMenu();
    m_menuchange_watchpoint = 1;
    m_mousecaught = 1;
    return true;
}

void CGameMenuMgr::Pop(void)
{
    if (nMenuPointer > 0)
    {
        DeInitializeMenu();
        nMenuPointer--;
        if (nMenuPointer == 0)
            Deactivate();
        else
            pActiveMenu = pMenuStack[nMenuPointer-1];

        m_menuchange_watchpoint = 1;
    }
    m_mousecaught = 1;
}

void CGameMenuMgr::Draw(void)
{
    if (pActiveMenu)
    {
        pActiveMenu->Draw();
        viewUpdatePages();
    }

    int32_t mousestatus = mouseReadAbs(&m_mousepos, &g_mouseAbs);
    if (mousestatus && g_mouseClickState == MOUSE_PRESSED)
        m_mousedownpos = m_mousepos;

    if (tilesiz[kCrosshairTile].x > 0 && mousestatus)
    {
        if (!MOUSEACTIVECONDITION)
            m_mousewake_watchpoint = 1;

        if (MOUSEACTIVECONDITIONAL(mouseAdvanceClickState()) || m_mousepos.x != m_prevmousepos.x || m_mousepos.y != m_prevmousepos.y)
        {
            m_prevmousepos = m_mousepos;
            m_mouselastactivity = totalclock;
        }
        else
            m_mousewake_watchpoint = 0;

        m_mousecaught = 0;
    }
    else
    {
        m_mouselastactivity = -M_MOUSETIMEOUT;

        m_mousewake_watchpoint = 0;
    }
    // Display the mouse cursor, except on touch devices.
    if (MOUSEACTIVECONDITION)
    {
        int32_t a = kCrosshairTile;

        if ((unsigned) a < MAXTILES)
        {
            vec2_t cursorpos = m_mousepos;
            int32_t z = 65536;
            uint8_t p = CROSSHAIR_PAL;
            uint32_t o = 2;

            auto const oyxaspect = yxaspect;
            int32_t alpha = CURSORALPHA;

            rotatesprite_fs_alpha(cursorpos.x, cursorpos.y, z, 0, a, 0, p, o, alpha);
        }
    }
    else
        g_mouseClickState = MOUSE_IDLE;
}

void CGameMenuMgr::Clear(void)
{
    pActiveMenu = NULL;
    memset(pMenuStack, 0, sizeof(pMenuStack));
    nMenuPointer = 0;
}

void CGameMenuMgr::Process(void)
{
    if (!pActiveMenu)
        return;

    if (m_menuchange_watchpoint > 0)
        m_menuchange_watchpoint++;

    CGameMenuEvent event;
    event.at0 = 0;
    event.at2 = 0;
    char key;
    if (!pActiveMenu->MouseEvent(event) && (key = keyGetScan()) != 0 )
    {
        keyFlushScans();
        keyFlushChars();
        event.at2 = key;
        switch (key)
        {
        case sc_Escape:
            event.at0 = kMenuEventEscape;
            break;
        case sc_Tab:
            if (keystatus[sc_LeftShift] || keystatus[sc_RightShift])
                event.at0 = kMenuEventUp;
            else
                event.at0 = kMenuEventDown;
            break;
        case sc_UpArrow:
        case sc_kpad_8:
            event.at0 = kMenuEventUp;
            break;
        case sc_DownArrow:
        case sc_kpad_2:
            event.at0 = kMenuEventDown;
            break;
        case sc_Enter:
        case sc_kpad_Enter:
            event.at0 = kMenuEventEnter;
            break;
        case sc_Space:
            event.at0 = kMenuEventSpace;
            break;
        case sc_LeftArrow:
        case sc_kpad_4:
            event.at0 = kMenuEventLeft;
            break;
        case sc_RightArrow:
        case sc_kpad_6:
            event.at0 = kMenuEventRight;
            break;
        case sc_Delete:
        case sc_kpad_Period:
            event.at0 = kMenuEventDelete;
            break;
        case sc_BackSpace:
            event.at0 = kMenuEventBackSpace;
            break;
        default:
            event.at0 = kMenuEventKey;
            break;
        }
    }
    if (pActiveMenu->Event(event))
        Pop();

    if (m_menuchange_watchpoint >= 3)
        m_menuchange_watchpoint = 0;
}

void CGameMenuMgr::Deactivate(void)
{
    Clear();
    keyFlushScans();
    keyFlushChars();
    m_bActive = false;

    mouseLockToWindow(1);
    gInputMode = INPUT_MODE_0;
}

bool CGameMenuMgr::MouseOutsideBounds(vec2_t const * const pos, const int32_t x, const int32_t y, const int32_t width, const int32_t height)
{
    return pos->x < x || pos->x >= x + width || pos->y < y || pos->y >= y + height;
}

CGameMenu::CGameMenu()
{
    m_nItems = 0;
    m_nFocus = at8 = -1;
    atc = 0;
}

CGameMenu::CGameMenu(int unk)
{
    m_nItems = 0;
    m_nFocus = at8 = -1;
    atc = unk;
}

CGameMenu::~CGameMenu()
{
    if (!atc)
        return;
    for (int i = 0; i < m_nItems; i++)
    {
        if (pItemList[i] != &itemBloodQAV && pItemList[i] != &itemSorryPicCycle)
            delete pItemList[i];
    }
}

void CGameMenu::InitializeItems(CGameMenuEvent &event)
{
    for (int i = 0; i < m_nItems; i++)
    {
        pItemList[i]->Event(event);
    }
}

void CGameMenu::Draw(void)
{
    for (int i = 0; i < m_nItems; i++)
    {
        if (i == m_nFocus || (i != m_nFocus && !pItemList[i]->bNoDraw))
            pItemList[i]->Draw();
    }
}

bool CGameMenu::Event(CGameMenuEvent &event)
{
    if (m_nItems <= 0)
        return true;
    switch (event.at0)
    {
    case kMenuEventInit:
    case kMenuEventDeInit:
        if (at8 >= 0)
            m_nFocus = at8;
        InitializeItems(event);
        return false;
    }
    if (m_nFocus < 0)
        return true;
    return pItemList[m_nFocus]->Event(event);
}

void CGameMenu::Add(CGameMenuItem *pItem, bool active)
{
    dassert(pItem != NULL);
    dassert(m_nItems < kMaxGameMenuItems);
    pItemList[m_nItems] = pItem;
    pItem->pMenu = this;
    if (active)
        m_nFocus = at8 = m_nItems;
    m_nItems++;
}

void CGameMenu::SetFocusItem(int nItem)
{
    dassert(nItem >= 0 && nItem < m_nItems && nItem < kMaxGameMenuItems);
    if (CanSelectItem(nItem))
        m_nFocus = at8 = nItem;
}

void CGameMenu::SetFocusItem(CGameMenuItem *pItem)
{
    for (int i = 0; i < m_nItems; i++)
        if (pItemList[i] == pItem)
        {
            SetFocusItem(i);
            break;
        }
}

bool CGameMenu::CanSelectItem(int nItem)
{
    dassert(nItem >= 0 && nItem < m_nItems && nItem < kMaxGameMenuItems);
    return pItemList[nItem]->bCanSelect && pItemList[nItem]->bEnable;
}

void CGameMenu::FocusPrevItem(void)
{
    dassert(m_nFocus >= -1 && m_nFocus < m_nItems && m_nFocus < kMaxGameMenuItems);
    int t = m_nFocus;
    do
    {
        m_nFocus--;
        if (m_nFocus < 0)
            m_nFocus += m_nItems;
        if (CanSelectItem(m_nFocus))
            break;
    } while(t != m_nFocus);
}

void CGameMenu::FocusNextItem(void)
{
    dassert(m_nFocus >= -1 && m_nFocus < m_nItems && m_nFocus < kMaxGameMenuItems);
    int t = m_nFocus;
    do
    {
        m_nFocus++;
        if (m_nFocus >= m_nItems)
            m_nFocus = 0;
        if (CanSelectItem(m_nFocus))
            break;
    } while(t != m_nFocus);
}

bool CGameMenu::IsFocusItem(CGameMenuItem *pItem)
{
    if (m_nFocus < 0)
        return false;
    dassert(m_nFocus >= 0 && m_nFocus < m_nItems && m_nFocus < kMaxGameMenuItems);
    return pItemList[m_nFocus] == pItem;
}

bool CGameMenu::MouseEvent(CGameMenuEvent &event)
{
    if (m_nItems <= 0 || m_nFocus < 0)
        return true;
    return pItemList[m_nFocus]->MouseEvent(event);
}

CGameMenuItem::CGameMenuItem()
{
    pzText = NULL;
    nX = nY = nWidth = 0;
    bCanSelect = 1;
    bEnable = 1;
    nFont = -1;
    pMenu = NULL;
    bNoDraw = 0;
}

bool CGameMenuItem::Event(CGameMenuEvent &event)
{
    switch (event.at0)
    {
    case kMenuEventEscape:
        return true;
    case kMenuEventUp:
        pMenu->FocusPrevItem();
        break;
    case kMenuEventDown:
        pMenu->FocusNextItem();
        break;
    }
    return false;
}

bool CGameMenuItem::MouseEvent(CGameMenuEvent &event)
{
    event.at0 = kMenuEventNone;
    if (MOUSEINACTIVECONDITIONAL(MOUSE_GetButtons()&LEFT_MOUSE))
    {
        event.at0 = kMenuEventEnter;
        MOUSE_ClearButton(LEFT_MOUSE);
    }
    else if (MOUSE_GetButtons()&RIGHT_MOUSE)
    {
        event.at0 = kMenuEventEscape;
        MOUSE_ClearButton(RIGHT_MOUSE);
    }
#if 0
    else if (MOUSEINACTIVECONDITIONAL((MOUSE_GetButtons()&LEFT_MOUSE) && (MOUSE_GetButtons()&WHEELUP_MOUSE)))
    {
        MOUSE_ClearButton(WHEELUP_MOUSE);
        event.bAutoAim = kMenuEventScrollLeft;
    }
    else if (MOUSEINACTIVECONDITIONAL((MOUSE_GetButtons()&LEFT_MOUSE) && (MOUSE_GetButtons()&WHEELDOWN_MOUSE)))
    {
        MOUSE_ClearButton(WHEELDOWN_MOUSE);
        event.bAutoAim = kMenuEventScrollRight;
    }
#endif
    else if (MOUSE_GetButtons()&WHEELUP_MOUSE)
    {
        MOUSE_ClearButton(WHEELUP_MOUSE);
        event.at0 = kMenuEventUp;
    }
    else if (MOUSE_GetButtons()&WHEELDOWN_MOUSE)
    {
        MOUSE_ClearButton(WHEELDOWN_MOUSE);
        event.at0 = kMenuEventDown;
    }
    return event.at0 != kMenuEventNone;
}

CGameMenuItemText::CGameMenuItemText()
{
    pzText = 0;
    bEnable = 0;
}

CGameMenuItemText::CGameMenuItemText(const char *a1, int a2, int a3, int a4, int a5)
{
    nWidth = 0;
    pzText = a1;
    nFont = a2;
    nX = a3;
    nY = a4;
    at20 = a5;
    bEnable = 0;
}

void CGameMenuItemText::Draw(void)
{
    if (pzText)
    {
        int width;
        int x = nX;
        switch (at20)
        {
        case 1:
            gMenuTextMgr.GetFontInfo(nFont, pzText, &width, NULL);
            x = nX-width/2;
            break;
        case 2:
            gMenuTextMgr.GetFontInfo(nFont, pzText, &width, NULL);
            x = nX-width;
            break;
        }
        gMenuTextMgr.DrawText(pzText,nFont, x, nY, -128, 0, false);
    }
}

CGameMenuItemTitle::CGameMenuItemTitle()
{
    pzText = 0;
    bEnable = 0;
}

CGameMenuItemTitle::CGameMenuItemTitle(const char *a1, int a2, int a3, int a4, int a5)
{
    nWidth = 0;
    pzText = a1;
    nFont = a2;
    nX = a3;
    nY = a4;
    at20 = a5;
    bEnable = 0;
}

void CGameMenuItemTitle::Draw(void)
{
    if (pzText)
    {
        int height;
        gMenuTextMgr.GetFontInfo(nFont, NULL, NULL, &height);
        if (at20 >= 0)
            rotatesprite(320<<15, nY<<16, 65536, 0, at20, -128, 0, 78, 0, 0, xdim-1, ydim-1);
        viewDrawText(nFont, pzText, nX, nY-height/2, -128, 0, 1, false);
    }
}

CGameMenuItemZBool::CGameMenuItemZBool()
{
    at20 = false;
    pzText = 0;
    at21 = "On";
    at25 = "Off";
}

CGameMenuItemZBool::CGameMenuItemZBool(const char *a1, int a2, int a3, int a4, int a5, bool a6, void(*a7)(CGameMenuItemZBool *), const char *a8, const char *a9)
{
    pzText = a1;
    nFont = a2;
    nX = a3;
    nY = a4;
    nWidth = a5;
    at20 = a6;
    at29 = a7;
    if (!a8)
        at21 = "On";
    else
        at21 = a8; 
    if (!a9)
        at25 = "Off";
    else
        at25 = a9; 
}

void CGameMenuItemZBool::Draw(void)
{
    int shade = 32;
    if (pMenu->IsFocusItem(this))
        shade = 32-(totalclock&63);
    if (pzText)
        gMenuTextMgr.DrawText(pzText, nFont, nX, nY, shade, 0, false);
    const char *value = at20 ? at21 : at25;
    int width, height;
    gMenuTextMgr.GetFontInfo(nFont, value, &width, &height);
    gMenuTextMgr.DrawText(value, nFont, nWidth-1+nX-width, nY, shade, 0, false);
    int mx = nX<<16;
    int my = nY<<16;
    int mw = nWidth<<16;
    int mh = height<<16;
    if (bEnable && MOUSEACTIVECONDITIONAL(!gGameMenuMgr.MouseOutsideBounds(&gGameMenuMgr.m_mousepos, mx, my, mw, mh)))
    {
        if (MOUSEWATCHPOINTCONDITIONAL(!gGameMenuMgr.MouseOutsideBounds(&gGameMenuMgr.m_prevmousepos, mx, my, mw, mh)))
        {
            pMenu->SetFocusItem(this);
        }

        if (!gGameMenuMgr.m_mousecaught && g_mouseClickState == MOUSE_RELEASED && !gGameMenuMgr.MouseOutsideBounds(&gGameMenuMgr.m_mousedownpos, mx, my, mw, mh))
        {
            pMenu->SetFocusItem(this);

            CGameMenuEvent event = { kMenuEventEnter, 0 };

            gGameMenuMgr.m_mousecaught = 1;

            Event(event);
        }
    }
}

bool CGameMenuItemZBool::Event(CGameMenuEvent &event)
{
    switch (event.at0)
    {
    case kMenuEventEnter:
    case kMenuEventSpace:
        at20 = !at20;
        if (at29)
            at29(this);
        return false;
    }
    return CGameMenuItem::Event(event);
}

CGameMenuItemChain::CGameMenuItemChain()
{
    pzText = NULL;
    at24 = NULL;
    at28 = -1;
    at2c = NULL;
    at30 = 0;
}

CGameMenuItemChain::CGameMenuItemChain(const char *a1, int a2, int a3, int a4, int a5, int a6, CGameMenu *a7, int a8, void(*a9)(CGameMenuItemChain *), int a10)
{
    pzText = a1;
    nFont = a2;
    nX = a3;
    nY = a4;
    nWidth = a5;
    at20 = a6;
    at24 = a7;
    at28 = a8;
    at2c = a9;
    at30 = a10;
}

void CGameMenuItemChain::Draw(void)
{
    if (!pzText) return;
    int shade = 32;
    if (pMenu->IsFocusItem(this))
        shade = 32-(totalclock&63);
    int width, height;
    int x = nX;
    int y = nY;
    gMenuTextMgr.GetFontInfo(nFont, pzText, &width, &height);
    switch (at20)
    {
    case 1:
        x = nX+nWidth/2-width/2;
        break;
    case 2:
        x = nX+nWidth-1-width;
        break;
    case 0:
    default:
        break;
    }
    gMenuTextMgr.DrawText(pzText, nFont, x, nY, shade, 0, true);
    if (bEnable && MOUSEACTIVECONDITIONAL(!gGameMenuMgr.MouseOutsideBounds(&gGameMenuMgr.m_mousepos, x<<16, y<<16, width<<16, height<<16)))
    {
        if (MOUSEWATCHPOINTCONDITIONAL(!gGameMenuMgr.MouseOutsideBounds(&gGameMenuMgr.m_prevmousepos, x<<16, y<<16, width<<16, height<<16)))
        {
            pMenu->SetFocusItem(this);
        }

        if (!gGameMenuMgr.m_mousecaught && g_mouseClickState == MOUSE_RELEASED && !gGameMenuMgr.MouseOutsideBounds(&gGameMenuMgr.m_mousedownpos, x<<16, y<<16, width<<16, height<<16))
        {
            pMenu->SetFocusItem(this);

            CGameMenuEvent event = { kMenuEventEnter, 0 };

            gGameMenuMgr.m_mousecaught = 1;

            Event(event);
        }
    }
}

bool CGameMenuItemChain::Event(CGameMenuEvent &event)
{
    switch (event.at0)
    {
    case kMenuEventEnter:
        if (at2c)
            at2c(this);
        if (at24)
            gGameMenuMgr.Push(at24, at28);
        return false;
    }
    return CGameMenuItem::Event(event);
}

CGameMenuItem7EA1C::CGameMenuItem7EA1C()
{
    pzText = NULL;
    at24 = NULL;
    at28 = -1;
    at2c = NULL;
    at30 = 0;
    at34 = NULL;
    at38[0] = 0;
    at48[0] = 0;
}

CGameMenuItem7EA1C::CGameMenuItem7EA1C(const char *a1, int a2, int a3, int a4, int a5, const char *a6, const char *a7, int a8, int a9, void(*a10)(CGameMenuItem7EA1C *), int a11)
{
    pzText = a1;
    nFont = a2;
    nX = a3;
    nY = a4;
    nWidth = a5;
    at20 = a8;
    at28 = a9;
    at2c = a10;
    at30 = a11;
    strncpy(at38, a6, 15);
    strncpy(at48, a7, 15);
}

void CGameMenuItem7EA1C::Draw(void)
{
    if (!pzText) return;
    int shade = 32;
    if (pMenu->IsFocusItem(this))
        shade = 32-(totalclock&63);
    int width;
    int x = nX;
    switch (at20)
    {
    case 1:
        gMenuTextMgr.GetFontInfo(nFont, pzText, &width, NULL);
        x = nX+nWidth/2-width/2;
        break;
    case 2:
        gMenuTextMgr.GetFontInfo(nFont, pzText, &width, NULL);
        x = nX+nWidth-1-width;
        break;
    case 0:
    default:
        break;
    }
    gMenuTextMgr.DrawText(pzText, nFont, x, nY, shade, 0, true);
}

void CGameMenuItem7EA1C::Setup(void)
{
    if (!at34 || !at24)
        return;
    if (!at34->SectionExists(at48))
        return;
    const char *title = at34->GetKeyString(at48, "Title", at48);
    at24->Add(new CGameMenuItemTitle(title, 1, 160, 20, 2038), false);
    at24->Add(&itemSorryPicCycle, true);
    int y = 40;
    for (int i = 0; i < 21; i++)
    {
        sprintf(buffer[i], "Line%ld", i+1);
        if (!at34->KeyExists(at48, buffer[i]))
            break;
        const char *line = at34->GetKeyString(at48, buffer[i], NULL);
        if (line)
        {
            if (*line == 0)
            {
                y += 10;
                continue;
            }
            at24->Add(new CGameMenuItemText(line, 1, 160, y, 1), false);
            y += 20;
        }
    }
    at24->Add(&itemBloodQAV, false);
}

bool CGameMenuItem7EA1C::Event(CGameMenuEvent &event)
{
    switch (event.at0)
    {
    case kMenuEventEnter:
    {
        if (at2c)
            at2c(this);
        if (at24)
            delete at24;
        at24 = new CGameMenu(1);
        DICTNODE *pRes = gGuiRes.Lookup(at38, "MNU");
        if (pRes)
        {
            at34 = new IniFile(gGuiRes.Load(pRes));
            Setup();
        }
        if (at24)
            gGameMenuMgr.Push(at24, at28);
        return false;
    }
    case kMenuEventDeInit:
        if (at34)
        {
            delete at34;
            at34 = NULL;
        }
        if (at24)
        {
            delete at24;
            at24 = NULL;
        }
        return false;
    }
    return CGameMenuItem::Event(event);
}

CGameMenuItem7EE34::CGameMenuItem7EE34()
{
    pzText = NULL;
    at28 = NULL;
    at20 = -1;
    at2c = NULL;
}

CGameMenuItem7EE34::CGameMenuItem7EE34(const char *a1, int a2, int a3, int a4, int a5, int a6)
{
    pzText = NULL;
    at28 = NULL;
    at20 = -1;
    at2c = NULL;
    nFont = a2;
    nX = a3;
    pzText = a1;
    nY = a4;
    nWidth = a5;
    at24 = a6;
}

void CGameMenuItem7EE34::Draw(void)
{
    if (!pzText) return;
    int shade = 32;
    if (pMenu->IsFocusItem(this))
        shade = 32-(totalclock&63);
    int width;
    int x = nX;
    switch (at24)
    {
    case 1:
        gMenuTextMgr.GetFontInfo(nFont, pzText, &width, NULL);
        x = nX+nWidth/2-width/2;
        break;
    case 2:
        gMenuTextMgr.GetFontInfo(nFont, pzText, &width, NULL);
        x = nX+nWidth-1-width;
        break;
    case 0:
    default:
        break;
    }
    gMenuTextMgr.DrawText(pzText, nFont, x, nY, shade, 0, true);
}

extern void SetVideoModeOld(CGameMenuItemChain *pItem);

void CGameMenuItem7EE34::Setup(void)
{
    if (!at28)
        return;
    at28->Add(new CGameMenuItemTitle("Video Mode", 1, 160, 20, 2038), false);
    if (!at2c)
    {
        at2c = new CGameMenu(1);
        at2c->Add(new CGameMenuItemTitle(" Mode Change ", 1, 160, 20, 2038), false);
        at2c->Add(&itemSorryPicCycle, true);
        CGameMenuItem *pItem1 = new CGameMenuItemText("VIDEO MODE WAS SET", 1, 160, 90, 1);
        CGameMenuItem *pItem2 = new CGameMenuItemText("NOT ALL MODES Work correctly", 1, 160, 110, 1);
        CGameMenuItem *pItem3 = new CGameMenuItemText("Press ESC to exit", 3, 160, 140, 1);
        at2c->Add(pItem1, false);
        pItem1->bEnable = 0;
        at2c->Add(pItem2, false);
        pItem2->bEnable = 0;
        at2c->Add(pItem3, true);
        pItem3->bEnable = 1;
        at2c->Add(&itemBloodQAV, false);
    }
    sprintf(buffer[0], "640 x 480 (default)");
    int y = 40;
    at28->Add(new CGameMenuItemChain(buffer[0], 3, 0, y, 320, 1, at2c, -1, SetVideoModeOld, validmodecnt), true);
    y += 20;
    for (int i = 0; i < validmodecnt && i < 20; i++)
    {
        sprintf(buffer[i+1], "%d x %d", validmode[i].xdim, validmode[i].ydim);
        at28->Add(new CGameMenuItemChain(buffer[i+1], 3, 0, y, 320, 1, at2c, -1, SetVideoModeOld, i), false);
        if (validmodecnt > 10)
            y += 7;
        else
            y += 15;
    }
    at28->Add(&itemBloodQAV, false);
}

bool CGameMenuItem7EE34::Event(CGameMenuEvent &event)
{
    switch (event.at0)
    {
    case kMenuEventEnter:
        if (at28)
            delete at28;
        at28 = new CGameMenu(1);
        Setup();
        if (at28)
            gGameMenuMgr.Push(at28, at20);
        return false;
    case kMenuEventDeInit:
        if (at28)
        {
            delete at28;
            at28 = 0;
        }
        return false;
    }
    return CGameMenuItem::Event(event);
}

CGameMenuItemChain7F2F0::CGameMenuItemChain7F2F0()
{
    at34 = -1;
}

CGameMenuItemChain7F2F0::CGameMenuItemChain7F2F0(char *a1, int a2, int a3, int a4, int a5, int a6, CGameMenu *a7, int a8, void(*a9)(CGameMenuItemChain *), int a10) :
    CGameMenuItemChain(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10)
{
    at34 = a10;
}

bool CGameMenuItemChain7F2F0::Event(CGameMenuEvent &event)
{
    switch (event.at0)
    {
    case kMenuEventEnter:
        if (at34 > -1)
            gGameOptions.nEpisode = at34;
        return CGameMenuItemChain::Event(event);
    }
    return CGameMenuItem::Event(event);
}

CGameMenuItemBitmap::CGameMenuItemBitmap()
{
    pzText = NULL;
}

CGameMenuItemBitmap::CGameMenuItemBitmap(const char *a1, int a2, int a3, int a4, int a5)
{
    pzText = a1;
    nFont = a2;
    nX = a3;
    nY = a4;
    at20 = a5;
}

void CGameMenuItemBitmap::Draw(void)
{
    int shade = 32;
    if (bEnable && pMenu->IsFocusItem(this))
        shade = 32-(totalclock&63);
    int x = nX;
    int y = nY;
    if (pzText)
    {
        int height;
        gMenuTextMgr.DrawText(pzText, nFont, x, y, shade, 0, false);
        gMenuTextMgr.GetFontInfo(nFont, NULL, NULL, &height);
        y += height + 2;
    }
    rotatesprite(x<<15,y<<15, 65536, 0, at20, 0, 0, 82, 0, 0, xdim-1,ydim-1);
}

bool CGameMenuItemBitmap::Event(CGameMenuEvent &event)
{
    if (bEnable && pMenu->IsFocusItem(this))
        pMenu->FocusNextItem();
    return CGameMenuItem::Event(event);
}

CGameMenuItemBitmapLS::CGameMenuItemBitmapLS()
{
    pzText = NULL;
}

CGameMenuItemBitmapLS::CGameMenuItemBitmapLS(const char *a1, int a2, int a3, int a4, int a5)
{
    at24 = -1;
    pzText = a1;
    nFont = a2;
    nX = a3;
    nY = a4;
    at28 = a5;
}

void CGameMenuItemBitmapLS::Draw(void)
{
    int shade = 32;
    if (bEnable && pMenu->IsFocusItem(this))
        shade = 32-(totalclock&63);
    int x = nX;
    int y = nY;
    if (pzText)
    {
        int height;
        gMenuTextMgr.DrawText(pzText, nFont, x, y, shade, 0, false);
        gMenuTextMgr.GetFontInfo(nFont, NULL, NULL, &height);
        y += height + 2;
    }
    char stat;
    int16_t ang;
    int picnum;
    if (at24 == -1)
    {
        stat = 66;
        ang = 0;
        picnum = at28;
    }
    else
    {
        ang = 512;
        stat = 70;
        picnum = at24;
    }
    rotatesprite(200<<15,215<<15,32768, ang, picnum, 0, 0, stat, 0, 0, xdim-1, ydim-1);
}

bool CGameMenuItemBitmapLS::Event(CGameMenuEvent &event)
{
    if (bEnable && pMenu->IsFocusItem(this))
        pMenu->FocusNextItem();
    return CGameMenuItem::Event(event);
}

CGameMenuItemKeyList::CGameMenuItemKeyList()
{
    pzText = NULL;
    nFont = 3;
    nX = 0;
    nY = 0;
    nRows = 0;
    nTopDelta = 0;
    nFocus = 0;
    nGameFuncs = 0;
    bScan = false;
}

CGameMenuItemKeyList::CGameMenuItemKeyList(const char *a1, int a2, int a3, int a4, int a5, int a6, int a7, void(*a8)(CGameMenuItemKeyList *))
{
    nTopDelta = 0;
    nFocus = 0;
    bScan = false;
    pzText = a1;
    nFont = a2;
    nX = a3;
    nY = a4;
    nWidth = a5;
    nRows = a6;
    pCallback = a8;
    nGameFuncs = a7;
}

void CGameMenuItemKeyList::Scan(void)
{
    KB_FlushKeyboardQueue();
    KB_FlushKeyboardQueueScans();
    KB_ClearKeysDown();
    KB_LastScan = 0;
    bScan = true;
}

extern uint8_t KeyboardKeys[NUMGAMEFUNCTIONS][2];
void CGameMenuItemKeyList::Draw(void)
{
    char buffer[40], buffer2[40];
    int width, height;
    int shade;
    gMenuTextMgr.GetFontInfo(nFont, NULL, NULL, &height);
    int y = nY;
    int k = nFocus - nTopDelta;
    int nNewFocus = nFocus;
    bool bClick = false;
    for (int i = 0; i < nRows; i++, y += height, k++)
    {
        BYTE key1, key2;
        key1 = KeyboardKeys[k][0];
        key2 = KeyboardKeys[k][1];
        const char *sKey1 = key1 == sc_Tilde ? "Tilde" : KB_ScanCodeToString(key1);
        const char *sKey2 = key2 == sc_Tilde ? "Tilde" : KB_ScanCodeToString(key2);
        sprintf(buffer, "%s", CONFIG_FunctionNumToName(k));
        if (key2 == 0 || key2 == 0xff)
        {
            if (key1 == 0 || key1 == 0xff)
                sprintf(buffer2, "????");
            else
                sprintf(buffer2, "%s", sKey1);
        }
        else
            sprintf(buffer2, "%s or %s", sKey1, sKey2);
        
        if (k == nFocus)
        {
            shade = 32;
            if (pMenu->IsFocusItem(this))
                shade = 32-(totalclock&63);
            viewDrawText(3, buffer, nX, y, shade, 0, 0, false);
            const char *sVal;
            if (bScan && (gGameClock & 32))
                sVal = "____";
            else
                sVal = buffer2;
            gMenuTextMgr.GetFontInfo(nFont, sVal, &width, 0);
            viewDrawText(nFont, sVal, nX+nWidth-1-width, y, shade, 0, 0, false);
        }
        else
        {
            viewDrawText(3, buffer, nX, y, 24, 0, 0, false);
            gMenuTextMgr.GetFontInfo(nFont, buffer2, &width, 0);
            viewDrawText(nFont, buffer2, nX+nWidth-1-width, y, 24, 0, 0, false);
        }
        int mx = nX<<16;
        int my = y<<16;
        int mw = nWidth<<16;
        int mh = height<<16;
        if (bEnable && MOUSEACTIVECONDITIONAL(!gGameMenuMgr.MouseOutsideBounds(&gGameMenuMgr.m_mousepos, mx, my, mw, mh)))
        {
            if (MOUSEWATCHPOINTCONDITIONAL(!gGameMenuMgr.MouseOutsideBounds(&gGameMenuMgr.m_prevmousepos, mx, my, mw, mh)))
            {
                nNewFocus = k;
            }

            if (!gGameMenuMgr.m_mousecaught && g_mouseClickState == MOUSE_RELEASED && !gGameMenuMgr.MouseOutsideBounds(&gGameMenuMgr.m_mousedownpos, mx, my, mw, mh))
            {
                nNewFocus = k;
                bClick = true;
            }
        }
    }
    nTopDelta += nNewFocus-nFocus;
    nFocus = nNewFocus;
    if (bClick)
    {
        CGameMenuEvent event = { kMenuEventEnter, 0 };

        gGameMenuMgr.m_mousecaught = 1;

        Event(event);
    }
}

bool CGameMenuItemKeyList::Event(CGameMenuEvent &event)
{
    if (bScan)
    {
        if (KB_LastScan && KB_LastScan != sc_Pause)
        {
            if (KB_KeyWaiting())
                KB_GetCh();
            BYTE key1, key2;
            extern uint8_t KeyboardKeys[NUMGAMEFUNCTIONS][2];
            key1 = KeyboardKeys[nFocus][0];
            key2 = KeyboardKeys[nFocus][1];
            if (key1 > 0 && key2 != KB_LastScan)
                key2 = key1;
            key1 = KB_LastScan;
            if (key1 == key2)
                key2 = 0;
            uint8_t oldKey[2];
            oldKey[0] = KeyboardKeys[nFocus][0];
            oldKey[1] = KeyboardKeys[nFocus][1];
            KeyboardKeys[nFocus][0] = key1;
            KeyboardKeys[nFocus][1] = key2;
            CONFIG_MapKey(nFocus, key1, oldKey[0], key2, oldKey[1]);
            KB_FlushKeyboardQueue();
            KB_FlushKeyboardQueueScans();
            KB_ClearKeysDown();
            keyFlushScans();
            keyFlushChars();
            bScan = 0;
        }
        return false;
    }
    switch (event.at0)
    {
    case kMenuEventUp:
        if (event.at2 == sc_Tab || nFocus == 0)
        {
            pMenu->FocusPrevItem();
            return false;
        }
        nFocus--;
        if (nTopDelta > 0)
            nTopDelta--;
        return false;
    case kMenuEventDown:
        if (event.at2 == sc_Tab || nFocus == nGameFuncs-1)
        {
            pMenu->FocusNextItem();
            return false;
        }
        nFocus++;
        if (nTopDelta+1 < nRows)
            nTopDelta++;
        return false;
    case kMenuEventEnter:
        if (pCallback)
            pCallback(this);
        Scan();
        return false;
    case kMenuEventDelete:
        if (keystatus[sc_LeftControl] || keystatus[sc_RightControl])
        {
            uint8_t oldKey[2];
            oldKey[0] = KeyboardKeys[nFocus][0];
            oldKey[1] = KeyboardKeys[nFocus][1];
            KeyboardKeys[nFocus][0] = 0;
            KeyboardKeys[nFocus][1] = 0;
            CONFIG_MapKey(nFocus, 0, oldKey[0], 0, oldKey[1]);
        }
        return false;
    case kMenuEventScrollUp:
        if (nFocus-nTopDelta > 0)
        {
            nTopDelta++;
            if (nTopDelta>0)
            {
                nFocus--;
                nTopDelta--;
            }
        }
        return false;
    case kMenuEventScrollDown:
        if (nFocus-nTopDelta+nRows < nGameFuncs)
        {
            nTopDelta--;
            if (nTopDelta+1 < nRows)
            {
                nFocus++;
                nTopDelta++;
            }
        }
        return false;
    }
    return CGameMenuItem::Event(event);
}

bool CGameMenuItemKeyList::MouseEvent(CGameMenuEvent &event)
{
    event.at0 = kMenuEventNone;
    if (MOUSEACTIVECONDITIONAL(MOUSE_GetButtons()&WHEELUP_MOUSE))
    {
        gGameMenuMgr.m_mouselastactivity = totalclock;
        MOUSE_ClearButton(WHEELUP_MOUSE);
        event.at0 = kMenuEventScrollUp;
    }
    else if (MOUSEACTIVECONDITIONAL(MOUSE_GetButtons()&WHEELDOWN_MOUSE))
    {
        gGameMenuMgr.m_mouselastactivity = totalclock;
        MOUSE_ClearButton(WHEELDOWN_MOUSE);
        event.at0 = kMenuEventScrollDown;
    }
    else
        return CGameMenuItem::MouseEvent(event);
    return event.at0 != kMenuEventNone;
}

CGameMenuItemSlider::CGameMenuItemSlider()
{
    pzText = NULL;
    nFont = -1;
    nX = 0;
    nY = 0;
    nValue = 0;
    nRangeLow = 0;
    nStep = 0;
    pCallback = NULL;
    pValue = NULL;
    nSliderTile = 2204;
    nCursorTile = 2028;
}

CGameMenuItemSlider::CGameMenuItemSlider(const char *a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, void(*a10)(CGameMenuItemSlider *), int a11, int a12)
{
    pzText = a1;
    nFont = a2;
    nX = a3;
    nY = a4;
    nWidth = a5;
    nRangeLow = a7;
    nRangeHigh = a8;
    nStep = a9;
    nValue = ClipRange(a6, nRangeLow, nRangeHigh);
    pCallback = a10;
    nSliderTile = 2204;
    nCursorTile = 2028;
    if (a11 >= 0)
        nSliderTile = a11;
    if (a12 >= 0)
        nCursorTile = a12;
}

CGameMenuItemSlider::CGameMenuItemSlider(const char *a1, int a2, int a3, int a4, int a5, int *pnValue, int a7, int a8, int a9, void(*a10)(CGameMenuItemSlider *), int a11, int a12)
{
    pzText = a1;
    nFont = a2;
    nX = a3;
    nY = a4;
    nWidth = a5;
    nRangeLow = a7;
    nRangeHigh = a8;
    nStep = a9;
    dassert(pnValue != NULL);
    pValue = pnValue;
    nValue = ClipRange(*pnValue, nRangeLow, nRangeHigh);
    pCallback = a10;
    nSliderTile = 2204;
    nCursorTile = 2028;
    if (a11 >= 0)
        nSliderTile = a11;
    if (a12 >= 0)
        nCursorTile = a12;
}

void CGameMenuItemSlider::Draw(void)
{
    int height;
    nValue = pValue ? *pValue : nValue;
    gMenuTextMgr.GetFontInfo(nFont, NULL, NULL, &height);
    int shade = 32;
    if (pMenu->IsFocusItem(this))
        shade = 32-(totalclock&63);
    if (pzText)
        gMenuTextMgr.DrawText(pzText, nFont, nX, nY, shade, 0, false);
    int sliderX = nX+nWidth-1-tilesiz[nSliderTile].x/2;
    rotatesprite(sliderX<<16, (nY+height/2)<<16, 65536, 0, nSliderTile, 0, 0, 10, 0, 0, xdim-1, ydim-1);
    int nRange = nRangeHigh - nRangeLow;
    dassert(nRange > 0);
    int value = nValue - nRangeLow;
    int width = tilesiz[nSliderTile].x-8;
    int cursorX = sliderX + ksgn(nStep)*(value * width / nRange - width / 2);
    rotatesprite(cursorX<<16, (nY+height/2)<<16, 65536, 0, nCursorTile, 0, 0, 10, 0, 0, xdim-1, ydim-1);
    int mx = nX;
    int my = nY;
    int mw = nWidth;
    int mh = height;
    if (height < tilesiz[nSliderTile].y)
    {
        my -= (tilesiz[nSliderTile].y-height)/2;
        height = tilesiz[nSliderTile].y;
    }
    mx <<= 16;
    my <<= 16;
    mw <<= 16;
    mh <<= 16;

    if (bEnable && MOUSEACTIVECONDITIONAL(!gGameMenuMgr.MouseOutsideBounds(&gGameMenuMgr.m_mousepos, mx, my, mw, mh)))
    {
        if (MOUSEWATCHPOINTCONDITIONAL(!gGameMenuMgr.MouseOutsideBounds(&gGameMenuMgr.m_prevmousepos, mx, my, mw, mh)))
        {
            pMenu->SetFocusItem(this);
        }

        if (!gGameMenuMgr.m_mousecaught && (g_mouseClickState == MOUSE_PRESSED || g_mouseClickState == MOUSE_HELD))
        {
            pMenu->SetFocusItem(this);

            int sliderx = nX+nWidth-1-tilesiz[nSliderTile].x;
            int sliderwidth = tilesiz[nSliderTile].x;
            int regionwidth = sliderwidth-8;
            int regionx = sliderx+(sliderwidth-regionwidth)/2;
            sliderx <<= 16;
            sliderwidth <<= 16;
            regionwidth <<= 16;
            regionx <<= 16;

            // region between the x-midline of the slidepoint at the extremes slides proportionally
            if (!gGameMenuMgr.MouseOutsideBounds(&gGameMenuMgr.m_mousepos, regionx, my, regionwidth, mh))
            {
                int dx = (gGameMenuMgr.m_mousepos.x - (regionx+regionwidth/2))*ksgn(nStep);
                nValue = nRangeLow + roundscale(dx+regionwidth/2, nRange, regionwidth);
                nValue = ClipRange(nValue, nRangeLow, nRangeHigh);
                if (pCallback)
                    pCallback(this);
                gGameMenuMgr.m_mousecaught = 1;
            }
            // region outside the x-midlines clamps to the extremes
            else if (!gGameMenuMgr.MouseOutsideBounds(&gGameMenuMgr.m_mousepos, sliderx, my, sliderwidth, mh))
            {
                if ((gGameMenuMgr.m_mousepos.x-(regionx+regionwidth/2))*ksgn(nStep) > 0)
                    nValue = nRangeHigh;
                else
                    nValue = nRangeLow;
                if (pCallback)
                    pCallback(this);
                gGameMenuMgr.m_mousecaught = 1;
            }
        }
    }
}

bool CGameMenuItemSlider::Event(CGameMenuEvent &event)
{
    nValue = pValue ? *pValue : nValue;
    switch (event.at0)
    {
    case kMenuEventUp:
        pMenu->FocusPrevItem();
        return false;
    case kMenuEventDown:
        pMenu->FocusNextItem();
        return false;
    case kMenuEventLeft:
        if (nStep > 0)
            nValue = DecBy(nValue, nStep);
        else
            nValue = IncBy(nValue, -nStep);
        nValue = ClipRange(nValue, nRangeLow, nRangeHigh);
        if (pCallback)
            pCallback(this);
        return false;
    case kMenuEventRight:
        if (nStep >= 0)
            nValue = IncBy(nValue, nStep);
        else
            nValue = DecBy(nValue, -nStep);
        nValue = ClipRange(nValue, nRangeLow, nRangeHigh);
        if (pCallback)
            pCallback(this);
        return false;
    case kMenuEventEnter:
        if (pCallback)
            pCallback(this);
        return false;
    }
    return CGameMenuItem::Event(event);
}

bool CGameMenuItemSlider::MouseEvent(CGameMenuEvent &event)
{
    event.at0 = kMenuEventNone;
    if (MOUSEINACTIVECONDITIONAL((MOUSE_GetButtons()&LEFT_MOUSE) && (MOUSE_GetButtons()&WHEELUP_MOUSE)))
    {
        MOUSE_ClearButton(WHEELUP_MOUSE);
        event.at0 = kMenuEventLeft;
    }
    else if (MOUSEINACTIVECONDITIONAL((MOUSE_GetButtons()&LEFT_MOUSE) && (MOUSE_GetButtons()&WHEELDOWN_MOUSE)))
    {
        MOUSE_ClearButton(WHEELDOWN_MOUSE);
        event.at0 = kMenuEventRight;
    }
    else if (MOUSE_GetButtons()&RIGHT_MOUSE)
    {
        MOUSE_ClearButton(RIGHT_MOUSE);
        event.at0 = kMenuEventEscape;
    }
    else if (MOUSE_GetButtons()&WHEELUP_MOUSE)
    {
        MOUSE_ClearButton(WHEELUP_MOUSE);
        MOUSE_ClearButton(LEFT_MOUSE);
        event.at0 = kMenuEventUp;
    }
    else if (MOUSE_GetButtons()&WHEELDOWN_MOUSE)
    {
        MOUSE_ClearButton(WHEELDOWN_MOUSE);
        MOUSE_ClearButton(LEFT_MOUSE);
        event.at0 = kMenuEventDown;
    }
    return event.at0 != kMenuEventNone;
}

CGameMenuItemSliderFloat::CGameMenuItemSliderFloat()
{
    pzText = NULL;
    nFont = -1;
    nX = 0;
    nY = 0;
    fValue = 0;
    fRangeLow = 0;
    fStep = 0;
    pCallback = NULL;
    pValue = NULL;
    nSliderTile = 2204;
    nCursorTile = 2028;
}

CGameMenuItemSliderFloat::CGameMenuItemSliderFloat(const char *a1, int a2, int a3, int a4, int a5, float a6, float a7, float a8, float a9, void(*a10)(CGameMenuItemSliderFloat *), int a11, int a12)
{
    pzText = a1;
    nFont = a2;
    nX = a3;
    nY = a4;
    nWidth = a5;
    fRangeLow = a7;
    fRangeHigh = a8;
    fStep = a9;
    fValue = ClipRangeF(a6, fRangeLow, fRangeHigh);
    pCallback = a10;
    nSliderTile = 2204;
    nCursorTile = 2028;
    if (a11 >= 0)
        nSliderTile = a11;
    if (a12 >= 0)
        nCursorTile = a12;
}

CGameMenuItemSliderFloat::CGameMenuItemSliderFloat(const char *a1, int a2, int a3, int a4, int a5, float *pnValue, float a7, float a8, float a9, void(*a10)(CGameMenuItemSliderFloat *), int a11, int a12)
{
    pzText = a1;
    nFont = a2;
    nX = a3;
    nY = a4;
    nWidth = a5;
    fRangeLow = a7;
    fRangeHigh = a8;
    fStep = a9;
    dassert(pnValue != NULL);
    pValue = pnValue;
    fValue = ClipRangeF(*pnValue, fRangeLow, fRangeHigh);
    pCallback = a10;
    nSliderTile = 2204;
    nCursorTile = 2028;
    if (a11 >= 0)
        nSliderTile = a11;
    if (a12 >= 0)
        nCursorTile = a12;
}

void CGameMenuItemSliderFloat::Draw(void)
{
    int height;
    fValue = pValue ? *pValue : fValue;
    gMenuTextMgr.GetFontInfo(nFont, NULL, NULL, &height);
    int shade = 32;
    if (pMenu->IsFocusItem(this))
        shade = 32-(totalclock&63);
    if (pzText)
        gMenuTextMgr.DrawText(pzText, nFont, nX, nY, shade, 0, false);
    int sliderX = nX+nWidth-1-tilesiz[nSliderTile].x/2;
    rotatesprite(sliderX<<16, (nY+height/2)<<16, 65536, 0, nSliderTile, 0, 0, 10, 0, 0, xdim-1, ydim-1);
    float fRange = fRangeHigh - fRangeLow;
    dassert(fRange > 0);
    float value = fValue - fRangeLow;
    int width = tilesiz[nSliderTile].x-8;
    int cursorX = sliderX + (int)(ksgnf(fStep)*(value * width / fRange - width / 2));
    rotatesprite(cursorX<<16, (nY+height/2)<<16, 65536, 0, nCursorTile, 0, 0, 10, 0, 0, xdim-1, ydim-1);
    int mx = nX;
    int my = nY;
    int mw = nWidth;
    int mh = height;
    if (height < tilesiz[nSliderTile].y)
    {
        my -= (tilesiz[nSliderTile].y-height)/2;
        height = tilesiz[nSliderTile].y;
    }
    mx <<= 16;
    my <<= 16;
    mw <<= 16;
    mh <<= 16;

    if (bEnable && MOUSEACTIVECONDITIONAL(!gGameMenuMgr.MouseOutsideBounds(&gGameMenuMgr.m_mousepos, mx, my, mw, mh)))
    {
        if (MOUSEWATCHPOINTCONDITIONAL(!gGameMenuMgr.MouseOutsideBounds(&gGameMenuMgr.m_prevmousepos, mx, my, mw, mh)))
        {
            pMenu->SetFocusItem(this);
        }

        if (!gGameMenuMgr.m_mousecaught && (g_mouseClickState == MOUSE_PRESSED || g_mouseClickState == MOUSE_HELD))
        {
            pMenu->SetFocusItem(this);

            int sliderx = nX+nWidth-1-tilesiz[nSliderTile].x;
            int sliderwidth = tilesiz[nSliderTile].x;
            int regionwidth = sliderwidth-8;
            int regionx = sliderx+(sliderwidth-regionwidth)/2;
            sliderx <<= 16;
            sliderwidth <<= 16;
            regionwidth <<= 16;
            regionx <<= 16;

            // region between the x-midline of the slidepoint at the extremes slides proportionally
            if (!gGameMenuMgr.MouseOutsideBounds(&gGameMenuMgr.m_mousepos, regionx, my, regionwidth, mh))
            {
                int dx = (gGameMenuMgr.m_mousepos.x - (regionx+regionwidth/2))*ksgnf(fStep);
                fValue = fRangeLow + (dx+regionwidth/2) * fRange / regionwidth;
                fValue = ClipRangeF(fValue, fRangeLow, fRangeHigh);
                if (pCallback)
                    pCallback(this);
                gGameMenuMgr.m_mousecaught = 1;
            }
            // region outside the x-midlines clamps to the extremes
            else if (!gGameMenuMgr.MouseOutsideBounds(&gGameMenuMgr.m_mousepos, sliderx, my, sliderwidth, mh))
            {
                if ((gGameMenuMgr.m_mousepos.x-(regionx+regionwidth/2))*ksgnf(fStep) > 0)
                    fValue = fRangeHigh;
                else
                    fValue = fRangeLow;
                if (pCallback)
                    pCallback(this);
                gGameMenuMgr.m_mousecaught = 1;
            }
        }
    }
}

bool CGameMenuItemSliderFloat::Event(CGameMenuEvent &event)
{
    fValue = pValue ? *pValue : fValue;
    switch (event.at0)
    {
    case kMenuEventUp:
        pMenu->FocusPrevItem();
        return false;
    case kMenuEventDown:
        pMenu->FocusNextItem();
        return false;
    case kMenuEventLeft:
        if (fStep > 0)
            fValue -= fStep;
        else
            fValue += fStep;
        fValue = ClipRangeF(fValue, fRangeLow, fRangeHigh);
        if (pCallback)
            pCallback(this);
        return false;
    case kMenuEventRight:
        if (fStep >= 0)
            fValue += fStep;
        else
            fValue -= fStep;
        fValue = ClipRangeF(fValue, fRangeLow, fRangeHigh);
        if (pCallback)
            pCallback(this);
        return false;
    case kMenuEventEnter:
        if (pCallback)
            pCallback(this);
        return false;
    }
    return CGameMenuItem::Event(event);
}

CGameMenuItemZEdit::CGameMenuItemZEdit()
{
    pzText = NULL;
    nFont = -1;
    nX = 0;
    nY = 0;
    at20 = NULL;
    at24 = 0;
    at32 = 0;
    at2c = 0;
    at30 = 0;
    at28 = 0;
    at31 = 1;
}

CGameMenuItemZEdit::CGameMenuItemZEdit(const char *a1, int a2, int a3, int a4, int a5, char *a6, int a7, char a8, void(*a9)(CGameMenuItemZEdit *, CGameMenuEvent *), int a10)
{
    at30 = 0;
    at31 = 1;
    pzText = a1;
    nFont = a2;
    nX = a3;
    nY = a4;
    nWidth = a5;
    at20 = a6;
    at24 = a7;
    at32 = a8;
    at2c = a9;
    at28 = a10;
}

void CGameMenuItemZEdit::AddChar(char ch)
{
    int i = strlen(at20);
    if (i + 1 < at24)
    {
        at20[i] = ch;
        at20[i + 1] = 0;
    }
}

void CGameMenuItemZEdit::BackChar(void)
{
    int i = strlen(at20);
    if (i > 0)
        at20[i - 1] = 0;
}

void CGameMenuItemZEdit::Draw(void)
{
    int height, width;
    gMenuTextMgr.GetFontInfo(nFont, NULL, NULL, &height);
    int shade = 32;
    if (pMenu->IsFocusItem(this))
        shade = 32-(totalclock&63);
    if (at30)
        shade = -128;
    if (pzText)
        gMenuTextMgr.DrawText(pzText, nFont, nX, nY, shade, 0, false);
    int x = nX+nWidth-1-(at24+1)*height;
    if (at20 && *at20)
    {
        gMenuTextMgr.GetFontInfo(nFont, NULL, &width, NULL);
        int shade2;
        if (at32)
        {
            if (at30)
                shade2 = -128;
            else
                shade2 = shade;
        }
        else
        {
            if (at30)
                shade2 = shade;
            else
                shade2 = 32;
        }
        gMenuTextMgr.DrawText(at20, nFont, x, nY, shade2, 0, false);
        x += width;
    }
    if (at30 && (gGameClock & 32))
        gMenuTextMgr.DrawText("_", nFont, x, nY, shade, 0, false);
}

bool CGameMenuItemZEdit::Event(CGameMenuEvent &event)
{
    static char buffer[256];
    switch (event.at0)
    {
    case kMenuEventEscape:
        if (at30)
        {
            strncpy(at20, buffer, at24);
            at20[at24-1] = 0;
            at30 = 0;
            return false;
        }
        return true;
    case kMenuEventEnter:
        if (!at31)
        {
            if (at2c)
                at2c(this, &event);
            return false;
        }
        if (at30)
        {
            if (at2c)
                at2c(this, &event);
            at30 = 0;
            return false;
        }
        strncpy(buffer, at20, at24);
        buffer[at24-1] = 0;
        at30 = 1;
        return false;
    case kMenuEventBackSpace:
        if (at30)
            BackChar();
        return false;
    case kMenuEventKey:
    case kMenuEventSpace:
    {
        char key;
        key = g_keyAsciiTable[event.at2];
        if (keystatus[sc_LeftShift] || keystatus[sc_RightShift])
            key = Btoupper(key);
        if (at30 && (isalnum(key) || ispunct(key) || isspace(key)))
        {
            AddChar(key);
            return false;
        }
        return CGameMenuItem::Event(event);
    }
    case kMenuEventUp:
        if (at30)
            return false;
        return CGameMenuItem::Event(event);
    case kMenuEventDown:
        if (at30)
            return false;
        return CGameMenuItem::Event(event);
    }
    return CGameMenuItem::Event(event);
}

CGameMenuItemZEditBitmap::CGameMenuItemZEditBitmap()
{
    pzText = NULL;
    nFont = -1;
    nX = 0;
    nY = 0;
    at20 = NULL;
    at24 = 0;
    at36 = 0;
    at30 = NULL;
    at2c = NULL;
    at34 = 0;
    at28 = 0;
    at37 = 0;
    at35 = 1;
}

CGameMenuItemZEditBitmap::CGameMenuItemZEditBitmap(char *a1, int a2, int a3, int a4, int a5, char *a6, int a7, char a8, void(*a9)(CGameMenuItemZEditBitmap *, CGameMenuEvent *), int a10)
{
    at2c = NULL;
    at34 = 0;
    at35 = 1;
    at37 = 0;
    pzText = a1;
    nFont = a2;
    nX = a3;
    nY = a4;
    nWidth = a5;
    at20 = a6;
    at24 = a7;
    at36 = a8;
    at30 = a9;
    at28 = a10;
}

void CGameMenuItemZEditBitmap::AddChar(char ch)
{
    int i = strlen(at20);
    if (i + 1 < at24)
    {
        at20[i] = ch;
        at20[i + 1] = 0;
    }
}

void CGameMenuItemZEditBitmap::BackChar(void)
{
    int i = strlen(at20);
    if (i > 0)
        at20[i - 1] = 0;
}

void CGameMenuItemZEditBitmap::Draw(void)
{
    int height, width;
    gMenuTextMgr.GetFontInfo(nFont, NULL, NULL, &height);
    int shade = 32;
    if (pMenu->IsFocusItem(this))
        shade = 32-(totalclock&63);
    at2c->at24 = -1;
    if (at34)
        shade = -128;
    if (pzText)
        gMenuTextMgr.DrawText(pzText, nFont, nX, nY, shade, 0, false);
    int x = nX+nWidth-1-(at24+1)*height;
    if (at20 && *at20)
    {
        gMenuTextMgr.GetFontInfo(nFont, at20, &width, NULL);
        int shade2;
        if (at36)
        {
            if (at34)
                shade2 = -128;
            else
                shade2 = shade;
        }
        else
        {
            if (at34)
                shade2 = shade;
            else
                shade2 = 32;
        }
        gMenuTextMgr.DrawText(at20, nFont, x, nY, shade2, 0, false);
        x += width;
    }
    if (at34 && (gGameClock & 32))
        gMenuTextMgr.DrawText("_", nFont, x, nY, shade, 0, false);
}

bool CGameMenuItemZEditBitmap::Event(CGameMenuEvent &event)
{
    static char buffer[256];
    switch (event.at0)
    {
    case kMenuEventEscape:
        if (at34)
        {
            strncpy(at20, buffer, at24);
            at20[at24-1] = 0;
            at34 = 0;
            gSaveGameActive = false;
            return false;
        }
        gSaveGameActive = true;
        return true;
    case kMenuEventEnter:
        if (!at35)
        {
            if (at30)
                at30(this, &event);
            gSaveGameActive = false;
            return false;
        }
        if (at34)
        {
            if (at30)
                at30(this, &event);
            at34 = 0;
            gSaveGameActive = false;
            return false;
        }
        strncpy(buffer, at20, at24);
        if (at37)
            at20[0] = 0;
        buffer[at24-1] = 0;
        at34 = 1;
        return false;
    case kMenuEventBackSpace:
        if (at34)
            BackChar();
        return false;
    case kMenuEventKey:
    case kMenuEventSpace:
    {
        char key;
        key = g_keyAsciiTable[event.at2];
        if (keystatus[sc_LeftShift] || keystatus[sc_RightShift])
            key = Btoupper(key);
        if (at34 && (isalnum(key) || ispunct(key) || isspace(key)))
        {
            AddChar(key);
            return false;
        }
        return CGameMenuItem::Event(event);
    }
    case kMenuEventUp:
        if (at34)
            return false;
        return CGameMenuItem::Event(event);
    case kMenuEventDown:
        if (at34)
            return false;
        return CGameMenuItem::Event(event);
    }
    return CGameMenuItem::Event(event);
}

CGameMenuItemQAV::CGameMenuItemQAV()
{
    at20 = NULL;
    at24 = NULL;
    at28 = 0;
    bEnable = 0;
}

CGameMenuItemQAV::CGameMenuItemQAV(const char *a1, int a2, int a3, int a4, const char *a5, bool widescreen, bool clearbackground)
{
    nWidth = 0;
    pzText = a1;
    nFont = a2;
    nY = a4;
    at20 = a5;
    nX = a3;
    bEnable = 0;
    bWideScreen = widescreen;
    bClearBackground = clearbackground;
}

void CGameMenuItemQAV::Draw(void)
{
    if (bClearBackground)
        videoClearScreen(0);
    if (at24)
    {
        int backFC = gFrameClock;
        gFrameClock = gGameClock;
        int nTicks = totalclock - at30;
        at30 = totalclock;
        at2c -= nTicks;
        if (at2c <= 0 || at2c > at28->at10)
        {
            at2c = at28->at10;
        }
        at28->Play(at28->at10 - at2c - nTicks, at28->at10 - at2c, -1, NULL);
        int wx1, wy1, wx2, wy2;
        wx1 = windowxy1.x;
        wy1 = windowxy1.y;
        wx2 = windowxy2.x;
        wy2 = windowxy2.y;
        windowxy1.x = 0;
        windowxy1.y = 0;
        windowxy2.x = xdim-1;
        windowxy2.y = ydim-1;
        if (bWideScreen)
        {
            int xdim43 = scale(ydim, 4, 3);
            int nCount = (xdim+xdim43-1)/xdim43;
            int backX = at28->x;
            for (int i = 0; i < nCount; i++)
            {
                at28->Draw(at28->at10 - at2c, 10+kQavOrientationLeft, 0, 0);
                at28->x += 320;
            }
            at28->x = backX;
        }
        else
            at28->Draw(at28->at10 - at2c, 10, 0, 0);

        windowxy1.x = wx1;
        windowxy1.y = wy1;
        windowxy2.x = wx2;
        windowxy2.y = wy2;
        gFrameClock = backFC;
    }
}

bool CGameMenuItemQAV::Event(CGameMenuEvent &event)
{
    switch (event.at0)
    {
    case kMenuEventLeft:
    case kMenuEventBackSpace:
        pMenu->FocusPrevItem();
        return false;
    case kMenuEventRight:
    case kMenuEventEnter:
    case kMenuEventSpace:
        pMenu->FocusNextItem();
        return false;
    case kMenuEventInit:
        if (at20)
        {
            if (!at28)
            {
                at24 = gSysRes.Lookup(at20, "QAV");
                if (!at24)
                    ThrowError("Could not load QAV %s\n", at20);
                at28 = (QAV*)gSysRes.Lock(at24);
                at28->nSprite = -1;
                at28->x = nX;
                at28->y = nY;
                at28->Preload();
                at2c = at28->at10;
                at30 = totalclock;
                return false;
            }
            gSysRes.Lock(at24);
        }
        return false;
    case kMenuEventDeInit:
        if (at20 && at28)
        {
            gSysRes.Unlock(at24);
            if (at24->lockCount == 0)
                at28 = NULL;
        }
        return false;
    }
    return CGameMenuItem::Event(event);
}

void CGameMenuItemQAV::Reset(void)
{
    at2c = at28->at10;
    at30 = totalclock;
}

CGameMenuItemZCycle::CGameMenuItemZCycle()
{
    pzText = NULL;
    at24 = 0;
    m_nItems = 0;
    atb4 = 0;
    at2c = 0;
    at30 = 0;
}

CGameMenuItemZCycle::CGameMenuItemZCycle(const char *a1, int a2, int a3, int a4, int a5, int a6, void(*a7)(CGameMenuItemZCycle *), const char **a8, int a9, int a10)
{
    pzText = a1;
    nFont = a2;
    nX = a3;
    nY = a4;
    at24 = 0;
    nWidth = a5;
    at28 = a6;
    atb4 = a7;
    m_nItems = 0;
    SetTextArray(a8, a9, a10);
}

CGameMenuItemZCycle::~CGameMenuItemZCycle()
{
    pzText = NULL;
    at24 = 0;
    m_nItems = 0;
    atb4 = 0;
    memset(at34, 0, sizeof(at34));
    at2c = 0;
    at30 = 0;
}

void CGameMenuItemZCycle::Draw(void)
{
    int width = 0, height = 0;
    int shade = 32;
    if (pMenu->IsFocusItem(this))
        shade = 32-(totalclock&63);
    int x = nX;
    int y = nY;
    if (pzText)
    {
        gMenuTextMgr.GetFontInfo(nFont, pzText, &width, &height);
        switch (at28)
        {
        case 1:
            x = nX+nWidth/2-width/2;
            break;
        case 2:
            x = nX+nWidth-1-width;
            break;
        case 0:
        default:
            break;
        }
        gMenuTextMgr.DrawText(pzText, nFont, x, y, shade, 0, false);
    }
    const char *pzText;
    if (!m_nItems)
        pzText = "????";
    else
        pzText = at34[at24];
    dassert(pzText != NULL);
    gMenuTextMgr.GetFontInfo(nFont, pzText, &width, NULL);
    gMenuTextMgr.DrawText(pzText, nFont, nX + nWidth - 1 - width, y, shade, 0, false);
    if (bEnable && MOUSEACTIVECONDITIONAL(!gGameMenuMgr.MouseOutsideBounds(&gGameMenuMgr.m_mousepos, x<<16, y<<16, nWidth<<16, height<<16)))
    {
        if (MOUSEWATCHPOINTCONDITIONAL(!gGameMenuMgr.MouseOutsideBounds(&gGameMenuMgr.m_prevmousepos, x<<16, y<<16, nWidth<<16, height<<16)))
        {
            pMenu->SetFocusItem(this);
        }

        if (!gGameMenuMgr.m_mousecaught && g_mouseClickState == MOUSE_RELEASED && !gGameMenuMgr.MouseOutsideBounds(&gGameMenuMgr.m_mousedownpos, x<<16, y<<16, nWidth<<16, height<<16))
        {
            pMenu->SetFocusItem(this);

            CGameMenuEvent event = { kMenuEventEnter, 0 };

            gGameMenuMgr.m_mousecaught = 1;

            Event(event);
        }
    }
}

bool CGameMenuItemZCycle::Event(CGameMenuEvent &event)
{
    switch (event.at0)
    {
    case kMenuEventRight:
    case kMenuEventEnter:
    case kMenuEventSpace:
        Next();
        if (atb4)
            atb4(this);
        return false;
    case kMenuEventLeft:
        Prev();
        if (atb4)
            atb4(this);
        return false;
    }
    return CGameMenuItem::Event(event);
}

void CGameMenuItemZCycle::Add(const char *pItem, bool active)
{
    dassert(pItem != NULL);
    dassert(m_nItems < kMaxGameCycleItems);
    at34[m_nItems] = pItem;
    if (active)
        at24 = m_nItems;
    m_nItems++;
}

void CGameMenuItemZCycle::Next(void)
{
    if (m_nItems > 0)
    {
        at24++;
        if (at24 >= m_nItems)
            at24 = 0;
    }
}

void CGameMenuItemZCycle::Prev(void)
{
    if (m_nItems > 0)
    {
        at24--;
        if (at24 < 0)
            at24 += m_nItems;
    }
}

void CGameMenuItemZCycle::Clear(void)
{
    m_nItems = at24 = 0;
    memset(at34, 0, sizeof(at34));
    at2c = 0;
    at30 = 0;
}

void CGameMenuItemZCycle::SetTextArray(const char **pTextArray, int nTextPtrCount, int nIndex)
{
    Clear();
    dassert(nTextPtrCount <= kMaxGameCycleItems);
    for (int i = 0; i < nTextPtrCount; i++)
        Add(pTextArray[i], false);
    SetTextIndex(nIndex);
}

void CGameMenuItemZCycle::SetTextIndex(int nIndex)
{
    at24 = ClipRange(nIndex, 0, m_nItems);
}

CGameMenuItemYesNoQuit::CGameMenuItemYesNoQuit()
{
    pzText = NULL;
    at24 = -1;
    at28 = 0;
}

CGameMenuItemYesNoQuit::CGameMenuItemYesNoQuit(const char *a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8)
{
    pzText = a1;
    nFont = a2;
    nX = a3;
    nY = a4;
    nWidth = a5;
    at20 = a6;
    at24 = a7;
    at28 = a8;
}

void CGameMenuItemYesNoQuit::Draw(void)
{
    if (!pzText) return;
    int shade = 32;
    if (pMenu->IsFocusItem(this))
        shade = 32-(totalclock&63);
    int width;
    int x = nX;
    switch (at20)
    {
    case 1:
        gMenuTextMgr.GetFontInfo(nFont, pzText, &width, NULL);
        x = nX+nWidth/2-width/2;
        break;
    case 2:
        gMenuTextMgr.GetFontInfo(nFont, pzText, &width, NULL);
        x = nX+nWidth-1-width;
        break;
    case 0:
    default:
        break;
    }
    gMenuTextMgr.DrawText(pzText, nFont, x, nY, shade, 0, true);
}

extern void Quit(CGameMenuItemChain *pItem);

bool CGameMenuItemYesNoQuit::Event(CGameMenuEvent &event)
{
    switch (event.at0)
    {
    case kMenuEventKey:
        if (event.at2 == sc_Y)
            Quit(NULL);
        else if (event.at2 == sc_N)
            gGameMenuMgr.Pop();
        return false;
    case kMenuEventEnter:
        Quit(NULL);
        return false;
    }
    return CGameMenuItem::Event(event);
}

CGameMenuItemPicCycle::CGameMenuItemPicCycle()
{
    pzText = NULL;
    at24 = 0;
    m_nItems = 0;
    atb0 = 0;
    at2c = 0;
    atb4 = 0;
}

CGameMenuItemPicCycle::CGameMenuItemPicCycle(int a1, int a2, void(*a3)(CGameMenuItemPicCycle *), int *a4, int a5, int a6)
{
    nWidth = 0;
    at24 = 0;
    m_nItems = 0;
    nX = a1;
    nY = a2;
    atb0 = a3;
    atb4 = 0;
    SetPicArray(a4, a5, a6);
}

void CGameMenuItemPicCycle::Draw(void)
{
    videoSetViewableArea(0, 0, xdim - 1, ydim - 1);
    if (atb4)
        rotatesprite(0, 0, 65536, 0, atb4, 0, 0, 82, 0, 0, xdim - 1, ydim - 1);
    if (at30[at24])
        rotatesprite(0, 0, 65536, 0, at30[at24], 0, 0, 82, 0, 0, xdim - 1, ydim - 1);
}

bool CGameMenuItemPicCycle::Event(CGameMenuEvent &event)
{
    switch (event.at0)
    {
    case kMenuEventRight:
    case kMenuEventEnter:
    case kMenuEventSpace:
        Next();
        if (atb0)
            atb0(this);
        return false;
    case kMenuEventLeft:
        Prev();
        if (atb0)
            atb0(this);
        return false;
    }
    return CGameMenuItem::Event(event);
}

void CGameMenuItemPicCycle::Add(int nItem, bool active)
{
    dassert(m_nItems < kMaxPicCycleItems);
    at30[m_nItems] = nItem;
    if (active)
        at24 = m_nItems;
    m_nItems++;
}

void CGameMenuItemPicCycle::Next(void)
{
    if (m_nItems > 0)
    {
        at24++;
        if (at24 >= m_nItems)
            at24 = 0;
    }
}

void CGameMenuItemPicCycle::Prev(void)
{
    if (m_nItems > 0)
    {
        at24--;
        if (at24 < 0)
            at24 += m_nItems;
    }
}

void CGameMenuItemPicCycle::Clear(void)
{
    m_nItems = at24 = 0;
    memset(at30, 0, sizeof(at30));
    at2c = 0;
}

void CGameMenuItemPicCycle::SetPicArray(int *pArray, int nTileCount, int nIndex)
{
    Clear();
    at2c = 0;
    dassert(nTileCount <= kMaxPicCycleItems);
    for (int i = 0; i < nTileCount; i++)
        Add(pArray[i], false);
    SetPicIndex(nIndex);
}

void CGameMenuItemPicCycle::SetPicIndex(int nIndex)
{
    at24 = ClipRange(nIndex, 0, m_nItems);
}

CGameMenuItemPassword::CGameMenuItemPassword()
{
    at37 = 0;
    pzText = NULL;
    at36 = 0;
    at32 = 0;
    at5b = 0;
}

CGameMenuItemPassword::CGameMenuItemPassword(const char *a1, int a2, int a3, int a4)
{
    at37 = 0;
    nWidth = 0;
    at36 = 0;
    at32 = 0;
    at5b = 0;
    pzText = a1;
    nFont = a2;
    nX = a3;
    nY = a4;
}

const char *kCheckPasswordMsg = "ENTER PASSWORD: ";
const char *kOldPasswordMsg = "ENTER OLD PASSWORD: ";
const char *kNewPasswordMsg = "ENTER NEW PASSWORD: ";
const char *kInvalidPasswordMsg = "INVALID PASSWORD.";

void CGameMenuItemPassword::Draw(void)
{
    bool focus = pMenu->IsFocusItem(this);
    int shade = 32;
    int shadef = 32-(totalclock&63);
    int width;
    switch (at37)
    {
    case 1:
    case 2:
    case 3:
        switch (at37)
        {
        case 1:
            strcpy(at3b, kCheckPasswordMsg);
            break;
        case 2:
            strcpy(at3b, kOldPasswordMsg);
            break;
        case 3:
            strcpy(at3b, kNewPasswordMsg);
            break;
        }
        for (int i = 0; i < at32; i++)
            strcat(at3b, "*");
        strcat(at3b, "_");
        gMenuTextMgr.GetFontInfo(nFont, at3b, &width, NULL);
        gMenuTextMgr.DrawText(at3b, nFont, nX-width/2, nY+20, shadef, 0, false);
        shadef = 32;
        break;
    case 4:
        if ((totalclock - at5b) & 32)
        {
            gMenuTextMgr.GetFontInfo(nFont, kInvalidPasswordMsg, &width, NULL);
            gMenuTextMgr.DrawText(kInvalidPasswordMsg, nFont, nX - width / 2, nY + 20, shade, 0, false);
        }
        if (at5b && totalclock-at5b > 256)
        {
            at5b = 0;
            at37 = 0;
        }
        break;
    }
    gMenuTextMgr.GetFontInfo(nFont, pzText, &width, NULL);
    gMenuTextMgr.DrawText(pzText, nFont, nX-width/2, nY, focus ? shadef : shade, 0, false);
}

bool CGameMenuItemPassword::Event(CGameMenuEvent &event)
{
    switch (at37)
    {
    case 0:
    case 4:
        if (event.at0 == kMenuEventEnter)
        {
            at29[0] = 0;
            if (strcmp(at20, ""))
                at37 = 2;
            else
                at37 = 3;
            return false;
        }
        return CGameMenuItem::Event(event);
    case 1:
    case 2:
    case 3:
        switch (event.at0)
        {
        case kMenuEventEnter:
            switch (at37)
            {
            case 1:
                at36 = strcmp(at20,at29) == 0;
                if (at36)
                    at37 = 0;
                else
                    at37 = 4;
                if (!at36)
                {
                    at5b = totalclock;
                    pMenu->FocusPrevItem();
                }
                else
                {
                    at5f->at20 = 0;
                    at5f->Draw();
                    gbAdultContent = false;
                    // NUKE-TODO:
                    //CONFIG_WriteAdultMode();
                    pMenu->FocusPrevItem();
                }
                return false;
            case 2:
                at36 = strcmp(at20,at29) == 0;
                if (at36)
                    at37 = 0;
                else
                    at37 = 4;
                if (at36)
                {
                    strcpy(at20, "");
                    strcpy(gzAdultPassword, "");
                    // NUKE-TODO:
                    //CONFIG_WriteAdultMode();
                    at37 = 0;
                }
                else
                    at5b = totalclock;
                return false;
            case 3:
                strcpy(at20, at29);
                strcpy(at20, gzAdultPassword);
                strcpy(gzAdultPassword, "");
                // NUKE-TODO:
                //CONFIG_WriteAdultMode();
                at37 = 0;
                return false;
            }
            break;
        case kMenuEventEscape:
            at37 = 0;
            Draw();
            return false;
        case kMenuEventKey:
            if (at32 < 8)
            {
                char key = Btoupper(g_keyAsciiTable[event.at2]);
                if (isalnum(key) || ispunct(key) || isspace(key))
                {
                    at29[at32++] = key;
                    at29[at32] = 0;
                }
            }
            return false;
        case kMenuEventBackSpace:
            if (at32 > 0)
                at29[--at32] = 0;
            return false;
        case kMenuEventLeft:
        case kMenuEventRight:
        case kMenuEventSpace:
            return false;
        }
    }
    return CGameMenuItem::Event(event);
}
