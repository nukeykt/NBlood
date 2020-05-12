#include "compat.h"
#include "reality.h"
#include "../duke3d.h"

struct Huffman {
    uint32_t f_0;
    uint16_t f_2;
    uint32_t f_8;
    uint16_t f_c;
};

static char byte_8026ecc1;
static const char *byte_8026ecc4;
static char *byte_8026ecc8;
static char *byte_8026eccc;
static uint32_t dword_8026ecd0;
static Huffman byte_8026ecd8[3][16];

static void sub_800706b8(const char *inbuf, char *outbuf)
{
    byte_8026ecc1 = 0;
    byte_8026ecc4 = inbuf + 0x12;
    byte_8026ecc8 = outbuf;
    byte_8026eccc = outbuf + B_BIG32(*(int*)(inbuf+4));
}

static int sub_800702d4(uint8_t a1)
{
    int v2 = 0;
    int v1 = 1;
    for (int i = a1 - 1; i >= 0; i--)
    {
        if (!byte_8026ecc1)
        {
            byte_8026ecc1 = 16;
            dword_8026ecd0 = B_LITTLE32(*(int*)byte_8026ecc4);
            byte_8026ecc4 += 2;
        }
        byte_8026ecc1--;
        char v3 = dword_8026ecd0 & 1;
        dword_8026ecd0 >>= 1;
        if (v3)
            v2 |= v1;
        v1 <<= 1;
    }
    return v2;
}

static void sub_800705c8(Huffman *a1, int a2)
{
    for (int i = a2 - 1; i >= 0; i--)
    {
        a1[i].f_0 = 0;
        a1[i].f_2 = -1;
        a1[i].f_8 = 0;
        a1[i].f_c = 0;
    }
}

static uint32_t sub_80070488(uint32_t a, int a2)
{
    int v2 = 0;
    for (int i = a2-1; i >= 0; i--)
    {
        v2 <<= 1;
        if (a & 1)
            v2 |= 1;
        a >>= 1;
    }
    return v2;
}

static void sub_800704dc(Huffman *a1, int a2)
{
    uint32_t v4 = 0;
    uint32_t v5 = 1UL << 31;
    for (int i = 1; i < 17; i++)
    {
        for (int j = 0; j < a2; j++)
        {
            if (a1[j].f_c == i)
            {
                a1[j].f_8 = sub_80070488(v4/v5, i);
                v4 += v5;
            }
        }
        v5 >>= 1;
    }
}

static void sub_80070614(Huffman *a1, int a2)
{
    sub_800705c8(a1, a2);
    int v1 = sub_800702d4(5);
    if (v1 > 16)
        v1 = 16;
    for (int i = 0; i < v1; i++)
    {
        a1[i].f_c = sub_800702d4(4);
    }
    sub_800704dc(a1, a2);
}

static int sub_800703a8(Huffman *a1)
{
    int v6 = 0;
    while (a1->f_c == 0 || a1->f_8 != (((1UL << ((a1->f_c &31)))-1) & dword_8026ecd0))
    {
        a1++;
        v6++;
    }
    sub_800702d4(a1->f_c);
    if (v6 > 1)
    {
        v6--;
        return sub_800702d4(v6) | (1<<((v6)&31));
    }
    return v6;
}

static int RNCDecompress1(const char *inbuf, char *outbuf)
{
    sub_800706b8(inbuf, outbuf);
    sub_800702d4(2);
    while (byte_8026ecc8 < byte_8026eccc)
    {
        sub_80070614(byte_8026ecd8[0], 16);
        sub_80070614(byte_8026ecd8[1], 16);
        sub_80070614(byte_8026ecd8[2], 16);
        int v1 = sub_800702d4(16);
        for (int j = v1-1; j >= 0; j--)
        {
            int v2 = sub_800703a8(byte_8026ecd8[0]);
            for (int i = 0; i < v2; i++)
            {
                *byte_8026ecc8++ = *byte_8026ecc4++;
            }
            dword_8026ecd0 = (dword_8026ecd0&((1<<(byte_8026ecc1&31))-1))
                | ((byte_8026ecc4[0] + (byte_8026ecc4[1]<<8) + (byte_8026ecc4[2]<<16))<<(byte_8026ecc1&31));
            if (j > 0)
            {
                v2 = sub_800703a8(byte_8026ecd8[1]);
                char* v3 = byte_8026ecc8 - (1 + v2);
                v2 = sub_800703a8(byte_8026ecd8[2]);
                v2 += 2;
                for (int i = 0; i < v2; i++)
                {
                    *byte_8026ecc8++ = *v3++;
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
            Bmemcpy(outbuf, inbuf+8, B_BIG32(*(int*)(inbuf+4)));
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
    if (rt_gamestate == 2)
        return _krand();
    return 42;
}

int RT_KRand2(void)
{
    if (rt_gamestatus == 0)
        return _krand();
    return 62007;
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
