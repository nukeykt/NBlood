#ifndef COMMUNITYAPI_H_
#define COMMUNITYAPI_H_

#include "compat.h"

#ifdef __cplusplus
extern "C" {
#endif

void communityapiInit(void);
void communityapiShutdown(void);
void communityapiRunCallbacks(void);

#ifdef VWSCREENSHOT
void communityapiSendScreenshot(char * filename);
#endif

#ifdef __cplusplus
}
#endif

#endif
