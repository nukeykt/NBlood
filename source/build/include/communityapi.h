#ifndef COMMUNITYAPI_H_
#define COMMUNITYAPI_H_

#include "compat.h"

#ifdef __cplusplus
extern "C" {
#endif

void communityapiInit(void);
void communityapiShutdown(void);
void communityapiRunCallbacks(void);

void communityapiUnlockAchievement(char const * id);
void communityapiSetStat(char const * id, int32_t value);

#ifdef VWSCREENSHOT
void communityapiSendScreenshot(char * filename);
#endif

#ifdef __cplusplus
}
#endif

#endif
