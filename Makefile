ESSENTIALSOURCES = main.c libcache.c load.c thread.c interrupt.c \
                   domain.c misc.c zero/zero.c vmsupport.c portal.c malloc.c \
                   gc/gc.c gc/gc_memcpy.c gc/gc_move.c gc/gc_alloc.c gc/gc_checkheap.c\
                   gc/gc_org.c gc/gc_common.c gc/gc_thread.c gc/gc_stack.c gc/gc_move_common.c\
                   gc/gc_new.c gc/gc_impl.c gc/gc_compacting.c gc/gc_bitmap.c gc/gc_chunked.c\
                   classes.c zip.c  execJAVA.c atomic.c \
                   exception_handler.c memfs.c atomicfn.c oneshot.c \
                   sched_global.c sched_local.c sched_local_rr.c sched_local_java.c sched_global_rr.c \
		   				 		 runq.c


SUPPORTSOURCES = profile.c thread_debug.c thread_emulation.c thread_profile.c \
                 monitor.c gc/gc_pa.c gc/gc_pgc.c minilzo.c bdt.c crc32.c ekhz.c

ZEROSOURCES  = zero/zero_AtomicVariable.c
ZEROSOURCES += zero/zero_BootFS.c
ZEROSOURCES += zero/zero_CAS.c
ZEROSOURCES += zero/zero_Clock.c
ZEROSOURCES += zero/zero_ComponentManager.c
ZEROSOURCES += zero/zero_CPU.c
ZEROSOURCES += zero/zero_CPUManager.c
ZEROSOURCES += zero/zero_CPUState.c
ZEROSOURCES += zero/zero_Credential.c
ZEROSOURCES += zero/zero_DebugChannel.c
ZEROSOURCES += zero/zero_DebugSupport.c
ZEROSOURCES += zero/zero_DiskEmulation.c
ZEROSOURCES += zero/zero_Domain.c
ZEROSOURCES += zero/zero_DomainManager.c
ZEROSOURCES += zero/zero_FBEmulation.c
ZEROSOURCES += zero/zero_IRQ.c
#ZEROSOURCES += zero_HLSchedulerSupport.c
#ZEROSOURCES += zero_JAVASchedulerSupport.c
#ZEROSOURCES += zero_LLSchedulerSupport.c
ZEROSOURCES += zero/zero_Memory.c
ZEROSOURCES += zero/zero_MemoryManager.c
ZEROSOURCES += zero/zero_Mutex.c
ZEROSOURCES += zero/zero_Naming.c
ZEROSOURCES += zero/zero_NetEmulation.c
ZEROSOURCES += zero/zero_Ports.c
ZEROSOURCES += zero/zero_Profiler.c
ZEROSOURCES += zero/zero_Scheduler.c
ZEROSOURCES += zero/zero_SMPCPUManager.c
ZEROSOURCES += zero/zero_TestDZperf.c
ZEROSOURCES += zero/zero_TimerEmulation.c
ZEROSOURCES += zero/zero_VMClass.c
ZEROSOURCES += zero/zero_VMMethod.c
ZEROSOURCES += zero/zero_VMObject.c
ZEROSOURCES += zero/zero_object.c
ZEROSOURCES += zero/zero_InterceptorInboundInfo.c

SOURCES = $(ESSENTIALSOURCES) $(ZEROSOURCES) $(SUPPORTSOURCES)

INCLUDES = config.h thread.h load.h zero/zero.h lowlevel.h profile.h \
           gc/gc.h gc/gc_memcpy.h gc/gc_move.h gc/gc_alloc.h gc/gc_pa.h gc/gc_pgc.h \
           gc/gc_org.h gc/gc_org_int.h gc/gc_new.h gc/gc_common.h gc/gc_thread.h \
           gc/gc_stack.h gc/gc_impl.h gc/gc_compacting.h\
           ekhz.h code.h libcache.h

ESSENTIALCORESOURCES =  minic.c multiboot.c irq.c lapic.c io_apic.c smp_detect.c smp_activate.c smp.c timer8254.c cpuid.c
CORESOURCES = $(SOURCES) $(ESSENTIALCORESOURCES) serialdbg.c symfind.c

