#pragma once

#ifndef libasync_config_h__
#define libasync_config_h__

#define LIBASYNC_STATIC
#define LIBASYNC_NO_EXCEPTIONS

#define LIBASYNC_ALIGNED_ALLOC(size, align) Xaligned_alloc(size, align)
#define LIBASYNC_ALIGNED_FREE(ptr)          Xaligned_free(ptr)

#include "libasync.h"

#endif  // libasync_config_h__
