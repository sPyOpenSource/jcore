
#ifndef __FLINT_MUTEX_H
#define __FLINT_MUTEX_H

#include <stddef.h>
#include <circle/atomic.h>
#include "flint_std.h"

class FMutex {
private:
    volatile int locked;
    uint32_t lockNest;
    volatile void *lockThread;

    FMutex(const FMutex &) = delete;
    void operator=(const FMutex &) = delete;
public:
    FMutex(void);

    void lock(void);
    void unlock(void);
};

#endif /* __FLINT_MUTEX_H */
