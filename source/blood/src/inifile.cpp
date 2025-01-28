//-------------------------------------------------------------------------
/*
Copyright (C) 2010-2019 EDuke32 developers and contributors
Copyright (C) 2019 sirlemonhead, Nuke.YKT
Copyright (C) NoOne

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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "common_game.h"

#include "inifile.h"



void strTrim(char* str, char side = 0x03)
{
    int l = strlen(str);
    int c;

    if (side & 0x01)
    {
        c = 0;
        while(str[c] && isspace(str[c])) c++;
        if (c)
            memcpy(str, &str[c], l - c + 1), l-=c;
    }


    if (side & 0x02)
    {
        while(--l >= 0 && isspace(str[l]));
        str[l + 1] = '\0';
    }
}


void IniFile::Init()
{
    node = NULL;
    numnodes = numgroups = 0;
    memset(filename, 0, sizeof(filename));
};

IniFile::~IniFile()
{
    int i = numnodes;
    while(--i >= 0)
    {
        ININODE* pNode = &node[i];
        if (pNode->hiWord) free(pNode->hiWord);
        if (pNode->loWord) free(pNode->loWord);
    }

    if (node)
        free(node);
}

IniFile::IniFile(const char* fileName, char flags)
{
    int nLength, hFil;
    unsigned char* pRaw;
    Init();

    if (!isempty(fileName))
    {
        if ((hFil = kopen4loadfrommod(fileName, 0)) >= 0)
        {
            if ((nLength = kfilelength(hFil)) > 0 && (pRaw = (unsigned char*)Xcalloc(1, nLength + 1)) != NULL)
            {
                kread(hFil, pRaw, nLength); pRaw[nLength] = '\0';
                Load(pRaw, nLength+1, flags);
                Xfree(pRaw);
            }

            kclose(hFil);
        }

        strcpy(filename, fileName);
    }
}

IniFile::IniFile(unsigned char* pBytes, int nLength, char flags)
{
    unsigned char* pRaw;
    Init();

    if (nLength > 0 && (pRaw = (unsigned char*)malloc(nLength+1)) != NULL)
    {
        memcpy(pRaw, pBytes, nLength);
        pRaw[nLength] = '\0';
        Load(pRaw, nLength+1, flags);
        free(pRaw);
    }
}

void IniFile::Load(unsigned char* pRaw, int nLength, char flags)
{
    char *ss, *se, *hi, *lo;
    unsigned char c;
    ININODE newNode;
    int n;

    if (!pRaw || nLength <= 0)
        return;

    n = 0;
    while(n < nLength)
    {
        c = pRaw[n];
        if (c == '\r' || c == '\n')
        {
            if (c == '\r')
            {
                if (n+1 < nLength && pRaw[n+1] == '\n')
                {
                    memcpy(&pRaw[n], &pRaw[n+1], nLength - n);
                    nLength--;
                }
            }

            pRaw[n] = '\0';
        }

        n++;
    }

    n = 0;
    while(n < nLength)
    {
        ss = (char*)&pRaw[n]; n+=strlen(ss)+1;
        memset(&newNode, 0, sizeof(newNode));
        hi = NULL; lo = NULL;
        strTrim(ss);

        switch(*ss)
        {
            case '\0':
                if (flags & INI_SKIPZR) continue;
                newNode.type = kIniNodeEmpty;
                break;
            case ';':
            case '#':
                if (flags & INI_SKIPCM) continue;
                newNode.type = kIniNodeComment;
                hi = ss;
                break;
            case '[':
                if ((se = strchr(ss, ']')) > ss)
                {
                    // section
                    hi = &ss[1], *se = '\0'; strTrim(hi);
                    newNode.type = kIniNodeSection;
                    numgroups++;
                    break;
                }
                // allow it to be a key?
                fallthrough__;
            default:
                if ((se = strchr(ss, '=')) <= ss)
                {
                    // simple string key
                    newNode.type = kIniNodeKeyStr;
                    hi = ss;
                }
                else
                {
                    // key / value separated
                    newNode.type = kIniNodeKeySep;
                    hi = ss; *se = '\0'; strTrim(hi);
                    se++; lo = se; strTrim(lo);
                }
                break;
        }

        if (hi) NodeSetWord(&newNode.hiWord, hi);
        if (lo) NodeSetWord(&newNode.loWord, lo);
        NodeAdd(&newNode, -1);
    }

}

char IniFile::Save(const char* saveName)
{
    const char* pSaveName = (saveName) ? saveName : filename;
    int hFil, i, t = 0; char c[1];
    ININODE* pNode;

    if (isempty(pSaveName))
        pSaveName = "inifile.ini";

    if ((hFil = open(pSaveName, O_CREAT|O_WRONLY|O_TEXT|O_TRUNC, S_IREAD|S_IWRITE)) >= 0)
    {
        if (numnodes > 0)
        {
            i = 0;
            while (1)
            {
                pNode = &node[i];
                switch (pNode->type)
                {
                    case kIniNodeEmpty:
                        break;
                    case kIniNodeSection:
                        c[0] = '['; t += write(hFil, c, 1);
                        t += write(hFil, pNode->hiWord, strlen(pNode->hiWord));
                        c[0] = ']'; t += write(hFil, c, 1);
                        break;
                    case kIniNodeKeySep:
                        t += write(hFil, pNode->hiWord, strlen(pNode->hiWord));
                        c[0] = '='; t += write(hFil, c, 1);
                        t += write(hFil, pNode->loWord, strlen(pNode->loWord));
                        break;
                    default:
                        t += write(hFil, pNode->hiWord, strlen(pNode->hiWord));
                        break;
                }

                if (++i == numnodes)
                    break;

                c[0] = '\n';    t += write(hFil, c, 1);
            }
        }

        close(hFil);
        return 1;
    }

    return 0;

}

const char* IniFile::GetKeyString(const char* section, const char *key, const char* defValue)
{
    int nID;
    if ((nID = KeyFind(section, key)) >= 0)
    {
        ININODE* pNode = &node[nID];
        return (pNode->type == kIniNodeKeySep) ? pNode->loWord : pNode->hiWord;
    }

    return defValue;
}

int IniFile::GetKeyInt(const char *section, const char *key, int nRetn)
{
    const char* pVal;
    if ((pVal = GetKeyString(section, key)) != NULL)
        nRetn = strtol(pVal, NULL, 0);

    return nRetn;
}

char IniFile::GetNextString(char* out, const char** pKey, const char** pVal, int* prevNode, const char *section)
{
    ININODE* pNode;
    int i = 0;

    if (*prevNode < 0 && section)
    {
        *prevNode = SectionFind(section);
        if (!rngok(*prevNode, 0, numnodes))
            return 0;
    }

    while( 1 )
    {
        *prevNode = *prevNode + 1;
        if (!rngok(*prevNode, 0, numnodes))
            return 0;

        pNode = &node[*prevNode];
        if (section && pNode->type == kIniNodeSection) return 0;
        else if (pNode->type == kIniNodeEmpty) continue;
        else break;
    }

    if (out)
    {
        if (pKey)
        {
            *pKey = &out[i]; i+=sprintf(&out[i], "%s", pNode->hiWord)+1;
        }

        if (pVal)
        {
            if (pNode->loWord)
            {
                *pVal = &out[i]; i+=sprintf(&out[i], "%s", pNode->loWord)+1;
            }
            else
            {
                *pVal = NULL;
            }
        }
    }
    else
    {
        if (pKey) *pKey = pNode->hiWord;
        if (pVal) *pVal = pNode->loWord;
    }

    return 1;
}

char IniFile::GetNextString(const char** pKey, const char** pVal, int* prevNode, const char *section)
{
    return GetNextString(NULL, pKey, pVal, prevNode, section);
}

int IniFile::GetNextSection(char** section)
{
    int i = 0;
    if (*section == NULL || (i = SectionFind(*section) + 1) > 0)
    {
        while (i < numnodes)
        {
            ININODE* pNode = &node[i];
            if (pNode->type == kIniNodeSection)
            {
                *section = pNode->hiWord;
                return 1;
            }

            i++;
        }
    }

    return 0;
}

void IniFile::NodeSetWord(char** wordPtr, const char* string)
{
    if (*wordPtr)
        free(*wordPtr);

    *wordPtr = NULL;
    if (!string)
        return;

    // empty strings allowed!
    *wordPtr = (char*)malloc(strlen(string)+1);
    dassert(*wordPtr != NULL);
    sprintf(*wordPtr, "%s", string);
}

int IniFile::NodeAdd(ININODE* pNode, int nPos)
{
    node = (ININODE*)realloc(node, sizeof(ININODE)*(numnodes + 1));
    dassert(node != NULL);

    if (!rngok(nPos, 0, numnodes)) nPos = numnodes;
    else memcpy(&node[nPos+1], &node[nPos], sizeof(ININODE)*(numnodes-nPos));

    memcpy(&node[nPos], pNode, sizeof(ININODE));
    numnodes++;
    return nPos;
}

int IniFile::NodeAddEmpty(int nPos)
{
    ININODE newNode;
    memset(&newNode, 0, sizeof(newNode));
    newNode.type = kIniNodeEmpty;
    return NodeAdd(&newNode, nPos);
}

void IniFile::NodeRemove(int nID)
{
    if (nID < numnodes - 1)
        memcpy(&node[nID], &node[nID+1], sizeof(ININODE)*(numnodes-nID));

    numnodes--;
    node = (ININODE*)realloc(node, sizeof(ININODE)*numnodes);
    if (numnodes > 0)
        dassert(node != NULL);
}

char IniFile::NodeComment(int nID, char hashChr)
{
    char c = (hashChr) ? '#' : ';';
    ININODE* pNode = &node[nID];
    char* buf;
    int n;

    n = 1;
    if (pNode->hiWord)
    {
        switch(*pNode->hiWord)
        {
            case '#':
            case ';':
                *pNode->hiWord = c;
                return 1;
        }

        n += strlen(pNode->hiWord);
    }

    if (pNode->loWord)
        n += strlen(pNode->loWord);

    if ((buf = (char*)malloc(n+8)) != NULL)
    {
        buf[0] = c;
        switch(pNode->type)
        {
            case kIniNodeEmpty:
                buf[1] = '\0';
                break;
            case kIniNodeKeySep:
                sprintf(&buf[1], "%s=%s", pNode->hiWord, pNode->loWord);
                break;
            case kIniNodeSection:
                sprintf(&buf[1], "[%s]", pNode->hiWord);
                break;
            default:
                sprintf(&buf[1], "%s", pNode->hiWord);
                break;
        }

        NodeSetWord(&pNode->hiWord, buf);
        NodeSetWord(&pNode->loWord, NULL);
        pNode->type = kIniNodeComment;
        free(buf);
        return 1;
    }

    return 0;
}

int IniFile::SectionFind(const char* name)
{
    int i = numnodes;
    while(--i >= 0)
    {
        ININODE* pNode = &node[i];
        if (pNode->type == kIniNodeSection && Bstrcasecmp(pNode->hiWord, name) == 0)
            break;
    }

    return i;
}

int IniFile::SectionAdd(const char* section)
{
    int nID;
    if ((nID = SectionFind(section)) >= 0)
        return nID;

    if (numnodes > 0
        && node[numnodes-1].type != kIniNodeEmpty)
            NodeAddEmpty(-1);

    ININODE newNode;
    memset(&newNode, 0, sizeof(newNode));
    NodeSetWord(&newNode.hiWord, section);
    newNode.type = kIniNodeSection;
    return NodeAdd(&newNode, -1);
}

void IniFile::SectionRemove(const char* section)
{
    int nID;
    while((nID = SectionFind(section)) >= 0)
    {
        do
        {
            NodeRemove(nID);
        }
        while(nID < numnodes
            && node[nID].type != kIniNodeSection);
    }
}

int IniFile::KeyFind(const char* section, const char* key)
{
    int nID;
    if ((nID = SectionFind(section)) >= 0)
    {
        while(++nID < numnodes && node[nID].type != kIniNodeSection)
        {
            switch(node[nID].type)
            {
                case kIniNodeKeySep:
                case kIniNodeKeyStr:
                    if (Bstrcasecmp(node[nID].hiWord, key) == 0) return nID;
                    break;
            }
        }
    }

    return -1;
}

int IniFile::KeyAddNew(const char* section, const char* hiWord, const char* loWord)
{
    ININODE newNode;
    int nID;

    memset(&newNode, 0, sizeof(newNode));
    NodeSetWord(&newNode.hiWord, hiWord);
    NodeSetWord(&newNode.loWord, loWord);
    newNode.type = (newNode.loWord) ? kIniNodeKeySep : kIniNodeKeyStr;
    if ((nID = SectionFind(section)) >= 0)
    {
        while(++nID < numnodes
            && node[nID].type != kIniNodeEmpty && node[nID].type != kIniNodeSection);
    }
    else
    {
        SectionAdd(section);
    }

    return NodeAdd(&newNode, nID);
}

int IniFile::KeyAdd(const char* section, const char* hiWord, const char* loWord)
{
    ININODE *pNode;
    int nID;

    if ((nID = KeyFind(section, hiWord)) >= 0)
    {
        pNode = &node[nID];
        NodeSetWord(&pNode->hiWord, hiWord);
        NodeSetWord(&pNode->loWord, loWord);
        pNode->type = (pNode->loWord) ? kIniNodeKeySep : kIniNodeKeyStr;
        return nID;
    }

    return KeyAddNew(section, hiWord, loWord);
}

void IniFile::KeyRemove(const char* section, const char* hiWord)
{
    int nID;
    while((nID = KeyFind(section, hiWord)) >= 0)
        NodeRemove(nID);
}

char IniFile::PutKeyInt(const char* section, const char* key, const int nVal)
{
    char buf[32]; sprintf(buf, "%d", nVal);
    if (!key)
        return (KeyAdd(section, buf, NULL) >= 0);

    return (KeyAdd(section, key, buf) >= 0);
}

char IniFile::PutKeyHex(const char* section, const char* key, const int nVal)
{
    char buf[32]; sprintf(buf, "0x%d", nVal);
    if (!key)
        return (KeyAdd(section, buf, NULL) >= 0);

    return (KeyAdd(section, key, buf) >= 0);
}
