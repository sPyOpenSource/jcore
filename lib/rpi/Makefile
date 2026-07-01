CIRCLEHOME = ../..

all: libbcm.a

include $(CIRCLEHOME)/Rules.mk

OBJS	= bcmframebuffer.o bcmmailbox.o \
	  bcmpropertytags.o bcmwatchdog.o

ifneq ($(filter 1 2 3,$(RASPPI)),)
OBJS	+= bcmrandom.o
else
OBJS	+= bcmpciehostbridge.o bcmrandom200.o
endif

ifeq ($(strip $(RASPPI)),4)
OBJS	+= bcm54213.o
endif

libbcm.a: $(OBJS)
	@echo "  AR    $@"
	@rm -f $@
	@$(AR) cr $@ $(OBJS)

-include $(DEPS)
