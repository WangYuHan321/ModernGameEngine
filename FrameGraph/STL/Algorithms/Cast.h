#pragma once

#include "./Config.h"

#ifdef  COMPILER_MSVC
#define and &&
#define or  ||
#define not !
#endif //  COMPILER_MSVC

#ifndef null
#define null nullptr
#endif // !null

#ifndef forceinline

#define forceinline inline

#endif


#ifdef COMPILER_MSVC


#endif








