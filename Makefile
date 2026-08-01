ESSENTIALSOURCES = core/main.c core/libcache.c core/load.c core/thread.c core/interrupt.c \
                   core/domain.c core/misc.c Interface/zero.c core/vmsupport.c core/portal.c core/malloc.c \
				   core/classes.c core/zip.c core/execJAVA.c core/atomic.c \
                   core/exception_handler.c core/memfs.c core/atomicfn.c core/oneshot.c \
                   core/sched_global.c core/sched_local.c core/sched_local_rr.c core/sched_local_java.c core/sched_global_rr.c \
		   		   core/runq.c core/syscalls.c

ESSENTIALSOURCES += Memory/gc.c Memory/gc_memcpy.c Memory/gc_move.c Memory/gc_alloc.c Memory/gc_checkheap.c \
                    Memory/gc_org.c Memory/gc_common.c Memory/gc_thread.c Memory/gc_stack.c Memory/gc_move_common.c \
                    Memory/gc_new.c Memory/gc_impl.c Memory/gc_compacting.c Memory/gc_bitmap.c Memory/gc_chunked.c

SUPPORTSOURCES = debug/profile.c debug/thread_debug.c debug/thread_emulation.c debug/thread_profile.c \
                 debug/monitor.c Memory/gc_pa.c Memory/gc_pgc.c core/minilzo.c core/bdt.c core/crc32.c core/ekhz.c

ZEROSOURCES  = Interface/zero_AtomicVariable.c Interface/zero_BootFS.c
ZEROSOURCES += Interface/zero_CAS.c Interface/zero_Clock.c
ZEROSOURCES += Interface/zero_ComponentManager.c Interface/zero_CPU.c
ZEROSOURCES += Interface/zero_CPUManager.c Interface/zero_CPUState.c
ZEROSOURCES += Interface/zero_Credential.c Interface/zero_DebugChannel.c
ZEROSOURCES += Interface/zero_DebugSupport.c Interface/zero_DiskEmulation.c
ZEROSOURCES += Interface/zero_Domain.c Interface/zero_DomainManager.c
ZEROSOURCES += Interface/zero_FBEmulation.c Interface/zero_IRQ.c
#ZEROSOURCES += NativeJavaInterface/zero_HLSchedulerSupport.c
#ZEROSOURCES += NativeJavaInterface/zero_JAVASchedulerSupport.c
#ZEROSOURCES += NativeJavaInterface/zero_LLSchedulerSupport.c
ZEROSOURCES += Interface/zero_Memory.c Interface/zero_MemoryManager.c
ZEROSOURCES += Interface/zero_Mutex.c  Interface/zero_Naming.c
ZEROSOURCES += Interface/zero_NetEmulation.c Interface/zero_Ports.c
ZEROSOURCES += Interface/zero_Profiler.c Interface/zero_Scheduler.c
ZEROSOURCES += Interface/zero_SMPCPUManager.c Interface/zero_TestDZperf.c
ZEROSOURCES += Interface/zero_TimerEmulation.c Interface/zero_VMClass.c
ZEROSOURCES += Interface/zero_VMMethod.c Interface/zero_VMObject.c
ZEROSOURCES += Interface/zero_object.c Interface/zero_InterceptorInboundInfo.c

SOURCES = $(ESSENTIALSOURCES) $(ZEROSOURCES) $(SUPPORTSOURCES)

INCLUDES = Headers/config.h Headers/core/thread.h Headers/core/load.h Interface/zero.h Headers/arch/lowlevel.h Headers/debug/profile.h \
           Memory/gc.h Memory/gc_memcpy.h Memory/gc_move.h Memory/gc_alloc.h Memory/gc_pa.h Memory/gc_pgc.h \
           Memory/gc_org.h Memory/gc_org_int.h Memory/gc_new.h Memory/gc_common.h Memory/gc_thread.h \
           Memory/gc_stack.h Memory/gc_impl.h Memory/gc_compacting.h \
           Headers/core/ekhz.h Headers/core/code.h Headers/core/libcache.h

ESSENTIALCORESOURCES =  arch/minic.c arch/multiboot.c arch/irq.c arch/lapic.c arch/io_apic.c arch/smp_detect.c arch/smp_activate.c arch/smp.c arch/timer8254.c arch/cpuid.c
CORESOURCES = $(SOURCES) $(ESSENTIALCORESOURCES) debug/serialdbg.c debug/symfind.c

LINUXSOURCES = $(SOURCES) symfind.c
ASMSOURCES   = Assembly/lowlevel.S Assembly/call.S Assembly/switch.S Assembly/schedSWITCH.S \
			   Assembly/bench.S Assembly/vm_eventLog.S Interface/zero_FastMemory.S
COREASMSOURCES = Assembly/crt0.S Assembly/stack.S Assembly/hwint.S Assembly/exception.S Assembly/timer.S \
				 Assembly/smp_startup.S Assembly/ipiint.S #Assembly/main64.S

COREINCLUDE = -Isrc -Isrc/Headers

