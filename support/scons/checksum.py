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

import hashlib
import os

#
# Calculate an MD5 or SHA1 checksum for a given file.
#
def Checksum(target, source, env):

    # Read source contents
    content = open(source).read()
    sum     = None

    # Retrieve extension.
    file, ext = os.path.splitext(target)

    # Process checksum request.
    if   ext == ".md5":
	print "  MD5 %s" % target
	sum = hashlib.md5(content).hexdigest()

    elif ext == ".sha1":
	print "  SHA %s" % target
	sum = hashlib.sha1(content).hexdigest()

    # Write to output file.
    out = open(target, "w")
    out.write(sum + "  " + source + "\n")
    out.close()

