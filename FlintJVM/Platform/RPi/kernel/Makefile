
CIRCLEHOME = ../../../../..

OBJS	= main.o main_java.o kernel.o \
		  ../Src/flint_system_api.o ../Src/flint_system_io_api.o ../Src/flint_system_thread_api.o

LIBS	= ../../../Native/libnative.a ../../../libwvm.a $(CIRCLEHOME)/lib/peripherals/libperi.a $(CIRCLEHOME)/lib/libcircle.a

INCLUDE	+= -I $(CIRCLEHOME)/include -I $(CIRCLEHOME)/addon -I ../Inc -I ../../../Native/Inc -I ../../../Inc -DKERNEL_MAX_SIZE=0x400000

STDLIB_SUPPORT = 1

include $(CIRCLEHOME)/Rules.mk

-include $(DEPS)
