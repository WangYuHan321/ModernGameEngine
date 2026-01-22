#pragma once

#if defined (DEBUG) || defined(_DEBUG)
#   define FG_DEBUG
#else
#   define FG_RELEASE
#endif


#define FG_FAST_HASH 0