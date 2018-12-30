#pragma once


extern int margc;
extern char const * const *margv;

extern const char *OptArgv[16];
extern int OptArgc;
extern const char *OptFull;

struct SWITCH {
    const char *name;
    int at4, at8;
};
int GetOptions(SWITCH *switches);