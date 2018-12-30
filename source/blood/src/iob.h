#pragma once

class IOBuffer
{
public:
    IOBuffer(int _nRemain, char *pBuffer);
    int nRemain;
    char *pBuffer;
    void Read(void *, int);
    void Write(void *, int);
    void Skip(int);
};