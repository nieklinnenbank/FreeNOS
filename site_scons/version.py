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

import datetime
import pysvn
import os
import platform
import re
import SCons
import build

# Read current version.
text = open("VERSION").read().strip()
expr = re.compile("\.")
version = expr.split(text)
versionCode  = 0
versionPower = 16

# Calculate version code.
for v in version:
    versionCode  += int(v) * pow(2, versionPower)
    versionPower -= 8

# Include subversion revision.
try:
    svnClient  = pysvn.Client()
    svnInfo    = svnClient.info(".")
    release = text + "-r" + str(svnInfo.revision.number)
except:
    release = text

#
# Regenerate version header file.
#
def regenerateHeader():

    # Open versioning file for writing.
    out = open("include/Version.h", "w")
    out.write('#ifndef __VERSION_H\n' \
	      '#define __VERSION_H\n' \
	      '\n' \
	      '#define VERSIONCODE ' + str(versionCode) + '\n' \
    	      '#define VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))\n' \
	      '#define RELEASE   "' + release + '"\n' \
	      '#define COPYRIGHT "Copyright (C) ' + str(datetime.datetime.today().year) + ' Niek Linnenbank\\r\\n" \\\n' \
	      '                  "This is free software; see the source for copying conditions.  There is NO\\r\\n" \\\n' \
	      '                  "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\\r\\n\\r\\n"\n' \
	      '\n' \
	      '#define COMPILER  "' + build.target['CC'] + ' ' + build.target['CCVERSION'] + '"\n' \
	      '#define DATETIME  "' + str(datetime.datetime.today()) + '"\n' \
	      '#define ARCH      "' + os.readlink("include/arch") + '"\n' \
	      '\n' \
	      '#define BUILDUSER "' + os.environ['USER'] + '"\n' \
	      '#define BUILDHOST "' + platform.node() + '"\n' \
	      '#define BUILDOS   "' + platform.system() + ' ' + platform.release() + '"\n' \
	      '#define BUILDARCH "' + platform.machine() + '"\n' \
	      '#define BUILDCPU  "' + platform.processor() + '"\n' \
	      '#define BUILDPY   "' + platform.python_version() + '"\n' \
	      '#define BUILDER   "SCons ' + SCons.__version__ + '"\n' \
	      '#define BUILDPATH "' + os.getcwd() + '"\n')

    # Include SVN repository, if available.
    try:
	out.write('#define BUILDURL  "' + svnInfo.url + '"\n')
    except:
	out.write('#define BUILDURL  BUILDPATH\n')

    # Terminate #ifndef.
    out.write('\n' \
	      '#endif\n')
    # Done writing.
    out.flush()
    out.close()
