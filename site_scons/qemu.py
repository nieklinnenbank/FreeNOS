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
		    qemu = 'qemu -curses -cdrom boot/boot.iso -s -S -d int,pcall,exec',
		   fqemu = 'qemu -curses -cdrom boot/boot.iso -s',
		   sqemu = 'qemu -sdl -cdrom boot/boot.iso -s')
