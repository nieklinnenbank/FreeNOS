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

import datetime
import os
import platform
import re
import SCons
from build import *

#
# Retrieves Subversion information.
#
def info_receiver(path, info, pool):
    global currentRev, url
    currentRev = current + "-r" + str(info.rev)
    url = str(info.URL)

# Read current version.
current      = target['VERSION']
expr         = re.compile("\.")
version      = expr.split(current)
versionCode  = 0
versionPower = 16
url          = None

# Calculate version code.
for v in version:
    versionCode  += int(v) * pow(2, versionPower)
    versionPower -= 8

# Include subversion revision.
try:
    import svn
    import svn.client
    import svn.core

    context  = svn.client.svn_client_create_context()
    rev      = svn.core.svn_opt_revision_t()
    rev.king = svn.core.svn_opt_revision_head
    path     = os.getcwd()
    currentRev = None

    svn.client.info(path, rev, rev, info_receiver, False, context)

# Not a SVN repository.
except:
    currentRev = current + "-local"

# Set the full version revision in environments.
target['RELEASE'] = currentRev
kernel['RELEASE'] = currentRev
host['RELEASE']   = currentRev

# Attempt to retrieve the correct compiler version
try:
    compiler = os.popen(target['CC'] + " --version | head -n 1").read().strip()
except:
    compiler = target['CC'] + ' ' + target['CCVERSION']

#
# Converts the into to a string, and escapes double quotes.
#
def escape(obj):
    return str(obj).replace('"', '\\"')

#
# Write all gathered data to the version.h file.
#
out = open("include/Version.h", "w")
out.write('#ifndef __VERSION_H\n' \
	  '#define __VERSION_H\n' \
	  '\n' \
	  '#define VERSIONCODE ' + escape(versionCode) + '\n' \
    	  '#define VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))\n' \
	  '#define RELEASE   "' + escape(currentRev) + '"\n' \
	  '#define COPYRIGHT "Copyright (C) ' + escape(datetime.datetime.today().year) + ' Niek Linnenbank\\r\\n" \\\n' \
	  '                  "This is free software; see the source for copying conditions.  There is NO\\r\\n" \\\n' \
	  '                  "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."\n' \
	  '\n' \
	  '#define COMPILER  "' + escape(compiler) + '"\n' \
	  '#define DATETIME  __DATE__ " " __TIME__\n' \
	  '#define ARCH      "' + escape(target['ARCH']) + '"\n' \
	  '#define SYSTEM    "' + escape(target['SYSTEM']) + '"\n' \
	  '\n' \
	  '#define BUILDUSER "' + escape(os.environ['USER']) + '"\n' \
	  '#define BUILDHOST "' + escape(platform.node()) + '"\n' \
	  '#define BUILDOS   "' + escape(platform.system()) + ' ' + escape(platform.release()) + '"\n' \
	  '#define BUILDARCH "' + escape(platform.machine()) + '"\n' \
	  '#define BUILDPY   "Python ' + escape(platform.python_version()) + '"\n' \
	  '#define BUILDER   "SCons ' + escape(SCons.__version__) + '"\n' \
	  '#define BUILDPATH "' + escape(os.getcwd()) + '"\n')

# Include SVN repository, if available.
if url is not None:
    out.write('#define BUILDURL  "' + escape(url) + '"\n')
else:
    out.write('#define BUILDURL  BUILDPATH\n')

# Terminate #ifndef.
out.write('\n' \
	  '#endif\n')

# Done writing.
out.flush()
out.close()

