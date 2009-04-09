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

# Current version
VERSION      = 0
PATCHLEVEL   = 0
SUBLEVEL     = 1
EXTRAVERSION =
SVNVERSION   = r$(shell svn info 2> /dev/null | grep '^Revision' | awk '{print $$NF}')
FULLVERSION  = $(VERSION).$(PATCHLEVEL).$(SUBLEVEL)$(EXTRAVERSION)-$(SVNVERSION)

# Update Version.h
pre-hooks: include/Version.h
include/Version.h: include/Config.h
	rm -f include/Version.h
	echo -ne '#ifndef __VERSION_H\n' >> $@
	echo -ne '#define __VERSION_H\n' >> $@
	echo -ne '#define RELEASE "$(FULLVERSION)"\n' >> $@
	echo -ne '#define VERSIONCODE $(shell expr $(VERSION) \* 65536 + $(PATCHLEVEL) \* 256 + $(SUBLEVEL))\n' >> $@
	echo -ne '#define VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))\n' >> $@
	echo -ne '#define BUILDUSER "$(shell whoami)"\n' >> $@
	echo -ne '#define BUILDHOST "$(shell hostname)"\n' >> $@
	echo -ne '#define ARCH "$(shell readlink include/arch)"\n' >> $@
	echo -ne '#define COMPILER "$(shell $(CC) --version|head -n 1)"\n' >> $@
	echo -ne '#define DATETIME "$(shell date)"\n' >> $@
	echo -ne '#define COPYRIGHT "Copyright (C) 2009 Niek Linnenbank\\r\\n" \\\n' >> $@
	echo -ne '                  "This is free software; see the source for copying conditions.  There is NO\\r\\n" \\\n' >> $@
	echo -ne '                  "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\\r\\n\\r\\n"\n' >> $@
	echo -ne '#endif\n' >> $@

clean += include/Version.h
