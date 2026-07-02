#ifndef _circle_atomic_h
#define _circle_atomic_h

#ifdef __cplusplus
extern "C" {
#endif

static inline int AtomicExchange(volatile int *ptr, int val) {
    return __atomic_exchange_n(ptr, val, __ATOMIC_SEQ_CST);
}

static inline void AtomicSet(volatile int *ptr, int val) {
    __atomic_store_n(ptr, val, __ATOMIC_SEQ_CST);
}

#ifdef __cplusplus
}
#endif

#endif
