#include "build.h"
#include "pragmas.h"
#include "common_game.h"
#include "resource.h"
#include "trig.h"

long costable[2048];

int OctantTable[8] = { 5, 6, 2, 1, 4, 7, 3, 0 };

int GetOctant(int x, int y)
{
    int vc = klabs(x)-klabs(y);
    return OctantTable[7-(x<0)-(y<0)*2-(vc<0)*4];
}

void RotateVector(long *dx, long *dy, int nAngle)
{
    int ox = *dx;
    int oy = *dy;
    *dx = dmulscale30r(ox, Cos(nAngle), -oy, Sin(nAngle));
    *dy = dmulscale30r(ox, Sin(nAngle), oy, Cos(nAngle));
}

void RotatePoint(long *x, long *y, int nAngle, int ox, int oy)
{
    long dx = *x-ox;
    long dy = *y-oy;
    *x = ox+dmulscale30r(dx, Cos(nAngle), -dy, Sin(nAngle));
    *y = oy+dmulscale30r(dx, Sin(nAngle), dy, Cos(nAngle));
}

void trigInit(Resource &Res)
{
    DICTNODE *pTable = Res.Lookup("cosine","dat");
    if (!pTable)
        ThrowError("Cosine table not found");
    if (pTable->size != 2048)
        ThrowError("Cosine table incorrect size");
    memcpy(costable, Res.Load(pTable), pTable->size);
    costable[512] = 0;
    for (int i = 513; i <= 1024; i++)
    {
        costable[i] = -costable[1024-i];
    }
    for (int i = 1025; i < 2048; i++)
    {
        costable[i] = costable[2048 - i];
    }
}
