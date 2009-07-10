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
from SCons.Action import *

#
# Generate a BootImage.
#
def generateBootImage(target, source, env):

    # Use the host compiled 'img' program.
    os.system("sbin/img/img " + str(source[0]) + " " + str(target[0]))
    os.system("gzip -fq '" + str(target[0]) + "'")

#
# Output user friendly command.
#
def generateBootImageStr(target, source, env):

    return "  IMG     " + str(target[0])

#
# Create boot image builder.
#
imgBuilder = Builder(action     = Action(generateBootImage, generateBootImageStr),
	    	     suffix     = '.img',
	    	     src_suffix = '.imgdesc')

target.Append(BUILDERS = { 'Img' : imgBuilder })

#
# Define dependencies and targets.
#
img = target.Img('boot/boot.img', ['boot/boot.imgdesc'])
target.Clean(img, '#boot/boot.img.gz')
Depends(img, ['bin', 'lib', 'kernel', 'sbin', 'srv'])
AlwaysBuild(img)
