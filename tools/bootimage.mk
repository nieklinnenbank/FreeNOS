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
# Setup variables.
#
BOOTIMAGE := boot.img
MKEXT2    := genext2fs
BLOCKSIZE := 1024
BLOCKS    := $(shell echo `du -s --exclude=*.img --exclude=*.iso | awk '{ print $$1 }'`)
CLEAN     += $(BOOTIMAGE)

#
# Builds a boot image.
#
bootimage: $(BOOTIMAGE)

#
# Generate extended 2 filesystem image.
#
$(BOOTIMAGE):
	rm -f *.img *.iso
	$(MKEXT2) -b $(BLOCKS) -d . $(BOOTIMAGE)

.PHONY: $(BOOTIMAGE)
