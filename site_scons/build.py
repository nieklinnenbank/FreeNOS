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
        env.Append(CPPFLAGS = [ '-include', 'lib/' + lib + '/Default.h' ])
        env.Append(LIBS    = [ lib ])

def UseServers(env, servers = []):
    """
    Prepares a given environment by adding server dependencies
    """
    if '#srv' not in env['CPPPATH']:
        env.Append(CPPPATH = [ '#srv' ])

    for serv in servers:
        env.Append(CPPPATH = [ '#srv/' + serv ])

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
host = Environment()
host.AddMethod(HostProgram, "HostProgram")
host.AddMethod(TargetProgram, "TargetProgram")
host.AddMethod(TargetLibrary, "TargetLibrary")
host.AddMethod(UseLibraries, "UseLibraries")
host.AddMethod(UseServers, "UseServers")
host.AddMethod(TargetInstall, "TargetInstall")
host.Append(ROOTFS = '#${BUILDROOT}/rootfs')
host.Append(ROOTFS_FILES = [])
host.Append(bin  = '${ROOTFS}/bin',
	    etc  = '${ROOTFS}/etc',
	    srv  = '${ROOTFS}/srv',
            boot = '${ROOTFS}/boot')

target = host.Clone(tools    = ["default", "bootimage", "iso", "binary", "linn"],
		    toolpath = ["site_scons"])

# Global top-level configuration.
global_vars = Variables('build.conf')
global_vars.Add('VERSION', 'FreeNOS release version number')
global_vars.Add(EnumVariable('ARCH', 'Target machine CPU architecture', 'x86',
			      allowed_values = ("x86",)))
global_vars.Add(EnumVariable('SYSTEM', 'Target machine system type', 'pc',
			      allowed_values = ("pc",)))
global_vars.Add(PathVariable('COMPILER', 'Target compiler chain', None))
global_vars.Add('BUILDROOT','Object output directory')
global_vars.Add(BoolVariable('VERBOSE', 'Output verbose compilation commands', False))
global_vars.Update(target)
global_vars.Update(host)

# System-specific configuration.
system_vars = Variables(target['COMPILER'])
system_vars.Add('CC', 'C Compiler')
system_vars.Add('AS', 'Assembler')
system_vars.Add('LD', 'Linker')
system_vars.Add('CCFLAGS', 'C/C++ Compiler flags')
system_vars.Add('CXXFLAGS', 'C++ Compiler flags')
system_vars.Add('ASFLAGS', 'Assembler flags')
system_vars.Add('LINKFLAGS', 'Linker flags')
system_vars.Add('LINKKERN', 'Linker flags for the kernel linker script')
system_vars.Add('LINKUSER', 'Linker flags for user programs linker script')
system_vars.Add('CPPPATH', 'C Preprocessor include directories')
system_vars.Update(target)
target.Append(LINKFLAGS = target['LINKUSER'])

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

# Host environment uses a different output directory.
host.Replace(ARCH      = 'host')
host.Replace(SYSTEM    = 'host')
host.Replace(BUILDROOT = 'build/host')
host.Append (CPPFLAGS  = '-D__HOST__')
host.Append (CPPPATH   = [ '#include' ])

# Provide configuration help.
Help(global_vars.GenerateHelpText(target))
Help(system_vars.GenerateHelpText(target))

# Make a symbolic link to the system-specific headers.
try:
    os.unlink("include/FreeNOS")
except:
    pass

try:
    os.symlink(target['ARCH'], "include/FreeNOS")
except:
    pass

