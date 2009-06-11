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

from md5 import *
from sha import *
from build import *
from SCons.Action import *

def ChecksumBuilder(target, source, env):

    # Read source contents
    content = open(str(source[0])).read()
    sum     = None

    # Retrieve extension.
    file, ext = os.path.splitext(str(target[0]))

    # Process checksum request.
    if   ext == ".md5":
	sum = md5(content).hexdigest()
    
    elif ext == ".sha1":
	sum = sha(content).hexdigest()

    # Write to output file.
    out = open(str(target[0]), "w")
    out.write(sum + "  " + str(source[0]) + "\n")
    out.close()

def ChecksumString(target, source, env):

    # Retrieve extension.
    file, ext = os.path.splitext(str(target[0]))

    # Print out accordingly.    
    if   ext == ".md5":
	return "  MD5     " + str(target[0])

    elif ext == ".sha1":
	return "  SHA1    " + str(target[0])

    else:
	return "  CHKSUM  " + str(target[0])

# 
# Registers the Checksum builder.
# 
target.Append(BUILDERS =
{
    'Checksum': target.Builder(
        action = Action(ChecksumBuilder, ChecksumString)
     ),
})
