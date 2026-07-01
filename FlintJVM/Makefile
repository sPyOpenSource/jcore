#
# Makefile
#

CIRCLEHOME = ../..

OBJS	= flint.o flint_utf8.o flint_execution.o flint_field_info.o flint_fields_data.o flint_method_info.o \
		  flint_const_pool.o flint_class_loader.o flint_common.o flint_array_object.o \
		  flint_java_class.o flint_java_object.o flint_java_thread.o flint_java_throwable.o \
		  flint_java_string.o flint_java_class_dict_node.o flint_java_string_dict_node.o \
		  flint_utf8_dict_node.o flint_mutex.o flint_debugger.o

libwvm.a: $(OBJS)
	@echo "  AR    $@"
	@rm -f $@
	@$(AR) cr $@ $(OBJS)

INCLUDE += -I ./Native/Inc

include $(CIRCLEHOME)/Rules.mk

-include $(DEPS)
