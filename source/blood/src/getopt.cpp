#include <stdlib.h>
#include <string.h>
#include "getopt.h"

int margc;
char const * const *margv;

const char *OptArgv[16];
int OptArgc;
const char *OptFull;
const char *SwitchChars = "-/";

int GetOptions(SWITCH *switches)
{
    static const char *pChar = NULL;
    static int OptIndex = 1;
    if (!pChar)
    {
        if (OptIndex >= margc)
            return -1;
        pChar = margv[OptIndex++];
        if (!pChar)
            return -1;
    }
    OptFull = pChar;
    if (!strchr(SwitchChars, *pChar))
    {
        pChar = NULL;
        return -2;
    }
    pChar++;
    int i;
    int vd;
    for (i = 0; true; i++)
    {
        if (!switches[i].name)
            return -3;
        int nLength = strlen(switches[i].name);
        if (!strnicmp(pChar, switches[i].name, nLength))
        {
            pChar += nLength;
            break;
        }
    }
    vd = switches[i].at4;
    if (*pChar == 0)
        pChar = NULL;
    OptArgc = 0;
    while (OptArgc < switches[i].at8)
    {
        if (!pChar)
        {
            if (OptIndex >= margc)
                break;
            pChar = margv[OptIndex++];
        }
        if (strchr(SwitchChars, *pChar) != 0)
            break;
        OptArgv[OptArgc++] = pChar;
        pChar = NULL;
    }
    return vd;
}
