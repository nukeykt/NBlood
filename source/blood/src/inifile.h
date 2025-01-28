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
#pragma once
enum
{
    kIniNodeEmpty   = 0,
    kIniNodeComment = 1,
    kIniNodeSection = 2,
    kIniNodeKeyStr  = 3,
    kIniNodeKeySep  = 4,
};

#pragma pack(push, 1)
struct ININODE
{
    char    type;
    char *hiWord;
    char *loWord;
};
#pragma pack(pop)

enum
{
    INI_NORMAL      = 0x00,
    INI_SKIPCM      = 0x01,
    INI_SKIPZR      = 0x02,
};

#define INI_DEFAULT INI_SKIPCM|INI_SKIPZR

class IniFile
{
    private:
    ININODE* node;
    int numgroups;
    int numnodes;
    public:
    char filename[BMAX_PATH];
    //---------------------------------------------------------------
    IniFile(unsigned char* pRaw, int nLength, char flags = INI_DEFAULT);
    IniFile(const char* fileName, char flags = INI_DEFAULT);
    ~IniFile();
    //---------------------------------------------------------------
    void Init();
    void Load(unsigned char* pRaw, int nLength, char flags = INI_DEFAULT);
    char Save(const char* saveName = NULL);
    //---------------------------------------------------------------
    int  NodeAdd(ININODE* pNode, int nPos);
    int  NodeAddEmpty(int nPos);
    void NodeSetWord(char** wordPtr, const char* string);
    void NodeRemove(int nID);
    char NodeComment(int nID, char hashChr);
    //---------------------------------------------------------------
    int  SectionFind(const char* name);
    int  SectionAdd(const char* section);
    void SectionRemove(const char* section);
    //---------------------------------------------------------------
    int  KeyFind(const char* section, const char* key);
    int  KeyAdd(const char* section, const char* hiWord, const char* loWord);
    int  KeyAddNew(const char* section, const char* hiWord, const char* loWord);
    void KeyRemove(const char* section, const char* hiWord);
    //---------------------------------------------------------------
    const char* GetKeyString(const char* section, const char* key, const char* pRetn = NULL);
    int   GetKeyInt(const char* section, const char* key, int nRetn = -1);
    char  GetNextString(char* out, const char** pKey, const char** pVal, int* prevNode, const char *section = NULL);
    char  GetNextString(const char** pKey, const char** pVal, int* prevNode, const char *section = NULL);
    int GetNextSection(char** pSection);
    //---------------------------------------------------------------
    char  PutKeyInt(const char* section, const char* hiWord, const int nVal);
    char  PutKeyHex(const char* section, const char* hiWord, const int nVal);
    //---------------------------------------------------------------
    inline char PutKeyString(const char* section, const char* hiWord, const char* loWord = NULL)    { return (KeyAdd(section, hiWord, loWord) >= 0); }
    inline char GetKeyBool(const char *section, const char *key, int nRetn)                         { return (GetKeyInt(section, key, nRetn) != 0); }
    inline int  GetKeyHex(const char *section, const char *key, int nRetn)                          { return GetKeyInt(section, key, nRetn); }
    inline char KeyExists(const char* section, const char* key)                                     { return (KeyFind(section, key) >= 0); }
    inline char SectionExists(const char* name)                                                     { return (SectionFind(name) >= 0); }
    inline char FindSection(const char* name)                                                       { return (SectionFind(name) >= 0); }
    inline void RemoveSection(const char* name)                                                     { SectionRemove(name); }
};