include settings.makefile
ifeq ($(strip $(CC)), icc)
# -w2 entspricht -Wall
# -X  entspricht -nostdinc
CORECCFLAGS  = -g -w -X
else
#CORECCFLAGS = -g -Wall -fcall-used-ebx -fcall-used-esi -fcall-used-edi -nostdinc
 CORECCFLAGS = -g -nostdinc -m32 -target i386-pc-linux-gnu -std=gnu89 -fno-builtin -fno-stack-protector -Wno-error=return-type -mno-sse -mno-sse2 -mno-mmx -msoft-float
endif

ifeq ($(shell uname -s),Darwin)
AS = $(CC)
LD = ld.lld
CORECCFLAGS += -Qunused-arguments -Wno-unused-command-line-argument
CORECCFLAGS += -Wno-visibility -Wno-incompatible-pointer-types -Wno-int-conversion
CORECCFLAGS += -Wno-extra-tokens -Wno-incompatible-function-pointer-types
CORECCFLAGS += -Wno-pointer-sign -Wno-return-type -Wno-compare-distinct-pointer-types
CORECCFLAGS += -Wno-pointer-integer-compare -Wno-parentheses
COREASFLAGS = $(CORECCFLAGS)
CORELDOPTS = -m elf_i386
else
COREASFLAGS =
CORELDOPTS =
endif


COREDEFINES += -DKERNEL
COREDEFINES += $(DEFINES)
COREINCLUDES = $(INCLUDES)

LINUXDEFINES += $(DEFINES)
LINUXINCLUDES = $(INCLUDES)
LINUXINCLUDE = -I.

LINUXLDOPTS = -g -Wall
LINUXLDOPTS += -L/usr/X11R6/lib/ -lX11 -lXext


#########  RULES ########

CORESRC  = $(CORESOURCES)
# asm objects must come first, because they contain the entry code

COREOBJ  = $(COREASMSOURCES:%.S=.kernel/%.o)
COREOBJ += $(ASMSOURCES:%.S=.kernel/%.o)
COREOBJ += $(CORESRC:%.c=.kernel/%.o)

CORESEC  = $(ASMSOURCES:%.S=%.s)
CORESEC += $(COREASMSOURCES:%.S=%.s)

LINUXSRC  = $(LINUXSOURCES)
LINUXOBJ  = $(LINUXSRC:%.c=.linux/%.o)
LINUXOBJ += $(ASMSOURCES:%.S=.linux/%.o)

COREOBJ2 = $(COREOBJ:.kernel/Memory/%=.kernel/%)
COREOBJ3 = $(COREOBJ2:.kernel/Assembly/%=.kernel/%)
COREOBJ4 = $(COREOBJ3:.kernel/Interface/%=.kernel/%)
COREOBJ5 = $(COREOBJ4:.kernel/core/%=.kernel/%)
COREOBJ6 = $(COREOBJ5:.kernel/arch/%=.kernel/%)
COREOBJ7 = $(COREOBJ6:.kernel/debug/%=.kernel/%)
COREBUILD = $(LD) $(CORELDOPTS) -T linker.ld -o jxcore $(COREOBJ7)

jxcore: .kernel src/Headers/realmode.h $(COREOBJ)
	$(COREBUILD)
	#perl mksymtab.perl jxcore symbols.h
	rm -f .kernel/symfind.o .kernel/atomicfn.o ; $(MAKE) .kernel/debug/symfind.o .kernel/core/atomicfn.o
	$(COREBUILD)
#	strip jxcore

.kernel/valid-symbols: src/Headers/symbols.h
	$(COREBUILD)
	$(MKSYMTAB) jxcore src/Headers/symbols.h
	$(MAKE) .kernel/debug/symfind.o
	touch .kernel/valid-symbols
	$(MAKE) jxcoremake

.kernel/debug/symfind.o: src/Headers/symbols.h src/debug/symfind.c

.kernel/core/atomicfn.o: src/Headers/symbols.h src/core/atomicfn.c src/Headers/core/atomicfn.h

.kernel:
	mkdir .kernel

realmode: src/Assembly/asm.S
	$(CC) -g -c -o asm.o src/Assembly/asm.S
	$(LD) -Ttext 0x9000 -o realmode asm.o
	perl mksymtab.perl realmode src/Headers/realmode.h
	touch src/main.c
	$(MAKE) jxcore
	@echo "***** USE BUILD TO UPDATE code.zip"

#.kernel/%.o: %.c
#	$(CC) $(CORECCFLAGS) $(COREDEFINES) $(COREINCLUDE) -S -o .kernel/$<.s $<
#	$(CC) $(CORECCFLAGS) $(COREDEFINES) $(COREINCLUDE) -c -o .kernel/$(@F) $<

.kernel/%.o: src/%.c
	$(CC) $(CORECCFLAGS) $(COREDEFINES) $(COREINCLUDE) -c -o .kernel/$(@F) $<

#.kernel/%.o: .kernel/%.c
#	$(CC) $(CORECCFLAGS) $(COREDEFINES) $(COREINCLUDE) -I.  -c -o .kernel/$(@F) $<
#.kernel/%.c: %.c
#	gcc -E $(CORECCFLAGS) $(COREDEFINES) $(COREINCLUDE)  -o .kernel/$(@F) $<

