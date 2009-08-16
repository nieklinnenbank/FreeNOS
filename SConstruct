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

import build
import version
import linn
import img
import pci
import iso
import emulate
import dist

#
# Update versioning header file.
#
version.regenerateHeader()

#
# Process subscripts.
#
SConscript(dirs = ['lib', 'kernel', 'bin', 'sbin', 'srv'])

#
# Per default, build libraries, kernel and user programs.
#
Default(['lib', 'kernel', 'bin', 'sbin', 'srv', 'etc'])
