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

from build import *
from SCons.Script import *

#
# Generate an Extended 2 Filesystem image.
#
def generateExt2(target, source, env):

    # Open the list.
    list = open(str(source[0]))
    
    # Create a temporary directory.
    temp = tempfile.mkdtemp()
    
    # Read out which file to add.
    for line in list.readlines():

        # Copy them to the temporary directory. 
        copyWithParents(line.strip(), temp)

    # Create an 16MB Extended 2 Filesystem image.
    os.system("genext2fs -d " + temp + " -b 16384 " + str(target[0]))

    # Done.
    os.system("rm -rf " + temp)
    list.close()

ext2Builder = Builder(action     = generateExt2,
	    	      suffix     = '.ext2',
	    	      src_suffix = '.ext2desc')

target.Append(BUILDERS = { 'Ext2' : ext2Builder })

ext2 = target.Ext2('boot/boot.ext2', ['boot/boot.ext2desc'])
Depends(ext2, ['bin', 'lib', 'kernel', 'sbin', 'srv'])
AlwaysBuild(ext2)
