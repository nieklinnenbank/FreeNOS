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

import os
import build

build.target.Append(ENV = os.environ)
build.PhonyTargets(build.target,
		   qemu        = 'qemu -usb -cdrom boot/boot.iso',
		   qemu_debug  = 'qemu -curses -usb -cdrom boot/boot.iso -s -S -d int,pcall,exec',
		   qemu_curses = 'qemu -curses -usb -cdrom boot/boot.iso',
		   qemu_sdl    = 'qemu -sdl -cdrom boot/boot.iso',
		   bochs       = 'bochs -q "ata0-slave: type=cdrom, path=boot/boot.iso, status=inserted" ' \
				           '"boot: cdrom"',
		   bochs_debug = 'bochs -q "ata0-slave: type=cdrom, path=boot/boot.iso, status=inserted" ' \
				           '"boot: cdrom" "gdbstub: enabled=1"')
