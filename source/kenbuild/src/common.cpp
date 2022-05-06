
#include "compat.h"
#include "build.h"

#include "names.h"
#include "common_game.h"

static const char *defaultgrpfilename = "STUFF.DAT";
static const char *defaultdeffilename = "kenbuild.def";

int g_useCwd;

const char *G_DefaultGrpFile(void)
{
    return defaultgrpfilename;
}
const char *G_GrpFile(void)
{
    return defaultgrpfilename;
}

const char *G_DefaultDefFile(void)
{
    return defaultdeffilename;
}
const char *G_DefFile(void)
{
    return defaultdeffilename;
}

static void Ken_InitMultiPsky()
{
    // default
    psky_t * const defaultsky = tileSetupSky(DEFAULTPSKY);
    defaultsky->lognumtiles = 1;
    defaultsky->horizfrac = 65536;

    // DAYSKY
    psky_t * const daysky = tileSetupSky(DAYSKY);
    daysky->lognumtiles = 1;
    daysky->horizfrac = 65536;

    // NIGHTSKY
    psky_t * const nightsky = tileSetupSky(NIGHTSKY);
    nightsky->lognumtiles = 3;
    nightsky->horizfrac = 65536;
}

void Ken_PostStartupWindow()
{
    Ken_InitMultiPsky();

    size_t i;
    char tempbuf[256];

    for (i=0; i<256; i++) tempbuf[i] = i;

    for (i=0; i<32; i++) tempbuf[i+192] = i+128; //green->red
    paletteMakeLookupTable(1,tempbuf,0,0,0,1);
    for (i=0; i<32; i++) tempbuf[i+192] = i+32; //green->blue
    paletteMakeLookupTable(2,tempbuf,0,0,0,1);
    for (i=0; i<32; i++) tempbuf[i+192] = i+224; //green->pink
    paletteMakeLookupTable(3,tempbuf,0,0,0,1);
    for (i=0; i<32; i++) tempbuf[i+192] = i+64; //green->brown
    paletteMakeLookupTable(4,tempbuf,0,0,0,1);
    for (i=0; i<32; i++) tempbuf[i+192] = i+96;
    paletteMakeLookupTable(5,tempbuf,0,0,0,1);
    for (i=0; i<32; i++) tempbuf[i+192] = i+160;
    paletteMakeLookupTable(6,tempbuf,0,0,0,1);
    for (i=0; i<32; i++) tempbuf[i+192] = i+192;
    paletteMakeLookupTable(7,tempbuf,0,0,0,1);

    for (i=0; i<256; i++)
        tempbuf[i] = ((i+32)&255);  //remap colors for screwy palette sectors
    paletteMakeLookupTable(16,tempbuf,0,0,0,1);

    for (i=0; i<256; i++) tempbuf[i] = i;
    paletteMakeLookupTable(17,tempbuf,96,96,96,1);

    for (i=0; i<256; i++) tempbuf[i] = i; //(i&31)+32;
    paletteMakeLookupTable(18,tempbuf,32,32,192,1);
}

int32_t voxid_PLAYER = -1, voxid_BROWNMONSTER = -1;

void Ken_LoadVoxels()
{
    if (!qloadkvx(nextvoxid,"voxel000.kvx"))
        voxid_PLAYER = nextvoxid++;
    if (!qloadkvx(nextvoxid,"voxel001.kvx"))
        voxid_BROWNMONSTER = nextvoxid++;
}
