#pragma once
#include <csignal>

#if !defined(NDEBUG)
#define ASSERT(x) if(!(x)){raise(SIGTRAP);}
#define ASSERT_ZERO(x) if(x){raise(SIGTRAP);}
#else
#define ASSERT(x) 
#define ASSERT_ZERO(x)
#endif