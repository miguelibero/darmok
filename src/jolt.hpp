#pragma once

#ifndef NDEBUG
// these seem to be missing in the library header in debug
#define JPH_PROFILE_ENABLED
#define JPH_DEBUG_RENDERER
#endif

#include <Jolt/Jolt.h>