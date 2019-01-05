#include "build.h"
#include "compat.h"
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

#define WIN32CODE

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
        event.at0 = 0x8000;
        event.at2 = 0;
        pActiveMenu->Event(event);
    }
}

void CGameMenuMgr::sub_7DF1C(void)
{
    if (pActiveMenu)
    {
        CGameMenuEvent event;
        event.at0 = 0x8001;
        event.at2 = 0;
        pActiveMenu->Event(event);
    }
}

bool CGameMenuMgr::Push(CGameMenu *pMenu, int nItem)
{
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

    mouseLockToWindow(0);
    return true;
}

void CGameMenuMgr::Pop(void)
{
    if (nMenuPointer > 0)
    {
        sub_7DF1C();
        nMenuPointer--;
        if (nMenuPointer == 0)
            Deactivate();
        else
            pActiveMenu = pMenuStack[nMenuPointer-1];
    }
}

void CGameMenuMgr::Draw(void)
{
    if (pActiveMenu)
    {
        pActiveMenu->Draw();
        viewUpdatePages();
    }
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
    CGameMenuEvent event;
    event.at0 = 0;
    event.at2 = 0;
    char key;
    if ( (key = keyGetScan()) != 0 )
    {
#ifdef WIN32CODE
        keyFlushScans();
        keyFlushChars();
#else
        keyFlushStream();
#endif
        event.at2 = key;
        switch (key)
        {
        case sc_Escape:
            event.at0 = 7;
            break;
        case sc_Tab:
            if (keystatus[sc_LeftShift] || keystatus[sc_RightShift])
                event.at0 = 2;
            else
                event.at0 = 3;
            break;
        case sc_UpArrow:
        case sc_kpad_8:
            event.at0 = 2;
            break;
        case sc_DownArrow:
        case sc_kpad_2:
            event.at0 = 3;
            break;
        case sc_Enter:
        case sc_kpad_Enter:
            event.at0 = 6;
            break;
        case sc_Space:
            event.at0 = 8;
            break;
        case sc_LeftArrow:
        case sc_kpad_4:
            event.at0 = 4;
            break;
        case sc_RightArrow:
        case sc_kpad_6:
            event.at0 = 5;
            break;
        case sc_Delete:
        case sc_kpad_Period:
            event.at0 = 10;
            break;
        case sc_BackSpace:
            event.at0 = 9;
            break;
        default:
            event.at0 = 1;
            break;
        }
    }
    if (pActiveMenu->Event(event))
        Pop();
}

void CGameMenuMgr::Deactivate(void)
{
    Clear();
#ifdef WIN32CODE
    keyFlushScans();
    keyFlushChars();
#else
    keyFlushStream();
#endif
    m_bActive = false;

    mouseLockToWindow(1);
    gInputMode = INPUT_MODE_0;
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
        if (i == m_nFocus || (i != m_nFocus && !(pItemList[i]->at18 & 8)))
            pItemList[i]->Draw();
    }
}

