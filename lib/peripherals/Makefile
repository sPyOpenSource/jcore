#
# Makefile
#
# Circle - A C++ bare metal environment for Raspberry Pi
# Copyright (C) 2014-2025  R. Stange <rsta2@gmx.net>
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

CIRCLEHOME = ../..

OBJS	= serial.o 

ifneq ($(strip $(RASPPI)),5)
OBJS	+= gpioclock.o gpiomanager.o gpiopin.o gpiopinfiq.o i2cmaster.o i2cmasterirq.o i2cslave.o \
	   pwmoutput.o spimaster.o spimasteraux.o spimasterdma.o
else
OBJS	+= gpiomanager2712.o gpiopin2712.o gpioclock-rp1.o \
	   pwmoutput-rp1.o i2cmaster-rp1.o spimaster-rp1.o spimasterdma-rp1.o
endif

libperi.a: $(OBJS)
	@echo "  AR    $@"
	@rm -f $@
	@$(AR) cr $@ $(OBJS)

include $(CIRCLEHOME)/Rules.mk

-include $(DEPS)