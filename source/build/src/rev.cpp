// This file is recompiled unconditionally so the timestamp gets updated, even for a partial recompile.
#include "rev.h"

#ifdef __cplusplus
extern "C" {
#endif
const char* s_buildRev = REVSTR;
const char* s_buildTimestamp = __DATE__ " " __TIME__;
#ifdef __cplusplus
}
#endif