bool CGameMenu::Event(CGameMenuEvent &event)
{
    if (m_nItems <= 0)
        return true;
    switch (event.at0)
    {
    case 0x8000:
    case 0x8001:
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

bool CGameMenu::CanSelectItem(int nItem)
{
    dassert(nItem >= 0 && nItem < m_nItems && nItem < kMaxGameMenuItems);
    return (pItemList[nItem]->at18 & 1) && (pItemList[nItem]->at18 & 2);
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

CGameMenuItem::CGameMenuItem()
{
    at4 = NULL;
    atc = at10 = at14 = 0;
    at18 |= 3;
    at8 = -1;
    pMenu = NULL;
    at18 &= ~8;
}

bool CGameMenuItem::Event(CGameMenuEvent &event)
{
    switch (event.at0)
    {
    case 7:
        return true;
    case 2:
        pMenu->FocusPrevItem();
        break;
    case 3:
        pMenu->FocusNextItem();
        break;
    }
    return false;
}

CGameMenuItemText::CGameMenuItemText()
{
    at4 = 0;
    at18 &= ~2;
}

CGameMenuItemText::CGameMenuItemText(const char *a1, int a2, int a3, int a4, int a5)
{
    at14 = 0;
    at4 = a1;
    at8 = a2;
    atc = a3;
    at10 = a4;
    at20 = a5;
    at18 &= ~2;
}

void CGameMenuItemText::Draw(void)
{
    if (at4)
    {
        int width;
        int x = atc;
        switch (at20)
        {
        case 1:
            gMenuTextMgr.GetFontInfo(at8, at4, &width, NULL);
            x = atc-width/2;
            break;
        case 2:
            gMenuTextMgr.GetFontInfo(at8, at4, &width, NULL);
            x = atc-width;
            break;
        }
        gMenuTextMgr.DrawText(at4,at8, x, at10, -128, 0, false);
    }
}

CGameMenuItemTitle::CGameMenuItemTitle()
{
    at4 = 0;
    at18 &= ~2;
}

CGameMenuItemTitle::CGameMenuItemTitle(const char *a1, int a2, int a3, int a4, int a5)
{
    at14 = 0;
    at4 = a1;
    at8 = a2;
    atc = a3;
    at10 = a4;
    at20 = a5;
    at18 &= ~2;
}

void CGameMenuItemTitle::Draw(void)
{
    if (at4)
    {
        int height;
        gMenuTextMgr.GetFontInfo(at8, NULL, NULL, &height);
        rotatesprite(320<<15, at10<<16, 65536, 0, at20, -128, 0, 78, 0, 0, xdim-1, ydim-1);
        viewDrawText(at8, at4, atc, at10-height/2, -128, 0, 1, false);
    }
}

CGameMenuItemZBool::CGameMenuItemZBool()
{
    at20 = false;
    at4 = 0;
    at21 = "On";
    at25 = "Off";
}

CGameMenuItemZBool::CGameMenuItemZBool(const char *a1, int a2, int a3, int a4, int a5, bool a6, void(*a7)(CGameMenuItemZBool *), const char *a8, const char *a9)
{
    at4 = a1;
    at8 = a2;
    atc = a3;
    at10 = a4;
    at14 = a5;
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
    if (at4)
        gMenuTextMgr.DrawText(at4, at8, atc, at10, shade, 0, false);
    const char *value = at20 ? at21 : at25;
    int width;
    gMenuTextMgr.GetFontInfo(at8, value, &width, NULL);
    gMenuTextMgr.DrawText(value, at8, at14-1+atc-width, at10, shade, 0, false);
}

bool CGameMenuItemZBool::Event(CGameMenuEvent &event)
{
    switch (event.at0)
    {
    case 6:
    case 8:
        at20 = !at20;
        if (at29)
            at29(this);
        return false;
    }
    return CGameMenuItem::Event(event);
}

CGameMenuItemChain::CGameMenuItemChain()
{
    at4 = NULL;
    at24 = NULL;
    at28 = -1;
    at2c = NULL;
    at30 = 0;
}

CGameMenuItemChain::CGameMenuItemChain(const char *a1, int a2, int a3, int a4, int a5, int a6, CGameMenu *a7, int a8, void(*a9)(CGameMenuItemChain *), int a10)
{
    at4 = a1;
    at8 = a2;
    atc = a3;
    at10 = a4;
    at14 = a5;
    at20 = a6;
    at24 = a7;
    at28 = a8;
    at2c = a9;
    at30 = a10;
}

void CGameMenuItemChain::Draw(void)
{
    if (!at4) return;
    int shade = 32;
    if (pMenu->IsFocusItem(this))
        shade = 32-(totalclock&63);
    int width;
    int x = atc;
    switch (at20)
    {
    case 1:
        gMenuTextMgr.GetFontInfo(at8, at4, &width, NULL);
        x = atc+at14/2-width/2;
        break;
    case 2:
        gMenuTextMgr.GetFontInfo(at8, at4, &width, NULL);
        x = atc+at14-1-width;
        break;
    case 0:
    default:
        break;
    }
    gMenuTextMgr.DrawText(at4, at8, x, at10, shade, 0, true);
}

bool CGameMenuItemChain::Event(CGameMenuEvent &event)
{
    switch (event.at0)
    {
    case 6:
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
    at4 = NULL;
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
    at4 = a1;
    at8 = a2;
    atc = a3;
    at10 = a4;
    at14 = a5;
    at20 = a8;
    at28 = a9;
    at2c = a10;
    at30 = a11;
    strncpy(at38, a6, 15);
    strncpy(at48, a7, 15);
}

void CGameMenuItem7EA1C::Draw(void)
{
    if (!at4) return;
    int shade = 32;
    if (pMenu->IsFocusItem(this))
        shade = 32-(totalclock&63);
    int width;
    int x = atc;
    switch (at20)
    {
    case 1:
        gMenuTextMgr.GetFontInfo(at8, at4, &width, NULL);
        x = atc+at14/2-width/2;
        break;
    case 2:
        gMenuTextMgr.GetFontInfo(at8, at4, &width, NULL);
        x = atc+at14-1-width;
        break;
    case 0:
    default:
        break;
    }
    gMenuTextMgr.DrawText(at4, at8, x, at10, shade, 0, true);
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
    case 6:
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
    case 0x8001:
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
    at4 = NULL;
    at28 = NULL;
    at20 = -1;
    at2c = NULL;
}

CGameMenuItem7EE34::CGameMenuItem7EE34(const char *a1, int a2, int a3, int a4, int a5, int a6)
{
    at4 = NULL;
    at28 = NULL;
    at20 = -1;
    at2c = NULL;
    at8 = a2;
    atc = a3;
    at4 = a1;
    at10 = a4;
    at14 = a5;
    at24 = a6;
}

void CGameMenuItem7EE34::Draw(void)
{
    if (!at4) return;
    int shade = 32;
    if (pMenu->IsFocusItem(this))
        shade = 32-(totalclock&63);
    int width;
    int x = atc;
    switch (at24)
    {
    case 1:
        gMenuTextMgr.GetFontInfo(at8, at4, &width, NULL);
        x = atc+at14/2-width/2;
        break;
    case 2:
        gMenuTextMgr.GetFontInfo(at8, at4, &width, NULL);
        x = atc+at14-1-width;
        break;
    case 0:
    default:
        break;
    }
    gMenuTextMgr.DrawText(at4, at8, x, at10, shade, 0, true);
}

extern void SetVideoMode(CGameMenuItemChain *pItem);

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
        pItem1->at18 &= ~2;
        at2c->Add(pItem2, false);
        pItem2->at18 &= ~2;
        at2c->Add(pItem3, true);
        pItem3->at18 |= 2;
        at2c->Add(&itemBloodQAV, false);
    }
#ifndef WIN32CODE
    getvalidvesamodes();
#endif
    sprintf(buffer[0], "640 x 480 (default)");
    int y = 40;
    at28->Add(new CGameMenuItemChain(buffer[0], 3, 0, y, 320, 1, at2c, -1, SetVideoMode, validmodecnt), true);
    y += 20;
    for (int i = 0; i < validmodecnt && i < 20; i++)
    {
        sprintf(buffer[i+1], "%d x %d", validmode[i].xdim, validmode[i].ydim);
        at28->Add(new CGameMenuItemChain(buffer[i+1], 3, 0, y, 320, 1, at2c, -1, SetVideoMode, i), false);
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
    case 6:
        if (at28)
            delete at28;
        at28 = new CGameMenu(1);
        Setup();
        if (at28)
            gGameMenuMgr.Push(at28, at20);
        return false;
    case 0x8001:
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
    case 6:
        if (at34 > -1)
            gGameOptions.nEpisode = at34;
        return CGameMenuItemChain::Event(event);
    }
    return CGameMenuItem::Event(event);
}

CGameMenuItemBitmap::CGameMenuItemBitmap()
{
    at4 = NULL;
}

CGameMenuItemBitmap::CGameMenuItemBitmap(const char *a1, int a2, int a3, int a4, int a5)
{
    at4 = a1;
    at8 = a2;
    atc = a3;
    at10 = a4;
    at20 = a5;
}

void CGameMenuItemBitmap::Draw(void)
{
    int shade = 32;
    if ((at18 & 2) && pMenu->IsFocusItem(this))
        shade = 32-(totalclock&63);
    int x = atc;
    int y = at10;
    if (at4)
    {
        int height;
        gMenuTextMgr.DrawText(at4, at8, x, y, shade, 0, false);
        gMenuTextMgr.GetFontInfo(at8, NULL, NULL, &height);
        y += height + 2;
    }
    rotatesprite(x<<15,y<<15, 65536, 0, at20, 0, 0, 82, 0, 0, xdim-1,ydim-1);
}

bool CGameMenuItemBitmap::Event(CGameMenuEvent &event)
{
    if ((at18 & 2) && pMenu->IsFocusItem(this))
        pMenu->FocusNextItem();
    return CGameMenuItem::Event(event);
}

CGameMenuItemBitmapLS::CGameMenuItemBitmapLS()
{
    at4 = NULL;
}

CGameMenuItemBitmapLS::CGameMenuItemBitmapLS(const char *a1, int a2, int a3, int a4, int a5)
{
    at24 = -1;
    at4 = a1;
    at8 = a2;
    atc = a3;
    at10 = a4;
    at28 = a5;
}

void CGameMenuItemBitmapLS::Draw(void)
{
    int shade = 32;
    if ((at18 & 2) && pMenu->IsFocusItem(this))
        shade = 32-(totalclock&63);
    int x = atc;
    int y = at10;
    if (at4)
    {
        int height;
        gMenuTextMgr.DrawText(at4, at8, x, y, shade, 0, false);
        gMenuTextMgr.GetFontInfo(at8, NULL, NULL, &height);
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
    if ((at18 & 2) && pMenu->IsFocusItem(this))
        pMenu->FocusNextItem();
    return CGameMenuItem::Event(event);
}

CGameMenuItemKeyList::CGameMenuItemKeyList()
{
    at4 = NULL;
    at8 = 3;
    atc = 0;
    at10 = 0;
    at28 = 0;
    at2c = 0;
    at30 = 0;
    at34 = 0;
    at38 = false;
}

CGameMenuItemKeyList::CGameMenuItemKeyList(const char *a1, int a2, int a3, int a4, int a5, int a6, int a7, void(*a8)(CGameMenuItemKeyList *))
{
    at2c = 0;
    at30 = 0;
    at38 = false;
    at4 = a1;
    at8 = a2;
    atc = a3;
    at10 = a4;
    at14 = a5;
    at28 = a6;
    at20 = a8;
    at34 = a7;
}

void CGameMenuItemKeyList::Scan(void)
{
    KB_FlushKeyboardQueue();
    KB_FlushKeyboardQueueScans();
    KB_ClearKeysDown();
    KB_LastScan = 0;
    at38 = true;
}

extern uint8_t KeyboardKeys[NUMGAMEFUNCTIONS][2];
void CGameMenuItemKeyList::Draw(void)
{
    char buffer[40], buffer2[40];
    int width, height;
    int shade;
    gMenuTextMgr.GetFontInfo(at8, NULL, NULL, &height);
    int y = at10;
    int k = at30 - at2c;
    for (int i = 0; i < at28; i++, y += height, k++)
    {
        BYTE key1, key2;
#ifdef WIN32CODE
        key1 = KeyboardKeys[k][0];
        key2 = KeyboardKeys[k][1];
#else
        CONTROL_GetKeyMap(k, &key1, &key2);
#endif
        const char *sKey1 = KB_ScanCodeToString(key1);
        const char *sKey2 = KB_ScanCodeToString(key2);
        sprintf(buffer, "%s", CONFIG_FunctionNumToName(k));
        if (key2 == 0)
        {
            if (key1 == 0)
                sprintf(buffer2, "????");
            else
                sprintf(buffer2, "%s", sKey1);
        }
        else
            sprintf(buffer2, "%s or %s", sKey1, sKey2);
        
        if (k == at30)
        {
            shade = 32;
            if (pMenu->IsFocusItem(this))
                shade = 32-(totalclock&63);
            viewDrawText(3, buffer, atc, y, shade, 0, 0, false);
            const char *sVal;
            if (at38 && (gGameClock & 32))
                sVal = "____";
            else
                sVal = buffer2;
            gMenuTextMgr.GetFontInfo(at8, sVal, &width, 0);
            viewDrawText(at8, sVal, atc+at14-1-width, y, shade, 0, 0, false);
        }
        else
        {
            viewDrawText(3, buffer, atc, y, 24, 0, 0, false);
            gMenuTextMgr.GetFontInfo(at8, buffer2, &width, 0);
            viewDrawText(at8, buffer2, atc+at14-1-width, y, 24, 0, 0, false);
        }
    }
}

bool CGameMenuItemKeyList::Event(CGameMenuEvent &event)
{
    if (at38)
    {
        if (KB_LastScan && KB_LastScan != sc_Pause)
        {
            if (KB_KeyWaiting())
                KB_GetCh();
            BYTE key1, key2;
#ifdef WIN32CODE
            extern uint8_t KeyboardKeys[NUMGAMEFUNCTIONS][2];
            key1 = KeyboardKeys[at30][0];
            key2 = KeyboardKeys[at30][1];
#else
            CONTROL_GetKeyMap(at30, &key1, &key2);
#endif
            if (key1 > 0 && key2 != KB_LastScan)
                key2 = key1;
            key1 = KB_LastScan;
            if (key1 == key2)
                key2 = 0;
#ifdef WIN32CODE
            uint8_t oldKey[2];
            oldKey[0] = KeyboardKeys[at30][0];
            oldKey[1] = KeyboardKeys[at30][1];
            CONFIG_MapKey(at30, key1, oldKey[0], key2, oldKey[1]);
#else
            CONTROL_MapKey(key1, key2);
#endif
            KB_FlushKeyboardQueue();
            KB_FlushKeyboardQueueScans();
            KB_ClearKeysDown();
#ifdef WIN32CODE
            keyFlushScans();
            keyFlushChars();
#else
            keyFlushStream();
#endif
            at38 = 0;
        }
        return false;
    }
    switch (event.at0)
    {
    case 2:
        if (event.at2 == sc_Tab || at30 == 0)
        {
            pMenu->FocusPrevItem();
            return false;
        }
        at30--;
        if (at2c > 0)
            at2c--;
        return false;
    case 3:
        if (event.at2 == sc_Tab || at30 == at34-1)
        {
            pMenu->FocusNextItem();
            return false;
        }
        at30++;
        if (at2c+1 < at28)
            at2c++;
        return false;
    case 6:
        if (at20)
            at20(this);
        Scan();
        return false;
    case 10:
        if (keystatus[sc_LeftControl] || keystatus[sc_RightControl])
        {
#ifdef WIN32CODE
            uint8_t oldKey[2];
            oldKey[0] = KeyboardKeys[at30][0];
            oldKey[1] = KeyboardKeys[at30][1];
            CONFIG_MapKey(at30, 0, oldKey[0], 0, oldKey[1]);
#else
            CONTROL_MapKey(at30, 0, 0);
#endif
        }
        return false;
    }
    return CGameMenuItem::Event(event);
}

CGameMenuItemSlider::CGameMenuItemSlider()
{
    at4 = NULL;
    at8 = -1;
    atc = 0;
    at10 = 0;
    at24 = 0;
    at28 = 0;
    at30 = 0;
    at34 = NULL;
    at20 = NULL;
    at38 = 2204;
    at3c = 2028;
}

CGameMenuItemSlider::CGameMenuItemSlider(const char *a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, void(*a10)(CGameMenuItemSlider *), int a11, int a12)
{
    at4 = a1;
    at8 = a2;
    atc = a3;
    at10 = a4;
    at14 = a5;
    at28 = a7;
    at2c = a8;
    at30 = a9;
    at24 = ClipRange(a6, at28, at2c);
    at34 = a10;
    at38 = 2204;
    at3c = 2028;
    if (a11 >= 0)
        at38 = a11;
    if (a12 >= 0)
        at3c = a12;
}

CGameMenuItemSlider::CGameMenuItemSlider(const char *a1, int a2, int a3, int a4, int a5, int *pnValue, int a7, int a8, int a9, void(*a10)(CGameMenuItemSlider *), int a11, int a12)
{
    at4 = a1;
    at8 = a2;
    atc = a3;
    at10 = a4;
    at14 = a5;
    at28 = a7;
    at2c = a8;
    at30 = a9;
    dassert(pnValue != NULL);
    at20 = pnValue;
    at24 = ClipRange(*pnValue, at28, at2c);
    at34 = a10;
    at38 = 2204;
    at3c = 2028;
    if (a11 >= 0)
        at38 = a11;
    if (a12 >= 0)
        at3c = a12;
}

void CGameMenuItemSlider::Draw(void)
{
    int height;
    at24 = at20 ? *at20 : at24;
    gMenuTextMgr.GetFontInfo(at8, NULL, NULL, &height);
    int shade = 32;
    if (pMenu->IsFocusItem(this))
        shade = 32-(totalclock&63);
    if (at4)
        gMenuTextMgr.DrawText(at4, at8, atc, at10, shade, 0, false);
    int sliderX = atc+at14-1-tilesiz[at38].x/2;
    rotatesprite(sliderX<<16, (at10+height/2)<<16, 65536, 0, at38, 0, 0, 10, 0, 0, xdim-1, ydim-1);
    int nRange = at2c - at28;
    dassert(nRange > 0);
    int nValue = at24 - at28;
    int nWidth = tilesiz[at38].x-8;
    int cursorX = sliderX + ksgn(at30)*(nValue * nWidth / nRange - nWidth / 2);
    rotatesprite(cursorX<<16, (at10+height/2)<<16, 65536, 0, at3c, 0, 0, 10, 0, 0, xdim-1, ydim-1);
}

bool CGameMenuItemSlider::Event(CGameMenuEvent &event)
{
    at24 = at20 ? *at20 : at24;
    switch (event.at0)
    {
    case 2:
        pMenu->FocusPrevItem();
        return false;
    case 3:
        pMenu->FocusNextItem();
        return false;
    case 4:
        if (at24 > 0)
            at24 = DecBy(at24, at30);
        else
            at24 = IncBy(at24, -at30);
        at24 = ClipRange(at24, at28, at2c);
        if (at34)
            at34(this);
        return false;
    case 5:
        if (at24 >= 0)
            at24 = IncBy(at24, at30);
        else
            at24 = DecBy(at24, -at30);
        at24 = ClipRange(at24, at28, at2c);
        if (at34)
            at34(this);
        return false;
    case 6:
        if (at34)
            at34(this);
        return false;
    }
    return CGameMenuItem::Event(event);
}

CGameMenuItemZEdit::CGameMenuItemZEdit()
{
    at4 = NULL;
    at8 = -1;
    atc = 0;
    at10 = 0;
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
    at4 = a1;
    at8 = a2;
    atc = a3;
    at10 = a4;
    at14 = a5;
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
    gMenuTextMgr.GetFontInfo(at8, NULL, NULL, &height);
    int shade = 32;
    if (pMenu->IsFocusItem(this))
        shade = 32-(totalclock&63);
    if (at30)
        shade = -128;
    if (at4)
        gMenuTextMgr.DrawText(at4, at8, atc, at10, shade, 0, false);
    int x = atc+at14-1-(at24+1)*height;
    if (at20 && *at20)
    {
        gMenuTextMgr.GetFontInfo(at8, NULL, &width, NULL);
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
        gMenuTextMgr.DrawText(at20, at8, x, at10, shade2, 0, false);
        x += width;
    }
    if (at30 && (gGameClock & 32))
        gMenuTextMgr.DrawText("_", at8, x, at10, shade, 0, false);
}

bool CGameMenuItemZEdit::Event(CGameMenuEvent &event)
{
    static char buffer[256];
    switch (event.at0)
    {
    case 6:
        if (at30)
        {
            strncpy(at20, buffer, at24);
            at20[at24-1] = 0;
            at30 = 0;
            return false;
        }
        return true;
    case 5:
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
    case 8:
        if (at30)
            BackChar();
        return false;
    case 0:
    case 7:
    {
        char key;
#ifdef WIN32CODE
        key = g_keyAsciiTable[event.at2];
        if (keystatus[sc_LeftShift] || keystatus[sc_RightShift])
            key = Btoupper(key);
#else
        if (keystatus[sc_LeftShift] || keystatus[sc_RightShift])
            key = ScanToAsciiShifted[event.at2];
        else
            key = ScanToAscii[event.at2];
#endif
        if (at30 && (isalnum(key) || ispunct(key) || isspace(key)))
        {
            AddChar(key);
            return false;
        }
        return CGameMenuItem::Event(event);
    }
    case 1:
        if (at30)
            return false;
        return CGameMenuItem::Event(event);
    case 2:
        if (at30)
            return false;
        return CGameMenuItem::Event(event);
    }
    return CGameMenuItem::Event(event);
}

CGameMenuItemZEditBitmap::CGameMenuItemZEditBitmap()
{
    at4 = NULL;
    at8 = -1;
    atc = 0;
    at10 = 0;
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
    at4 = a1;
    at8 = a2;
    atc = a3;
    at10 = a4;
    at14 = a5;
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
    gMenuTextMgr.GetFontInfo(at8, NULL, NULL, &height);
    int shade = 32;
    if (pMenu->IsFocusItem(this))
        shade = 32-(totalclock&63);
    at2c->at24 = -1;
    if (at34)
        shade = -128;
    if (at4)
        gMenuTextMgr.DrawText(at4, at8, atc, at10, shade, 0, false);
    int x = atc+at14-1-(at24+1)*height;
    if (at20 && *at20)
    {
        gMenuTextMgr.GetFontInfo(at8, at20, &width, NULL);
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
        gMenuTextMgr.DrawText(at20, at8, x, at10, shade2, 0, false);
        x += width;
    }
    if (at34 && (gGameClock & 32))
        gMenuTextMgr.DrawText("_", at8, x, at10, shade, 0, false);
}

bool CGameMenuItemZEditBitmap::Event(CGameMenuEvent &event)
{
    static char buffer[256];
    switch (event.at0)
    {
    case 7:
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
    case 6:
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
    case 9:
        if (at34)
            BackChar();
        return false;
    case 1:
    case 8:
    {
        char key;
#ifdef WIN32CODE
        key = g_keyAsciiTable[event.at2];
        if (keystatus[sc_LeftShift] || keystatus[sc_RightShift])
            key = Btoupper(key);
#else
        if (keystatus[sc_LeftShift] || keystatus[sc_RightShift])
            key = ScanToAsciiShifted[event.at2];
        else
            key = ScanToAscii[event.at2];
#endif
        if (at34 && (isalnum(key) || ispunct(key) || isspace(key)))
        {
            AddChar(key);
            return false;
        }
        return CGameMenuItem::Event(event);
    }
    case 2:
        if (at34)
            return false;
        return CGameMenuItem::Event(event);
    case 3:
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
    at18 &= ~2;
}

CGameMenuItemQAV::CGameMenuItemQAV(const char *a1, int a2, int a3, int a4, const char *a5)
{
    at14 = 0;
    at4 = a1;
    at8 = a2;
    at10 = a4;
    at20 = a5;
    atc = a3;
    at18 &= ~2;
}

void CGameMenuItemQAV::Draw(void)
{
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
    case 4:
    case 9:
        pMenu->FocusPrevItem();
        return false;
    case 5:
    case 6:
    case 8:
        pMenu->FocusNextItem();
        return false;
    case 0x8000:
        if (at20)
        {
            if (!at28)
            {
                at24 = gSysRes.Lookup(at20, "QAV");
                if (!at24)
                    ThrowError("Could not load QAV %s\n", at20);
                at28 = (QAV*)gSysRes.Lock(at24);
                at28->x = atc;
                at28->y = at10;
                at28->Preload();
                at2c = at28->at10;
                at30 = totalclock;
                return false;
            }
            gSysRes.Lock(at24);
        }
        return false;
    case 0x8001:
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
    at4 = NULL;
    at24 = 0;
    m_nItems = 0;
    atb4 = 0;
    at2c = 0;
    at30 = 0;
}

CGameMenuItemZCycle::CGameMenuItemZCycle(const char *a1, int a2, int a3, int a4, int a5, int a6, void(*a7)(CGameMenuItemZCycle *), const char **a8, int a9, int a10)
{
    at4 = a1;
    at8 = a2;
    atc = a3;
    at10 = a4;
    at24 = 0;
    at14 = a5;
    at28 = a6;
    atb4 = a7;
    m_nItems = 0;
    SetTextArray(a8, a9, a10);
}

CGameMenuItemZCycle::~CGameMenuItemZCycle()
{
    at4 = NULL;
    at24 = 0;
    m_nItems = 0;
    atb4 = 0;
    memset(at34, 0, sizeof(at34));
    at2c = 0;
    at30 = 0;
}

void CGameMenuItemZCycle::Draw(void)
{
    int width;
    int shade = 32;
    if (pMenu->IsFocusItem(this))
        shade = 32-(totalclock&63);
    int x = atc;
    int y = at10;
    if (at4)
    {
        switch (at28)
        {
        case 1:
            gMenuTextMgr.GetFontInfo(at8, at4, &width, NULL);
            x = atc+at14/2-width/2;
            break;
        case 2:
            gMenuTextMgr.GetFontInfo(at8, at4, &width, NULL);
            x = atc+at14-1-width;
            break;
        case 0:
        default:
            break;
        }
        gMenuTextMgr.DrawText(at4, at8, x, y, shade, 0, false);
    }
    const char *pzText;
    if (!m_nItems)
        pzText = "????";
    else
        pzText = at34[at24];
    dassert(pzText != NULL);
    gMenuTextMgr.GetFontInfo(at8, pzText, &width, NULL);
    gMenuTextMgr.DrawText(pzText, at8, atc + at14 - 1 - width, y, shade, 0, false);
}

bool CGameMenuItemZCycle::Event(CGameMenuEvent &event)
{
    switch (event.at0)
    {
    case 5:
    case 6:
    case 8:
        Next();
        if (atb4)
            atb4(this);
        return false;
    case 4:
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
    at4 = NULL;
    at24 = -1;
    at28 = 0;
}

CGameMenuItemYesNoQuit::CGameMenuItemYesNoQuit(const char *a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8)
{
    at4 = a1;
    at8 = a2;
    atc = a3;
    at10 = a4;
    at14 = a5;
    at20 = a6;
    at24 = a7;
    at28 = a8;
}

void CGameMenuItemYesNoQuit::Draw(void)
{
    if (!at4) return;
    int shade = 32;
    if (pMenu->IsFocusItem(this))
        shade = 32-(totalclock&63);
    int width;
    int x = atc;
    switch (at20)
    {
    case 1:
        gMenuTextMgr.GetFontInfo(at8, at4, &width, NULL);
        x = atc+at14/2-width/2;
        break;
    case 2:
        gMenuTextMgr.GetFontInfo(at8, at4, &width, NULL);
        x = atc+at14-1-width;
        break;
    case 0:
    default:
        break;
    }
    gMenuTextMgr.DrawText(at4, at8, x, at10, shade, 0, true);
}

extern void Quit(CGameMenuItemChain *pItem);

bool CGameMenuItemYesNoQuit::Event(CGameMenuEvent &event)
{
    switch (event.at0)
    {
    case 1:
        if (event.at2 == sc_Y)
            Quit(NULL);
        else if (event.at2 == sc_N)
            gGameMenuMgr.Pop();
        return false;
    case 6:
        Quit(NULL);
        return false;
    }
    return CGameMenuItem::Event(event);
}

CGameMenuItemPicCycle::CGameMenuItemPicCycle()
{
    at4 = NULL;
    at24 = 0;
    m_nItems = 0;
    atb0 = 0;
    at2c = 0;
    atb4 = 0;
}

CGameMenuItemPicCycle::CGameMenuItemPicCycle(int a1, int a2, void(*a3)(CGameMenuItemPicCycle *), int *a4, int a5, int a6)
{
    at14 = 0;
    at24 = 0;
    m_nItems = 0;
    atc = a1;
    at10 = a2;
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
    case 1:
    case 2:
    case 4:
        Next();
        if (atb0)
            atb0(this);
        return false;
    case 0:
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
    at4 = NULL;
    at36 = 0;
    at32 = 0;
    at5b = 0;
}

CGameMenuItemPassword::CGameMenuItemPassword(const char *a1, int a2, int a3, int a4)
{
    at37 = 0;
    at14 = 0;
    at36 = 0;
    at32 = 0;
    at5b = 0;
    at4 = a1;
    at8 = a2;
    atc = a3;
    at10 = a4;
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
    int x;
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
        gMenuTextMgr.GetFontInfo(at8, at3b, &width, NULL);
        gMenuTextMgr.DrawText(at3b, at8, atc-width/2, at10+20, shadef, 0, false);
        shadef = 32;
        break;
    case 4:
        if ((totalclock - at5b) & 32)
        {
            gMenuTextMgr.GetFontInfo(at8, kInvalidPasswordMsg, &width, NULL);
            gMenuTextMgr.DrawText(kInvalidPasswordMsg, at8, atc - width / 2, at10 + 20, shade, 0, false);
        }
        if (at5b && totalclock-at5b > 256)
        {
            at5b = 0;
            at37 = 0;
        }
        break;
    }
    gMenuTextMgr.GetFontInfo(at8, at4, &width, NULL);
    gMenuTextMgr.DrawText(at4, at8, atc-width/2, at10, focus ? shadef : shade, 0, false);
}

bool CGameMenuItemPassword::Event(CGameMenuEvent &event)
{
    switch (at37)
    {
    case 0:
    case 4:
        if (event.at0 == 6)
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
        case 5:
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
        case 6:
            at37 = 0;
            Draw();
            return false;
        case 0:
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
        case 8:
            if (at32 > 0)
                at29[--at32] = 0;
            return false;
        case 3:
        case 4:
        case 7:
            return false;
        }
    }
    return CGameMenuItem::Event(event);
}