LINUXSOURCES = $(SOURCES) symfind.c
ASMSOURCES   = lowlevel.S call.S switch.S schedSWITCH.S bench.S zero/zero_FastMemory.S vm_eventLog.S
ASMSOURCES2   = lowlevel.S call.S switch.S schedSWITCH.S bench.S zero_FastMemory.S vm_eventLog.S
COREASMSOURCES  = crt0.S stack.S hwint.S exception.S timer.S
COREASMSOURCES  += smp_startup.S ipiint.S

COREINCLUDE = -Isrc

include settings.makefile
ifeq ($(strip $(CC)), icc)
# -w2 entspricht -Wall
# -X  entspricht -nostdinc
LINUXCCFLAGS = -g -w
CORECCFLAGS  = -g -w -X
else
#LINUXCCFLAGS = -g -Wall -fcall-used-ebx -fcall-used-esi -fcall-used-edi
#CORECCFLAGS = -g -Wall -fcall-used-ebx -fcall-used-esi -fcall-used-edi -nostdinc
LINUXCCFLAGS = -g -m32 -fcall-used-ebx -fcall-used-esi -fcall-used-edi
LINUXCCFLAGS2 = -g
CORECCFLAGS = -g -fcall-used-ebx -fcall-used-esi -fcall-used-edi -nostdinc
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

COREOBJ2 = $(COREOBJ:.kernel/gc/%=.kernel/%)
COREBUILD = ld -m elf_i386 -Ttext 100000 -o jxcore $(COREOBJ2:.kernel/zero/%=.kernel/%)

jxcore: Makefile.dep .kernel src/realmode.h $(COREOBJ)
	$(COREBUILD)
	#perl mksymtab.perl jxcore symbols.h
	rm -f .kernel/symfind.o .kernel/atomicfn.o ; $(MAKE) .kernel/symfind.o .kernel/atomicfn.o
	$(COREBUILD)
#	strip jxcore

.kernel/valid-symbols: src/symbols.h
	$(COREBUILD)
	$(MKSYMTAB) jxcore src/symbols.h
	$(MAKE) .kernel/symfind.o
	touch .kernel/valid-symbols
	$(MAKE) jxcoremake

.kernel/symfind.o: src/symbols.h src/symfind.c

.kernel/atomicfn.o: src/symbols.h src/atomicfn.c src/atomicfn.h

.kernel:
	mkdir .kernel

realmode: src/asm.S
	gcc -m32 -g -c -o asm.o src/asm.S
	ld -m elf_i386 -Ttext 0x9000 -o realmode asm.o
	perl mksymtab.perl realmode src/realmode.h
	touch src/main.c
	$(MAKE) jxcore
	@echo "***** USE BUILD TO UPDATE code.zip"


#.kernel/%.o: %.c
#	$(CC) $(CORECCFLAGS) $(COREDEFINES) $(COREINCLUDE) -S -o .kernel/$<.s $<
#	$(CC) $(CORECCFLAGS) $(COREDEFINES) $(COREINCLUDE) -c -o .kernel/$(@F) $<

.kernel/%.o: src/%.c
	$(CC) -m32 $(CORECCFLAGS) $(COREDEFINES) $(COREINCLUDE) -c -o .kernel/$(@F) $<


#.kernel/%.o: .kernel/%.c
#	$(CC) $(CORECCFLAGS) $(COREDEFINES) $(COREINCLUDE) -I.  -c -o .kernel/$(@F) $<
#.kernel/%.c: %.c
#	gcc -E $(CORECCFLAGS) $(COREDEFINES) $(COREINCLUDE)  -o .kernel/$(@F) $<

.kernel/%.o: src/%.s
	$(AS) --32 $(COREINCLUDE) -c -nostdinc -o .kernel/$(@F) $< || (cp $< src/x.s ; exit 1)

src/%.s: src/%.S
	$(CC) -E $< $(CORECCFLAGS) -DASSEMBLER $(COREDEFINES) $(COREINCLUDE) > src/$(@F)

