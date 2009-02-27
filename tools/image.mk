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
IMAGE	 := boot.iso
KERNEL 	 := kernel/kernel
MKISO	 := mkisofs
TMPDIR	 := $(shell mktemp -d -u)
CLEAN    += $(IMAGE)

#
# Build a bootable ISO image.
#
image: $(TMPDIR) $(IMAGE)

#
# Fill a temporary directory.
#
$(TMPDIR):
	mkdir -p $(TMPDIR)/boot/grub
	cp /boot/grub/stage* $(TMPDIR)/boot/grub
	cp $(KERNEL) $(TMPDIR)/boot/kernel
	cp srv/terminal/terminal.bin $(TMPDIR)/boot/terminal.bin
	cp srv/idle/idle.bin $(TMPDIR)/boot/idle.bin
	cp sbin/init/init.bin $(TMPDIR)/boot/init.bin
	cp bin/sh/sh.bin $(TMPDIR)/boot/sh.bin
	cp srv/memory/memory.bin $(TMPDIR)/boot/memory.bin
	cp srv/process/process.bin $(TMPDIR)/boot/process.bin
	echo 'timeout 0' >> $(TMPDIR)/boot/grub/menu.lst
	echo 'title Kernel' >> $(TMPDIR)/boot/grub/menu.lst
	echo 'root (cd)' >> $(TMPDIR)/boot/grub/menu.lst
	echo 'kernel /boot/kernel' >> $(TMPDIR)/boot/grub/menu.lst
	echo 'module /boot/terminal.bin' >> $(TMPDIR)/boot/grub/menu.lst
	echo 'module /boot/init.bin' >> $(TMPDIR)/boot/grub/menu.lst
	echo 'module /boot/idle.bin' >> $(TMPDIR)/boot/grub/menu.lst
	echo 'module /boot/sh.bin' >> $(TMPDIR)/boot/grub/menu.lst
	echo 'module /boot/memory.bin' >> $(TMPDIR)/boot/grub/menu.lst
	echo 'module /boot/process.bin' >> $(TMPDIR)/boot/grub/menu.lst

#
# Generate ISO image.
#
$(IMAGE):
	$(MKISO) -R -b boot/grub/stage2_eltorito -no-emul-boot \
	    -boot-load-size 4 -boot-info-table -o $(IMAGE) $(TMPDIR)
	rm -rf $(TMPDIR)

.PHONY: $(TMPDIR) $(IMAGE)
