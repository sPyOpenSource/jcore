#ifndef ALL_H
#define ALL_H

#include "config.h"
#include "ErrorCodes.h"
#include "lib/efi.h"
#include "lib/efilibs.h"

#ifndef KERNEL
//#include <stdio.h>
//#include <signal.h>
//#include <unistd.h>
//#include <sys/time.h>
//#include <stdlib.h>
//#include <termios.h>
//#include <sys/mman.h>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <fcntl.h>
#include <stdarg.h>
//#include <string.h>
#else
#include "minic.h"
#endif

#include "interface/execJAVA.h"
#include "interface/portal.h"
#include "interface/zero_Profiler.h"
#include "interface/portal_proto.h"
#include "interface/exception_handler.h"
#include "interface/zero.h"

#include "mem/malloc.h"
#include "mem/gc.h"
#include "mem/malloc_proto.h"

#include "load.h"
#include "context.h"
#include "lowlevel.h"
#include "thread.h"
#include "domain.h"
#include "config.h"
#include "interrupt.h"
#include "vmsupport.h"
#include "monitor.h"
#include "atomic.h"

#include "thread.h"
#include "monitor.h"
#include "lapic.h"
#include "smp.h"
//#include "serialdbg.h"
#include "spinlock.h"
#include "symfind.h"
#include "interrupt.h"

#include "irq.h"
#include "smp.h"
#include "lib/misc.h"

#ifdef JAVASCHEDULER
#include "javascheduler.h"
#include "execJAVA.h"
#endif

#include "libcache.h"
#include "memfs.h"
//#include "multiboot.h"
#include "lib/misc.h"
#include "lib/zip.h"
#include "lib/types.h"
#include "lib/printf.h"

#include "intr.h"
#include "debug_reg.h"

#include "interface/object.h"
#include "bench.h"
#include "ekhz.h"
#include "sched.h"


#ifndef INATOMICFN
#define ATOMICFN(_r_, _n_, _s_) extern _r_ (* _n_) _s_;
#define ATOMICFN0(_r_, _n_, _s_) extern _r_ (* _n_) _s_;
#include "atomicfn.h"
#endif

#endif				/* ALL_H */
