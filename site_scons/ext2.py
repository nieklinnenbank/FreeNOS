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
import tempfile
import shutil

from build import *
from SCons.Script import *
from SCons.Action import *

#
# Generate an Extended 2 Filesystem image.
#
def generateExt2(target, source, env):

    # Open the list.
    list = open(str(source[0]))
    
    # Copy the entire project.
    shutil.copytree(".", "tmproot")

    # Strip all binaries.
    os.system("strip `find tmproot` 2> /dev/null")
    
    # Create an Extended 2 Filesystem image.
    os.system("srv/filesystem/ext2/host/create " + str(target[0]) +
	      " tmproot -e '*.cpp' -e '*.h' -e '*.c' -e '*.o' -e 'lib*' -e 'doc' "
	      " -e 'SCons*' -e '*.a' -e '*.S' -e '*.ld' -e 'boot*'")

    # Done.
    os.system("rm -rf tmproot")
    list.close()

#
# Prints out a user friendly command-line string.
#
def generateExt2Str(target, source, env):

    return "  EXT2    " + str(target[0])

#
# Create extended 2 builder.
#
ext2Builder = Builder(action     = Action(generateExt2, generateExt2Str),
	    	      suffix     = '.ext2',
	    	      src_suffix = '.ext2desc')

target.Append(BUILDERS = { 'Ext2' : ext2Builder })

#
# Specify dependencies and targets.
#
ext2 = target.Ext2('boot/boot.ext2', ['boot/boot.ext2desc'])
Depends(ext2, ['bin', 'lib', 'kernel', 'sbin', 'srv'])
AlwaysBuild(ext2)
