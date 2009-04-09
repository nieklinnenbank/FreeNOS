#
# Copyright (C) 2009 Niek Linnenbank
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

#
# General variables.
#
TOPDIR	    := $(subst /tools/rules.mk,,$(lastword $(MAKEFILE_LIST)))
TOPDIR	    := ./$(subst tools/rules.mk,,$(TOPDIR))
SRCDIR      += $(CURDIR) $(srcdirs)

#
# Compiler chain.
#
CC	    := $(CROSS)gcc
C++	    := $(CROSS)g++
CPP	    := $(CROSS)cpp
LD          := $(CROSS)ld
AR	    := $(CROSS)ar
RANLIB	    := $(CROSS)ranlib
CPPFLAGS    += -isystem $(TOPDIR)/include -isystem $(CURDIR) $(foreach inc,$(includes),-isystem $(TOPDIR)/$(inc)) -nostdinc
CFLAGS	    += $(CPPFLAGS) -Wall -Werror -MD -O0 -g3 -fno-builtin $(cflags)
C++FLAGS    += $(CFLAGS) -fno-rtti -fno-exceptions
ASMFLAGS    += $(CPPFLAGS) -Wall -Werror -O0 -g3
LDFLAGS     += --whole-archive -nostdlib -nodefaultlibs $(ldflags)
ldscript    ?= $(TOPDIR)/tools/link.ld
LDSCRIPT     = $(ldscript)

#
# Source and object files.
#
C_SOURCES   += $(foreach dir,$(SRCDIR),$(wildcard $(dir)/*.c))
C_OBJECTS   += $(patsubst %.c,%.o,$(C_SOURCES))
C++_SOURCES += $(foreach dir,$(SRCDIR),$(wildcard $(dir)/*.cpp))
C++_OBJECTS += $(patsubst %.cpp,%.o,$(C++_SOURCES))
ASM_SOURCES += $(foreach dir,$(SRCDIR),$(wildcard $(dir)/*.S))
ASM_OBJECTS += $(patsubst %.S,%.o,$(ASM_SOURCES))
SOURCES	    += $(C_SOURCES) $(C++_SOURCES) $(ASM_SOURCES)
OBJECTS	    += $(C_OBJECTS) $(C++_OBJECTS) $(ASM_OBJECTS)

#
# Process library dependencies.
#
CFLAGS      += $(foreach lib,$(libraries),-include $(TOPDIR)/lib/$(lib)/Default.h -isystem $(TOPDIR)/lib/$(lib)/)
LDFLAGS	    += $(foreach lib,$(libraries), $(TOPDIR)/lib/$(lib)/$(lib).a)

#
# The default target is all.
#
all: pre-hooks subdirs $(OBJECTS) $(program) $(library) post-hooks
pre-hooks:
post-hooks:

#
# Recurse into subdirectories (NOT parallel!)
#
subdirs:
	@for dir in $(subdirs); do \
	  $(MAKE) -C $$dir; \
	done

#
# Link a program.
#
$(program): $(OBJECTS)
	$(LD) $(LDFLAGS) -T $(LDSCRIPT) -o $@ $(OBJECTS) $(objects)
	$(LD) $(LDFLAGS) -T $(LDSCRIPT) -o $@.bin --oformat binary $(OBJECTS) $(objects)

#
# Built a library.
#
$(library): $(OBJECTS)
	$(AR) ru $(library) $(OBJECTS) $(objects)
	$(RANLIB) $(library)

#
# C program source.
#
$(C_OBJECTS) : %.o : %.c
	$(CC) $(CFLAGS) -o $@ -c $<

#
# C++ program source.
#
$(C++_OBJECTS) : %.o : %.cpp
	$(C++) $(C++FLAGS) -o $@ -c $<

#
# Assembly program source.
#
$(ASM_OBJECTS) : %.o : %.S
	$(CC) $(ASMFLAGS) -o $@ -c $<

#
# Include generated dependancies.
#
-include $(foreach dir,$(SRCDIR),$(wildcard $(dir)*.d))

#
# Cleans up OBJECTS.
#
clean:
	@for dir in $(subdirs); do \
	  $(MAKE) -C $$dir clean; \
	done
	rm -f $(OBJECTS) $(program) $(program).bin $(library) *.d $(clean) $(CLEAN)

#
# These need to be forced.
#
.PHONY: clean $(subdirs) $(PHONY)
