#ifndef ATOMIC_H
#define ATOMIC_H

void nopreempt_init(void);
code_t nopreempt_register(code_t original, u4_t size);

void atomicfn_init(void);

#endif				/* ATOMIC_H */
