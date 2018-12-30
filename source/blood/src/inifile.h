// Note: This module is based on the sirlemonhead's work
#pragma once
struct FNODE
{
    FNODE *next;
    char name[1];
};

// 161 bytes
class IniFile
{
public:
    IniFile(const char *fileName);
    IniFile(void *res);
    ~IniFile();

    void Save(void);
    bool FindSection(const char *section);
    bool SectionExists(const char *section);
    bool FindKey(const char *key);
    void AddSection(const char *section);
    void AddKeyString(const char *key, const char *value);
    void ChangeKeyString(const char *key, const char *value);
    bool KeyExists(const char *section, const char *key);
    void PutKeyString(const char *section, const char *key, const char *value);
    const char* GetKeyString(const char *section, const char *key, const char *defaultValue);
    void PutKeyInt(const char *section, const char *key, const int value);
    int GetKeyInt(const char *section, const char *key, int defaultValue);
    void PutKeyHex(const char *section, const char *key, int value);
    int GetKeyHex(const char *section, const char *key, int defaultValue);
    bool GetKeyBool(const char *section, const char *key, int defaultValue);
    void RemoveKey(const char *section, const char *key);
    void RemoveSection(const char *section);

private:
    FNODE head;
    FNODE *curNode;
    FNODE *anotherNode;

    char *_13;

    char fileName[144]; // watcom maxpath
    
    void LoadRes(void *);
    void Load();
};