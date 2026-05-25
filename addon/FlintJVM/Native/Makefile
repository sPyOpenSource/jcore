#
# Makefile
#

CIRCLEHOME = ../../..

OBJS	= flint_native.o flint_native_methods.o flint_native_method_info.o flint_native_methods_data.o

libnative.a: $(OBJS)
	@echo "  AR    $@"
	@rm -f $@
	@$(AR) cr $@ $(OBJS)

include $(CIRCLEHOME)/Rules.mk

-include $(DEPS)
