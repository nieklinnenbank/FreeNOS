#
# Copyright (C) 2015 Niek Linnenbank
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

from build import *
from archive import *

#
# Target build
#
build_env = target
Export('build_env')

# Build programs, libraries and servers.
VariantDir(target['BUILDROOT'] + '/lib', '#lib', duplicate = 0)
VariantDir(target['BUILDROOT'] + '/bin', '#bin', duplicate = 0)
VariantDir(target['BUILDROOT'] + '/srv', '#srv', duplicate = 0)
VariantDir(target['BUILDROOT'] + '/kernel', '#kernel', duplicate = 0)

# Install files to the target RootFS
target.TargetInstall('VERSION')
target.TargetInstall('build.conf', target['etc'])
target.TargetInstall('build.host.conf', target['etc'])

SConscript(target['BUILDROOT'] + '/lib/SConscript')
SConscript(target['BUILDROOT'] + '/bin/SConscript')
SConscript(target['BUILDROOT'] + '/srv/SConscript')
SConscript(target['BUILDROOT'] + '/kernel/' + target['ARCH'] + '/' + target['SYSTEM'] + '/SConscript')


#
# Host build
#
build_env = host
Export('build_env')

# Build programs and libraries.
VariantDir(host['BUILDROOT'] + '/lib', '#lib', duplicate = 0)
VariantDir(host['BUILDROOT'] + '/bin', '#bin', duplicate = 0)
VariantDir(host['BUILDROOT'] + '/srv', '#srv', duplicate = 0)
SConscript(host['BUILDROOT'] + '/lib/SConscript')
SConscript(host['BUILDROOT'] + '/bin/SConscript')
SConscript(host['BUILDROOT'] + '/srv/SConscript')
