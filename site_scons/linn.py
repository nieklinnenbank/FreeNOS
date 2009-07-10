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
from SCons.Action import *

#
# Generate an LinnFS filesystem image.
#
def generateLinnFS(target=None, source=None, env=None):

    os.system("srv/filesystem/linn/host/create boot/boot.linn" \
	      " -d . -s -n 16384 -e '*.cpp' -e '*.h' -e '*.c' -e '*.o' -e 'lib*' -e 'doc' " \
	      " -e 'SCons*' -e '*.a' -e '*.S' -e '*.ld' -e 'boot*'")
    os.system("gzip -f boot/boot.linn")

#
# Prints out a user friendly command-line string.
#
def generateLinnFSStr(target, source, env):

    return "  LINN    boot/boot.linn.gz"

#
# Create LinnFS Action.
#
action  = Action(generateLinnFS, generateLinnFSStr)
