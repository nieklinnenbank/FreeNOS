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

from build import *

#
# Copies a program to the /srv/pci directory. When
# the PCIServer detects the given device, it will forkexec()
# it using the bus, slot and function number as an argument.
# A symbolic link may be a cleaner solution, when the
# FileSystems support it.
#
def PCIRegister(env, program, vendor, device):

    target.Command('#/etc/pci/' + hex(vendor) + ':' + hex(device),
		    str(program), Copy("$TARGET", "$SOURCE"))

target.AddMethod(PCIRegister, 'PCI')
