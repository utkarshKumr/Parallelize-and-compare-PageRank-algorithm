
#include <assert.h>

#undef REQUIRES_CHECK

#ifdef DEBUG

#define REQUIRES_CHECK(COND) assert(COND)

#else

#define REQUIRES_CHECK(COND) ((void)0)

#endif
