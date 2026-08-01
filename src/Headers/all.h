#ifndef ALL_H
#define ALL_H

#include "config.h"

#ifndef KERNEL
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
//#include <asm/sigcontext.h>
#include <termios.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#else
#include "arch/minic.h"
#endif

#include "core/malloc.h"
#include "core/load.h"
#include "arch/context.h"
#include "arch/lowlevel.h"
#include "core/thread.h"
#include "core/portal.h"
#include "core/domain.h"
#include "config.h"
#include "core/interrupt.h"
#include "Memory/gc.h"
#include "core/vmsupport.h"
#include "core/execJAVA.h"
#include "debug/monitor.h"
#include "Interface/zero.h"
#include "Interface/zero_Profiler.h"
#include "core/malloc_proto.h"
#include "core/portal_proto.h"
#include "core/atomic.h"
#include "core/exception_handler.h"

#include "core/thread.h"
#include "misc.h"
#include "debug/monitor.h"
#include "arch/lapic.h"
#include "arch/smp.h"
#include "debug/serialdbg.h"
#include "core/spinlock.h"
#include "debug/symfind.h"
#include "core/interrupt.h"

#include "arch/irq.h"
#include "arch/smp.h"
#include "Interface/zero.h"

#ifdef JAVASCHEDULER
#include "core/javascheduler.h"
#include "core/execJAVA.h"
#endif

//#include "mutex.h"
#include "core/libcache.h"
#include "core/memfs.h"
#include "arch/multiboot.h"
#include "misc.h"
#include "core/memfs.h"
#include "core/zip.h"
#include "types.h"
#include "arch/intr.h"
#include "debug/debug_reg.h"

#include "core/object.h"
#include "debug/bench.h"
#include "core/ekhz.h"
#include "core/sched.h"


#ifndef INATOMICFN
#define ATOMICFN(_r_, _n_, _s_) extern _r_ (* _n_) _s_;
#define ATOMICFN0(_r_, _n_, _s_) extern _r_ (* _n_) _s_;
#include "core/atomicfn.h"
#endif

#endif				/* ALL_H */
