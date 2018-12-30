#include <stdlib.h>
#include <string.h>
#include "compat.h"
#include "common_game.h"
#include "iob.h"

IOBuffer::IOBuffer(int _nRemain, char *_pBuffer)
{
    nRemain = _nRemain;
    pBuffer =_pBuffer;
}

void IOBuffer::Read(void *pData, int nSize)
{
    if (nSize <= nRemain)
    {
        memcpy(pData, pBuffer, nSize);
        nRemain -= nSize;
        pBuffer += nSize;
    }
    else
    {
        ThrowError("Read buffer overflow");
    }
}

void IOBuffer::Write(void *pData, int nSize)
{
    if (nSize <= nRemain)
    {
        memcpy(pBuffer, pData, nSize);
        nRemain -= nSize;
        pBuffer += nSize;
    }
    else
    {
        ThrowError("Write buffer overflow");
    }
}

void IOBuffer::Skip(int nSize)
{
    if (nSize <= nRemain)
    {
        nRemain -= nSize;
        pBuffer += nSize;
    }
    else
    {
        ThrowError("Skip overflow");
    }
}
