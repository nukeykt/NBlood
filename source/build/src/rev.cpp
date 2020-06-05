// This file is recompiled unconditionally so the timestamp gets updated, even for a partial recompile.

#ifdef _WIN32
# include "windows_inc.h"
#else
# if defined REV
#  define REV__(x) #x
#  define REV_(x) REV__(x)
#  define REVSTR REV_(REV)
# else
#  define REVSTR "r(?)"
#  define EDUKE32_UNKNOWN_REVISION
# endif
#endif

#ifdef __cplusplus
extern "C" {
#endif
const char* s_buildRev = REVSTR;
const char* s_buildTimestamp = __DATE__ " " __TIME__;
#ifdef __cplusplus
}
#endif