.kernel/%.s: src/%.c
	$(CC) -S $(CORECCFLAGS) $(COREDEFINES) $(COREINCLUDE) -o .kernel/$(@F) $<

.SECONDARY: $(CORESEC)


# hidden dependencies
src/zero/zero_Memory.c: src/zero/zero_Memory_new.c src/zero/zero_Memory_org.c src/zero/zero_Memory_shared.c src/zero/zero_Memory_simple.c


ifeq ($(findstring jxcore, $(MAKECMDGOALS)), jxcore)
 -include kernel.dep
endif

depend:
	#rm -f kernel.dep linux.dep
	$(MAKE) kernel.dep

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
	rm -rf .kernel *.o *.d thread lock *~ jxcore src/*.s

print:
	rm -rf .print ; mkdir .print
	@for i in *.h $(ESSENTIALSOURCES) *.S ; do  (echo -n "$$i "; $(UNIFDEF)  -UDEBUG -UNEVER -UPROFILE -DKERNEL -DDIRECT_SEND_PORTAL -DNEW_PORTALCALL  -UVERBOSE_SENDER_QUEUE -UCHECK_DEPPARAMS -USAMPLE_FASTPATH -USMP -UMULTI_PORTAL_HACK -UDBG_DEP -UPORTAL_STATISTICS -UPORTAL_INTERCEPTOR -UASSERT_PORTALCALL_NOT_IN_IRQHANDLER -U__SMP -UCHECK_SERIAL_IN_PORTAL -UCONT_PORTAL -UCHECK_HEAPUSAGE -UPORTAL_TRANSFER_INTERCEPTOR -UJAVASCHEDULER -UVERBOSE_PORTAL_PARAM_COPY -UDBG_AUTO_PORTAL_PROMO -DSERVICE_EAGER_CLEANUP -UCOPY_TO_DOMAINZERO -UPROFILE_EVENT_PORTAL -UPROFILE_SAMPLE -UPROFILE_EVENT_JXMALLOC -UPROFILE_AGING -UDEBUG_HANDLE -DGC_USE_NEW -DGC_NEW_IMPL -UGC_COMPACTING_IMPL -DENABLE_GC -UGC_USE_MMX -UCHECKHEAP_VERBOSE -UDBG_GC -UPROFILE_GC -UDBG_STACKMAP -UDBG_SCAN_HEAP2 -UDBG_GCSTATIC -UVERBOSE_GC -UGC_USE_ONLY_ONE -UNOTICE_GC -UMEASURE_GC_TIME -UGC_FAST_MEMCPY -UUSE_MAGIC -UUSE_QMAGIC -UDBG_THREAD -UDBG_IRQ -DUSE_LIB_INDEX -UDBG_LOAD -UEVENT_LOG -UEVENTLOG -UFRAMEBUFFER_EMULATION -UDISK_EMULATION -UNET_EMULATION -UTIMER_EMULATION -UVERBOSE_EXCEPTION -UVERBOSE_EXCEPTION_HANDLER -UUSE_PUSHED_METHODDESC -UMPROTECT_HEAP -UDEBUG_MEMORY_CREATION -UCHECK_HEAP_BEFORE_ALLOC -UCHECK_HEAP_AFTER_ALLOC -UPROFILE_HEAPUSAGE -UUSE_FMAGIC -UMICROBENCHMARKS -UUSE_EKHZ -UCHECK_FREE -UVERBOSE_MALLOC -UVERBOSE_FREE -UPROFILE_EVENT_THREADSWITCH $$i > .print/$$i ; echo ok) ; done
	@echo DONE
	(cd .print ; $(A2PS) --pretty-print=c --columns=1 --rows=1 --landscape --chars-per-line=160 --major=rows -o jxcore.ps *.h $(ESSENTIALSOURCES) *.S)

p2:
		($(UNIFDEF) -UDEBUG -UPROFILE $$i > .print/$$i  ;

.PHONY : clean
# DO NOT DELETE
