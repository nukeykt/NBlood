#pragma once

#ifndef libdivide_config_h__
#define libdivide_config_h__

#if defined(__x86_64__) || defined(_M_X64) || defined __SSE2__ || (defined _M_IX86_FP && _M_IX86_FP == 2)
#define LIBDIVIDE_SSE2 1
#endif
#include "libdivide.h"
#endif // libdivide_config_h__
