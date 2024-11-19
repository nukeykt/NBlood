#pragma once

#ifndef xxhash_config_h__
#define xxhash_config_h__

#ifndef NDEBUG
# define XXH_NO_INLINE_HINTS 1
#endif

#define XXH_STATIC_LINKING_ONLY 1

#include "xxhash.h"
#endif // xxhash_config_h__
