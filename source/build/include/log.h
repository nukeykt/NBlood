#pragma once
#ifndef log_h__
#define log_h__

#define LOGURU_FILENAME_WIDTH       12
#define LOGURU_THREADNAME_WIDTH     8
#define LOGURU_SCOPE_TIME_PRECISION 6

#include "loguru.hpp"

#include <inttypes.h>

enum loguru_verbosities_engine
{
    LOG_ENGINE = 1,
    LOG_GFX,
    LOG_GL,
    LOG_ASS,
    LOG_INPUT,
    LOG_NET,
    LOG_PR,
    LOG_MEM,
    LOG_ENGINE_MAX,
    LOG_DEBUG = INT8_MAX,
};

#define CB_ENGINE "engine"

extern bool g_useLogCallback;
void engineSetupLogging(int &argc, char **argv);
void engineSetLogFile(const char* fn, loguru::Verbosity verbosity = LOG_ENGINE_MAX, loguru::FileMode mode = loguru::Truncate);
void engineSetLogVerbosityCallback(const char* (*cb)(loguru::Verbosity));

#endif // log_h__
