#
# Copyright (C) 2010 Niek Linnenbank
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

from build import *
from version import *
from archive import *
from gdbinit import *

#
# Target build
#
build_env = target
Export('build_env')

# Build programs, libraries and servers.
VariantDir(target['BUILDROOT'] + '/lib', '#lib', duplicate = 0)
VariantDir(target['BUILDROOT'] + '/bin', '#bin', duplicate = 0)
VariantDir(target['BUILDROOT'] + '/server', '#server', duplicate = 0)
VariantDir(target['BUILDROOT'] + '/regress', '#regress', duplicate = 0)
SConscript(target['BUILDROOT'] + '/lib/SConscript')
SConscript(target['BUILDROOT'] + '/bin/SConscript')
SConscript(target['BUILDROOT'] + '/server/SConscript')
SConscript(target['BUILDROOT'] + '/regress/SConscript')

#
# Host build
#
build_env = host
Export('build_env')

# Build programs and libraries.
VariantDir(host['BUILDROOT'] + '/lib', '#lib', duplicate = 0)
VariantDir(host['BUILDROOT'] + '/bin', '#bin', duplicate = 0)
VariantDir(host['BUILDROOT'] + '/regress', '#regress', duplicate = 0)
SConscript(host['BUILDROOT'] + '/lib/SConscript')
SConscript(host['BUILDROOT'] + '/bin/SConscript')
SConscript(host['BUILDROOT'] + '/regress/SConscript')

#
# Kernel build
#
build_env = kernel
Export('build_env')

# Build the kernel.
VariantDir(kernel['BUILDROOT'] + '/kernel',
	    '#kernel/' + kernel['ARCH'] + '/' + kernel['SYSTEM'], duplicate = 0)
SConscript(kernel['BUILDROOT'] + '/kernel/SConscript')

