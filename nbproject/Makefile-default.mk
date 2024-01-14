#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile).#
_/_=/ShExtension=.shDevice=SAM9X60
ProjectDir=/Users/xuyi/Source/C/stm32.X
ProjectName=stm32
ConfName=default
TOOLCHAIN_XC16=/Applications/microchip/xc16/v1.70/bin
TOOLCHAIN_ARM=/Users/xuyi/Library/Arduino15/packages/arduino/tools/arm-none-eabi-gcc/4.8.3-2014q1/bin
.build-conf:  ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-default.mk image

# ------------------------------------------------------------------------------------
# Rules for buildStep: build and debug
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
image: /Users/xuyi/Source/C/stm32.X/stm32.d nbproject/Makefile-default.mk 
else
image: /Users/xuyi/Source/C/stm32.X/stm32 nbproject/Makefile-default.mk 
endif

.PHONY: /Users/xuyi/Source/C/stm32.X/stm32.d
/Users/xuyi/Source/C/stm32.X/stm32.d: 
	cd /Users/xuyi/Source/C/group && make debug

.PHONY: /Users/xuyi/Source/C/stm32.X/stm32
/Users/xuyi/Source/C/stm32.X/stm32: 
	cd /Users/xuyi/Source/C/group && make


# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	cd /Users/xuyi/Source/C/group && make clean

