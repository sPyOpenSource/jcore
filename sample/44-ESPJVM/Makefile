#
# Makefile
#

CIRCLEHOME = ../..

OBJS	= main.o kernel.o

LIBS	= $(CIRCLEHOME)/lib/libcircle.a $(CIRCLEHOME)/lib/fonts/libfont.a $(CIRCLEHOME)/lib/rpi/libbcm.a $(CIRCLEHOME)/lib/peripherals/libperi.a \
	  $(CIRCLEHOME)/addon/FlintJVM/libwvm.a

EXTRAINCLUDE = -I$(CIRCLEHOME)/addon/FlintJVM/Inc -I$(CIRCLEHOME)/addon/FlintJVM/Native/Inc

include ../Rules.mk

-include $(DEPS)
