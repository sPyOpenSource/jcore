#pragma once

#include "peripherals/base.h"
#include "common.h"

#define CLOCKHZ 1000000

//10.2
struct timer_regs {
    reg32 control_status;
    reg32 counter_lo;
    reg32 counter_hi;
    reg32 compare[4];
};

#define REGS_TIMER ((struct timer_regs *)(PBASE + 0x00003000))

void timer_init();
void handle_timer_1();
void handle_timer_3();
void timer_sleep(u32 ms);
u64 timer_get_ticks();
