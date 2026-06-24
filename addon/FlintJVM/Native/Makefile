#
# Makefile
#

CIRCLEHOME = ../../..

OBJS	= flint_native.o flint_native_array.o flint_native_character.o flint_native_class.o \
		  flint_native_constructor.o flint_native_double.o flint_native_float.o \
		  flint_native_interface.o flint_native_io_file.o flint_native_io_file_descriptor.o \
		  flint_native_io_file_input_stream.o flint_native_io_file_output_stream.o \
		  flint_native_math.o flint_native_method.o flint_native_object.o \
		  flint_native_reflection.o flint_native_string.o flint_native_system.o \
		  flint_native_thread.o flint_native_rpi.o

libnative.a: $(OBJS)
	@echo "  AR    $@"
	@rm -f $@
	@$(AR) cr $@ $(OBJS)

include $(CIRCLEHOME)/Rules.mk

INCLUDE += -I ../Inc

-include $(DEPS)