.kernel/%.o: src/%.asm
	$(AS) $(COREASFLAGS) $(COREINCLUDE) -c -nostdinc -o .kernel/$(@F) $<

src/Interface/zero_FastMemory.asm: src/Interface/zero_FastMemory.S
	$(CC) -E $< $(CORECCFLAGS) -DASSEMBLER $(COREDEFINES) $(COREINCLUDE) > src/Interface/$(@F)

src/%.asm: src/%.S
	$(CC) -E $< $(CORECCFLAGS) -DASSEMBLER $(COREDEFINES) $(COREINCLUDE) > src/Assembly/$(@F)

#.kernel/%.asm: src/%.c
#	$(CC) -S $(CORECCFLAGS) $(COREDEFINES) $(COREINCLUDE) -o .kernel/$(@F) $<

.SECONDARY: $(CORESEC)


# hidden dependencies
#src/Interface/zero_Memory.c: src/Interface/zero_Memory_new.c src/Interface/zero_Memory_org.c src/Interface/zero_Memory_shared.c src/Interface/zero_Memory_simple.c

ifeq ($(findstring jxcore, $(MAKECMDGOALS)), jxcore)
 #-include kernel.dep
endif

depend:
	#rm -f kernel.dep linux.dep
	#$(MAKE) kernel.dep

kernel.dep:
	touch $@
	./makedepend -- $(CORECCFLAGS) $(COREDEFINES) $(COREINCLUDE) -- -f $@ -p .kernel/ $(CORESRC)

# cleans all files if the Makefile was modified
Makefile.dep: Makefile settings.makefile
	$(MAKE) clean
	touch $@
	$(MAKE) depend

wc:
	wc -l $(ESSENTIALSOURCES) $(ZEROSOURCES) $(ESSENTIALCORESOURCES) | sort -n

clean:
	rm -rf .kernel *.o *.d thread lock *~ jxcore src/Assembly/*.s src/zero/*.s

print:
	rm -rf .print ; mkdir .print
	@for i in *.h $(ESSENTIALSOURCES) *.S ; do  (echo -n "$$i "; $(UNIFDEF)  -UDEBUG -UNEVER -UPROFILE -DKERNEL -DDIRECT_SEND_PORTAL -DNEW_PORTALCALL  -UVERBOSE_SENDER_QUEUE -UCHECK_DEPPARAMS -USAMPLE_FASTPATH -USMP -UMULTI_PORTAL_HACK -UDBG_DEP -UPORTAL_STATISTICS -UPORTAL_INTERCEPTOR -UASSERT_PORTALCALL_NOT_IN_IRQHANDLER -U__SMP -UCHECK_SERIAL_IN_PORTAL -UCONT_PORTAL -UCHECK_HEAPUSAGE -UPORTAL_TRANSFER_INTERCEPTOR -UJAVASCHEDULER -UVERBOSE_PORTAL_PARAM_COPY -UDBG_AUTO_PORTAL_PROMO -DSERVICE_EAGER_CLEANUP -UCOPY_TO_DOMAINZERO -UPROFILE_EVENT_PORTAL -UPROFILE_SAMPLE -UPROFILE_EVENT_JXMALLOC -UPROFILE_AGING -UDEBUG_HANDLE -DGC_USE_NEW -DGC_NEW_IMPL -UGC_COMPACTING_IMPL -DENABLE_GC -UGC_USE_MMX -UCHECKHEAP_VERBOSE -UDBG_GC -UPROFILE_GC -UDBG_STACKMAP -UDBG_SCAN_HEAP2 -UDBG_GCSTATIC -UVERBOSE_GC -UGC_USE_ONLY_ONE -UNOTICE_GC -UMEASURE_GC_TIME -UGC_FAST_MEMCPY -UUSE_MAGIC -UUSE_QMAGIC -UDBG_THREAD -UDBG_IRQ -DUSE_LIB_INDEX -UDBG_LOAD -UEVENT_LOG -UEVENTLOG -UFRAMEBUFFER_EMULATION -UDISK_EMULATION -UNET_EMULATION -UTIMER_EMULATION -UVERBOSE_EXCEPTION -UVERBOSE_EXCEPTION_HANDLER -UUSE_PUSHED_METHODDESC -UMPROTECT_HEAP -UDEBUG_MEMORY_CREATION -UCHECK_HEAP_BEFORE_ALLOC -UCHECK_HEAP_AFTER_ALLOC -UPROFILE_HEAPUSAGE -UUSE_FMAGIC -UMICROBENCHMARKS -UUSE_EKHZ -UCHECK_FREE -UVERBOSE_MALLOC -UVERBOSE_FREE -UPROFILE_EVENT_THREADSWITCH $$i > .print/$$i ; echo ok) ; done
	@echo DONE
	(cd .print ; $(A2PS) --pretty-print=c --columns=1 --rows=1 --landscape --chars-per-line=160 --major=rows -o jxcore.ps *.h $(ESSENTIALSOURCES) *.S)

p2:
		($(UNIFDEF) -UDEBUG -UPROFILE $$i > .print/$$i  ;

.PHONY : clean
# DO NOT DELETE
