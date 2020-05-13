
#include "view.h"
#include "menu.h"
#include "witchaven.h"

static int displaytime;
char displaybuf[50];

void StatusMessage(int nTime, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    vsprintf(displaybuf, fmt, args);

    displaytime = nTime;
}

void DisplayStatusMessage()
{
    if (displaytime > 0)
    {
        fancyfontscreen(18, 24, THEFONT, displaybuf);
        displaytime -= synctics;
    }
}

void StatusMessageDisplayTime(int nTime)
{
    displaytime = nTime;
}
