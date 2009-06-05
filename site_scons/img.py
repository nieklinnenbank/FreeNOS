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
import build
from SCons.Script import *

#
# Generate a BootImage.
#
def generateBootImage(target, source, env):

    # Use the host compiled 'img' program.
    os.system("sbin/img/img " + str(source[0]) + " " + str(target[0]))

imgBuilder = Builder(action     = generateBootImage,
	    	     suffix     = '.img',
	    	     src_suffix = '.imgdesc')

build.target.Append(BUILDERS = { 'Img' : imgBuilder })

img = build.target.Img('boot/boot.img', ['boot/boot.imgdesc'])
Depends(img, ['bin', 'lib', 'kernel', 'sbin', 'srv'])
AlwaysBuild(img)
