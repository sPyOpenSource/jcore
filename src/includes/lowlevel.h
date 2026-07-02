#ifndef LOWLEVEL_H
#define LOWLEVEL_H

#include "thread.h"

#ifndef ASSEMBLER

jint callnative_special(jint * params, ObjectDesc * obj, code_t f,
                        jint params_size);
jint callnative_special_portal(jint * params, ObjectDesc * obj, code_t f,
                               jint params_size);
jint callnative_static(jint * params, code_t f, jint params_size);
void callnative_handler(u4_t * fp, char *addr);

void activate_thread_desc(jint number);
jint get_thread_desc();

void yield(ThreadDesc ** current);
jint destroy_switch_to(ThreadDesc ** current, ThreadDesc * to);
void softint();

static inline u4_t cas(u4_t * var, u4_t old, u4_t new)
{
    u4_t result;
    asm volatile (
        "   ldrex %0, [%1]  \n"
        "   cmp   %0, %2    \n"
        "   strexeq %0, %3, [%1] \n"
        "   moveq %0, #1    \n"
        "   movne %0, #0    \n"
        : "=&r" (result)
        : "r" (var), "r" (old), "r" (new)
        : "cc", "memory"
    );
    return result;
}

static inline int fetchAndStore(int *var, int new)
{
    int result = *var;
    while (!cas((u4_t *) var, (u4_t) result, (u4_t) new))
        result = *var;
    return result;
}

#define getCallerIP() \
    ({ \
        register unsigned int _temp__; \
        asm volatile("mov %0, r11" : "=r" (_temp__)); \
        _temp__; \
    })

#define getCallerCallerIP() \
    ({ \
        register unsigned int _temp__; \
        unsigned int fp; \
        asm volatile("mov %0, r11" : "=r" (fp)); \
        _temp__ = *(unsigned int *)(fp + 4); \
        _temp__; \
    })

#define ALIGN4(x) ((((u4_t)x)+3)&~3)

#endif

#endif
