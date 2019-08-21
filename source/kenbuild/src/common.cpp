
#include "compat.h"
#include "build.h"

#include "names.h"
#include "common_game.h"

static const char *defaultgrpfilename = "stuff.dat";
static const char *defaultdeffilename = "kenbuild.def";

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

void Ken_InitMultiPsky(void)
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
