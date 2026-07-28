#pragma once

#include "Version.h"

//=============================================================================
// Build Information
//=============================================================================

#ifndef BUILD_TYPE
#define BUILD_TYPE "Integration"
#endif

#ifndef BUILD_BRANCH
#define BUILD_BRANCH "local"
#endif

#ifndef BUILD_COMMIT
#define BUILD_COMMIT "unknown"
#endif

#ifndef BUILD_NUMBER
#define BUILD_NUMBER "0"
#endif

//=============================================================================
// Compiler
//=============================================================================

#define BUILD_DATE        __DATE__
#define BUILD_TIME        __TIME__
#define BUILD_DATETIME    __DATE__ " " __TIME__

//=============================================================================
// Platform
//=============================================================================

#define BUILD_PLATFORM    ARDUINO_BOARD
#define BUILD_FRAMEWORK   "Arduino"
#define BUILD_ARCH        "ESP8266"

//=============================================================================
// Helper
//=============================================================================

namespace BuildInfo
{

inline const char* version()
{
    return FW_VERSION_STRING;
}

inline const char* fullVersion()
{
    return FW_VERSION_FULL;
}

inline const char* label()
{
    return FW_VERSION_LABEL;
}

inline const char* firmware()
{
    return FW_NAME;
}

inline const char* targetVersion()
{
    return FW_TARGET_VERSION;
}

inline const char* buildDate()
{
    return BUILD_DATE;
}

inline const char* buildTime()
{
    return BUILD_TIME;
}

inline const char* buildDateTime()
{
    return BUILD_DATETIME;
}

inline const char* buildType()
{
    return BUILD_TYPE;
}

inline const char* buildBranch()
{
    return BUILD_BRANCH;
}

inline const char* buildCommit()
{
    return BUILD_COMMIT;
}

inline const char* buildNumber()
{
    return BUILD_NUMBER;
}

}

