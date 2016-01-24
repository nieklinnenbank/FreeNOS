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

import os.path
import config
from SCons.Script import *
from autoconf import *

""" A list of files which are installed in the final RootFS """
rootfs_files = []
Export('rootfs_files')

def UseLibraries(env, libs = [], arch = None):
    """
    Prepares a given environment, by adding library dependencies.
    """
    # By default exclude host. If arch filter is set, apply it.
    if (not arch and env['ARCH'] == 'host') or (arch and env['ARCH'] != arch):
        return

    # Loop all required libraries.
    for lib in libs:

        # Add them to the include and linker path.
        env.Append(CPPPATH = [ '#lib/' + lib ])
        env.Append(LIBPATH = [ '#' + env['BUILDROOT'] + '/lib/' + lib ])
        env.Append(LIBS    = [ lib ])

def UseServers(env, servers = []):
    """
    Prepares a given environment by adding server dependencies
    """
    if '#server' not in env['CPPPATH']:
        env.Append(CPPPATH = [ '#server' ])

    for serv in servers:
        env.Append(CPPPATH = [ '#server/' + serv ])

def HostProgram(env, target, source):
    if env['ARCH'] == 'host':
	env.Program(target, source)

def TargetProgram(env, target, source, install_dir = None):
    if env['ARCH'] != 'host':
	env.Program(target, source)
	env.TargetInstall(target, install_dir)

def TargetLibrary(env, lib, source):
    if env['ARCH'] != 'host':
	env.Library(lib, source)

def CopyStrFunc(target, source, env):
    return "  " + env.subst_target_source("COPY $SOURCE => $TARGET", 0, target, source)

def DirStrFunc(target):
    return "  MKDIR " + target

def TargetInstall(env, source, target = None):
    if env['ARCH'] != 'host':
	SCons.Tool.install.install_action.strfunction = CopyStrFunc

	if not target:
	    target = '${ROOTFS}/' + Dir('.').srcnode().path

	env.Install(target, source)
	rootfs_files.append(str(target) + os.sep + os.path.basename(source))

def SubDirectories():
    dir_list = []
    dir_src  = Dir('.').srcnode().abspath

    if dir_src:
	for f in os.listdir(dir_src):
	    if os.path.isdir(dir_src + os.sep + f):
		dir_list.append(f)

	SConscript( dirs = dir_list )

Export('SubDirectories')

# Create target, host and kernel environments.
host = Environment(tools    = ["default", "phony", "test"],
		   toolpath = ["support/scons"])
host.AddMethod(HostProgram, "HostProgram")
host.AddMethod(TargetProgram, "TargetProgram")
host.AddMethod(TargetLibrary, "TargetLibrary")
host.AddMethod(UseLibraries, "UseLibraries")
host.AddMethod(UseServers, "UseServers")
host.AddMethod(TargetInstall, "TargetInstall")
host.Append(ROOTFS = '#${BUILDROOT}/rootfs')
host.Append(ROOTFS_FILES = [])
host.Append(bin     = '${ROOTFS}/bin',
	    etc     = '${ROOTFS}/etc',
	    server  = '${ROOTFS}/server',
            boot    = '${ROOTFS}/boot')

target = host.Clone(tools    = ["default", "bootimage", "iso", "binary", "linn", "phony", "test"],
                    toolpath = ["support/scons"])

# Apply configuration
config.initialize(target, host, ARGUMENTS)
config.write_header(target)
config.write_header(host)

# Enables verbose compilation command output.
if not target['VERBOSE']:
    target['CXXCOMSTR']    = host['CXXCOMSTR']    = "  CXX  $TARGET"
    target['CCCOMSTR']     = host['CCCOMSTR']     = "  CC   $TARGET"
    target['ASCOMSTR']     = host['ASCOMSTR']     = "  AS   $TARGET"
    target['ASPPCOMSTR']   = host['ASPPCOMSTR']   = "  AS   $TARGET"
    target['ARCOMSTR']     = host['ARCOMSTR']     = "  AR   $TARGET"
    target['RANLIBCOMSTR'] = host['RANLIBCOMSTR'] = "  LIB  $TARGET"
    target['LINKCOMSTR']   = host['LINKCOMSTR']   = "  LD   $TARGET"
    target['COPYSTR']      = host['COPYSTR']      = "  COPY $SOURCE => $TARGET"

# Verify the configured CFLAGS.
if not GetOption('clean'):
    CheckCCFlags(target)
    CheckCXXFlags(target)
