// Copyright: 2020 Nuke.YKT, EDuke32 developers
// License: GPLv2
#include "compat.h"
#include "reality.h"
#include "../duke3d.h"

struct Huffman {
    uint32_t f_0;
    uint16_t f_2;
    uint32_t f_8;
    uint16_t f_c;
};

static char nBitsLoaded;
static const char *pStream;
static const char *pStreamEnd;
static char *pOutput;
static char *pOutputEnd;
static uint32_t loadedBits;
static Huffman huffVars[3][16];

static void SetupDecompression(const char *inbuf, char *outbuf)
{
    nBitsLoaded = 0;
    pStream = inbuf + 18; // skip header
    pStreamEnd = pStream + B_BIG32(*(int*)(inbuf + 8));
    pOutput = outbuf;
    pOutputEnd = outbuf + B_BIG32(*(int*)(inbuf + 4));
}

static int GetBitsFromStream(uint8_t nBits)
{
    int ret = 0;
    int v1 = 1;

    for (int i = 0; i < nBits; i++)
    {
        if (!nBitsLoaded)
        {
            nBitsLoaded = 16;
            loadedBits = B_LITTLE32(*(int*)pStream);
            pStream += 2;
        }
        nBitsLoaded--;

        char v3 = loadedBits & 1;
        loadedBits >>= 1;
        if (v3)
            ret |= v1;
        v1 <<= 1;
    }

    return ret;
}

static void InitTable(Huffman *table, int nSize)
{
    for (int i = 0; i < nSize; i++)
    {
        table[i].f_0 = 0;
        table[i].f_2 = -1;
        table[i].f_8 = 0;
        table[i].f_c = 0;
    }
}

static uint32_t sub_80070488(uint32_t a, int a2)
{
    int v2 = 0;
    for (int i = a2 - 1; i >= 0; i--)
    {
        v2 <<= 1;
        if (a & 1)
            v2 |= 1;
        a >>= 1;
    }
    return v2;
}

static void sub_800704dc(Huffman *table, int nSize)
{
    uint32_t v4 = 0;
    uint32_t v5 = 1UL << 31;

    for (int i = 1; i < 17; i++)
    {
        for (int j = 0; j < nSize; j++)
        {
            if (table[j].f_c == i)
            {
                table[j].f_8 = sub_80070488(v4 / v5, i);
                v4 += v5;
            }
        }
        v5 >>= 1;
    }
}

static void LoadTable(Huffman *table, int nSize)
{
    InitTable(table, nSize);

    int nLeafNodes = GetBitsFromStream(5);
    if (nLeafNodes > 16)
        nLeafNodes = 16;

    for (int i = 0; i < nLeafNodes; i++)
    {
        table[i].f_c = GetBitsFromStream(4); // leaf depth
    }

    sub_800704dc(table, nSize);
}

static int GetTableLength(Huffman *a1)
{
    int v6 = 0;
    while (a1->f_c == 0 || a1->f_8 != (((1UL << ((a1->f_c & 31))) - 1) & loadedBits))
    {
        a1++;
        v6++;
    }
    GetBitsFromStream(a1->f_c);
    if (v6 > 1)
    {
        v6--;
        return GetBitsFromStream(v6) | (1 << ((v6) & 31));
    }
    return v6;
}

static int RNCDecompress1(const char *inbuf, char *outbuf)
{
    SetupDecompression(inbuf, outbuf);
    GetBitsFromStream(2); // ignore first two bits

    while (pOutput < pOutputEnd) // go through the pack chunks
    {
        LoadTable(huffVars[0], 16); // raw table
        LoadTable(huffVars[1], 16); // distance table
        LoadTable(huffVars[2], 16); // length table

        int nSubChunks = GetBitsFromStream(16); // get count of subchunks in this pack chunk

        for (int j = 0; j < nSubChunks; j++)
        {
            int nLength = GetTableLength(huffVars[0]);

            for (int i = 0; i < nLength; i++)
            {
                *pOutput++ = *pStream++;
            }

            loadedBits = (loadedBits & ((1 << (nBitsLoaded & 31)) - 1))
                | ((pStream[0] + (pStream[1] << 8) + (pStream[2] << 16)) << (nBitsLoaded & 31));

            if (j < (nSubChunks - 1))
            {
                nLength = GetTableLength(huffVars[1]) + 1;
                char* v3 = pOutput - nLength;
                nLength = GetTableLength(huffVars[2]) + 2;

                for (int i = 0; i < nLength; i++)
                {
                    *pOutput++ = *v3++;
                }
            }
        }
    }
    return 0;
}

static int RNCDecompress2(const char *inbuf, char *outbuf)
{
    return 0;
}

int RNCDecompress(const char *inbuf, char *outbuf)
{
    if (inbuf[0] == 'R' && inbuf[1] == 'N' && inbuf[2] == 'C')
    {
        int const type = inbuf[3];
        switch (type)
        {
        case 0:
            Bmemcpy(outbuf, inbuf + 8, B_BIG32(*(int*)(inbuf + 4)));
            return 0;
        case 1:
            return RNCDecompress1(inbuf, outbuf);
        case 2:
            return RNCDecompress2(inbuf, outbuf);
        default:
            return -2;
        }
    }
    return -1;
}

int RT_FakeKRand(void)
{
    // if (rt_gamestate == 2)
        return _krand();
    // return 42;
}

int RT_KRand2(void)
{
    // if (rt_gamestatus == 0)
        return _krand();
    // return 62007;
}

static float rt_gatable[513];

void RT_BuildAngleTable(void)
{
    for (int i = 0; i <= 512; i++)
        rt_gatable[i] = atanf(i * (1.f / 512.f));
}

float RT_GetAngle(float dy, float dx)
{
    if (dy == 0.f && dx == 0.f)
        return 0.f;

    if (dy < 0)
    {
        if (dx < 0)
        {
            if (dy < dx)
                return -(fPI/2.f) - rt_gatable[Blrintf(dx / dy * 512.f)];
            else
                return -fPI + rt_gatable[Blrintf(dy / dx * 512.f)];
        }
        else
        {
            if (-dy > dx)
                return -(fPI/2.f) + rt_gatable[Blrintf(-dx / dy * 512.f)];
            else
                return -rt_gatable[Blrintf(-dy / dx * 512.f)];
        }
    }
    else
    {
        if (dx < 0)
        {
            if (dy > -dx)
                return (fPI/2.f) + rt_gatable[Blrintf(-dx / dy * 512.f)];
            else
                return fPI - rt_gatable[Blrintf(-dy / dx * 512.f)];
        }
        else
        {
            if (dy > dx)
                return (fPI/2.f) - rt_gatable[Blrintf(dx / dy * 512.f)];
            else
                return rt_gatable[Blrintf(dy / dx * 512.f)];
        }
    }
    return 0.f;
}

float RT_AngleMod(float a)
{
    while (a < 0.f)
    {
        a += 360.f;
    }
    while (a >= 360.f)
    {
        a -= 360.f;
    }
    return a;
}
