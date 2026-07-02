#include "all.h"

#define MAX_THREADS 8
#define STACK_SIZE 4096

static ThreadDesc threads[MAX_THREADS];
static int thread_count = 0;
static int current_thread_idx = 0;
static ThreadDesc *current_thread = NULL;

DomainDesc *domainZero = &domainZeroStorage;
static DomainDesc domainZeroStorage;

extern void switch_context(ThreadDesc *old, ThreadDesc *new);

void threads_init() {
    memset(threads, 0, sizeof(threads));
    
    // Create the initial thread (the one we're currently in)
    ThreadDesc *main_thread = &threads[0];
    main_thread->state = STATE_RUNNABLE;
    current_thread = main_thread;
    thread_count = 1;
}

ThreadDesc *createThread(DomainDesc * domain, void (*entry)(void *), void *param, int state, int schedParam) {
    if (thread_count >= MAX_THREADS) return NULL;

    ThreadDesc *t = &threads[thread_count++];
    
    static u4_t stacks[MAX_THREADS][STACK_SIZE / 4];
    u4_t *stack_top = &stacks[thread_count - 1][STACK_SIZE / 4 - 1];

    u4_t *sp = (u4_t *)stack_top;
    *(--sp) = (u4_t)entry;         // PC
    *(--sp) = (u4_t)0;             // LR
    *(--sp) = (u4_t)0;             // R11
    *(--sp) = (u4_t)0;             // R10
    *(--sp) = (u4_t)0;             // R9
    *(--sp) = (u4_t)0;             // R8
    *(--sp) = (u4_t)0;             // R7
    *(--sp) = (u4_t)0;             // R6
    *(--sp) = (u4_t)0;             // R5
    *(--sp) = (u4_t)0;             // R4
    *(--sp) = (u4_t)0;             // R3
    *(--sp) = (u4_t)0;             // R2
    *(--sp) = (u4_t)0;             // R1
    *(--sp) = (u4_t)param;           // R0
    
    t->context[PCB_ESP] = (u4_t)sp;
    t->state = state;
    t->domain = domain;
    
    return t;
}

void threadyield() {
    int next_idx = (current_thread_idx + 1) % thread_count;
    while (threads[next_idx].state != STATE_RUNNABLE) {
        next_idx = (next_idx + 1) % thread_count;
        if (next_idx == current_thread_idx) break;
    }

    if (next_idx != current_thread_idx) {
        ThreadDesc *old = current_thread;
        current_thread_idx = next_idx;
        current_thread = &threads[current_thread_idx];
        switch_context(old, current_thread);
    }
}

void Sched_preempted() {
    threadyield();
}

void setThreadName(ThreadDesc *t, const char *name) {
    (void)t;
    (void)name;
}

void thread_exit() {
    current_thread->state = STATE_ZOMBIE;
    threadyield();
    while (1) {}
}

void init_domainsys() {
    // stub
}
