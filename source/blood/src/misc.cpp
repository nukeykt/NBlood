#include <stdio.h>
#include <string.h>
#include "common_game.h"

#include "misc.h"

void *ResReadLine(char *buffer, unsigned long nBytes, void **pRes)
{
    int i;
    char ch;
    if (!pRes || !*pRes || *((char*)*pRes) == 0)
        return false;
    for (i = 0; i < nBytes; i++)
    {
        ch = *((char*)*pRes);
        if(ch == 0 || ch == '\n')
            break;
        buffer[i] = ch;
        *pRes = ((char*)*pRes)+1;
    }
    if (*((char*)*pRes) == '\n' && i < nBytes)
    {
        ch = *((char*)*pRes);
        buffer[i] = ch;
        *pRes = ((char*)*pRes)+1;
        i++;
    }
    else
    {
        while (true)
        {
            ch = *((char*)*pRes);
            if (ch == 0 || ch == '\n')
                break;
            *pRes = ((char*)*pRes)+1;
        }
        if (*((char*)*pRes) == '\n')
            *pRes = ((char*)*pRes)+1;
    }
    if (i < nBytes)
        buffer[i] = 0;
    return *pRes;
}

bool FileRead(FILE *handle, void *buffer, unsigned long size)
{
    return fread(buffer, 1, size, handle) == size;
}

bool FileWrite(FILE *handle, void *buffer, unsigned long size)
{
    return fwrite(buffer, 1, size, handle) == size;
}

bool FileLoad(const char *name, void *buffer, unsigned long size)
{
    dassert(buffer != NULL);

    FILE *handle = fopen(name, "rb");
    if (!handle)
        return false;

    int nread = fread(buffer, 1, size, handle);
    fclose(handle);
    return nread == size;
}

int FileLength(FILE *handle)
{
    if (!handle)
        return 0;
    int nPos = ftell(handle);
    fseek(handle, 0, SEEK_END);
    int nLength = ftell(handle);
    fseek(handle, nPos, SEEK_SET);
    return nLength;
}

unsigned long randSeed = 1;

unsigned long qrand(void)
{
    if (randSeed&0x80000000)
        randSeed = ((randSeed<<1)^0x20000004)|0x1;
    else
        randSeed = randSeed<<1;
    return randSeed&0x7fff;
}

void ChangeExtension(char *pzFile, const char *pzExt)
{
    char drive[BMAX_PATH];
    char dir[BMAX_PATH];
    char filename[BMAX_PATH];
    _splitpath(pzFile, drive, dir, filename, NULL);
    _makepath(pzFile, drive, dir, filename, pzExt);
}
