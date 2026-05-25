
#include <stddef.h>
#include "flint_mutex.h"
#include "flint_system_api.h"

FMutex::FMutex(void) {
    locked = 0;
    lockNest = 0;
    lockThread = NULL;
}

void FMutex::lock(void) {
    void *currentThread = FlintAPI::Thread::getCurrentThread();
    while(1) {
        while(AtomicExchange(&locked, 1))
            FlintAPI::Thread::sleep(1);
        if(lockThread == NULL) {
            lockNest = 1;
            lockThread = currentThread;
            AtomicSet(&locked, 0);
            return;
        }
        else if(lockThread == currentThread) {
            lockNest++;
            AtomicSet(&locked, 0);
            return;
        }
        AtomicSet(&locked, 0);
        FlintAPI::Thread::sleep(1);
    }
}

void FMutex::unlock(void) {
    while(AtomicExchange(&locked, 1))
        FlintAPI::Thread::sleep(1);
    if(lockNest > 0) {
        if(--lockNest == 0)
            lockThread = NULL;
    }
    AtomicSet(&locked, 0);
}
