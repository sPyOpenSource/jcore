
#include "flint_system_api.h"

using namespace FlintAPI::Thread;

ThreadHandle FlintAPI::Thread::create(void (*task)(void *), void *param, uint32_t stackSize) {
    (void)task; (void)param; (void)stackSize;
    return nullptr;
}

ThreadHandle FlintAPI::Thread::getCurrentThread(void) {
    return nullptr;
}

void FlintAPI::Thread::terminate(ThreadHandle handle) {
    (void)handle;
}

void FlintAPI::Thread::sleep(uint32_t ms) {
    (void)ms;
}

void FlintAPI::Thread::yield(void) {
}
