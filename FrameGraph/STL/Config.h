#pragma once

#if defined (DEBUG) || defined(_DEBUG)
#   define FG_DEBUG
#else
#   define FG_RELEASE
#endif

#ifdef FG_DEBUG
#define FG_ENABLE_DATA_RACE_CHECK
#else
#endif


#define FG_FAST_HASH 0