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
import shutil
import tempfile
import version
import checksum
import linn

from build import *
from SCons.Script import *
from SCons.Action import *

#
# Generate an ISO 9660 image.
#
def generateISO(target, source, env):

    # Open the list.
    list = open(str(source[0]))
    
    # Create a temporary directory.
    temp = tempfile.mkdtemp()
    
    # Read out which file to add.
    for line in list.readlines():

	# Copy them to the temporary directory.
	copyWithParents(line.strip(), temp)

    # Create an bootable ISO image.
    os.system("mkisofs -R -b boot/grub/stage2_eltorito -no-emul-boot " +
	      "        -boot-load-size 4 -boot-info-table -V 'FreeNOS " + version.currentRev + "'" +
	      "        -o " + str(target[0]) + " " + temp)

    # Done.
    os.system("rm -rf " + temp)
    list.close()

#
# Output a user friendly command.
#
def generateISOstr(target, source, env):

    return "  ISO     " + str(target[0])

#
# Create the ISO builder.
#
isoBuilder = Builder(action     = Action(generateISO, generateISOstr),
	    	     suffix     = '.iso',
		     src_suffix = '.isodesc')

target.Append(BUILDERS = { 'ISO' : isoBuilder })

#
# Instructs to build an ISO and MD5+SHA1 checksums.
#
isoImage     = target.ISO('#boot/boot.iso', ['#boot/boot.isodesc'])
isoImageMd5  = target.Checksum('#boot/boot.iso.md5',  '#boot/boot.iso')
isoImageSha1 = target.Checksum('#boot/boot.iso.sha1', '#boot/boot.iso')

#
# Dependencies and target aliases.
#
target.AddPreAction(isoImage, linn.action)
target.Clean(isoImage, 'boot/boot.linn.gz')

Depends(isoImage, ['bin', 'lib', 'kernel', 'sbin', 'srv', '#boot/boot.img'])
Alias('iso', [ isoImage, isoImageMd5, isoImageSha1 ])
AlwaysBuild(isoImage, isoImageMd5, isoImageSha1)
