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

from SCons.Script import *
import os
import os.path
import shutil

#
# Allow cross compilation.
#
try:
    cross = os.environ['CROSS']
except:
    cross = ""

#
# Command-line options for the target build chain.
# TODO: -Werror
#
targetVars = Variables()
targetVars.AddVariables(
    ('CC',        'Set the target C compiler to use',   cross + 'gcc'),
    ('CXX',       'Set the target C++ compiler to use', cross + 'g++'),
    ('LINK',      'Set the target linker to use',       cross + 'ld'),
    ('CCFLAGS',   'Change target C compiler flags',
		[ '-O0', '-g3', '-nostdinc', '-Wall', '-Werror',
		  '-fno-builtin', '-Wno-write-strings', '-fno-stack-protector' ]),
    ('CXXFLAGS',  'Change target C++ compiler flags',
		[ '-fno-rtti', '-fno-exceptions', '-nostdinc' ]),
    ('CPPFLAGS',  'Change target C preprocessor flags', '-isystem include'),
    ('LINKFLAGS', 'Change the flags for the target linker',
		[ '--whole-archive', '-nostdlib', '-T', 'kernel/X86/user.ld' ])
)

#
# Define the default build environment.
#
target = DefaultEnvironment(CPPPATH   = '.',
		            ENV       = {'PATH' : os.environ['PATH'],
                    	                 'TERM' : os.environ['TERM'],
                        	         'HOME' : os.environ['HOME']},
			    variables = targetVars)
Help(targetVars.GenerateHelpText(target))

#
# The only target currently supported is x86.
#
try:
    os.symlink("X86", "include/FreeNOS")
except:
    pass

#
# Command-line options for the host build chain.
#
hostVars = Variables()
hostVars.AddVariables(
    ('HOSTCC',       'Set the host C compiler to use',   'gcc'),
    ('HOSTCXX',      'Set the host C++ compiler to use', 'g++'),
    ('HOSTCCFLAGS',  'Change host C compiler flags',
		[ '-O0', '-g3', '-Wall', '-Werror', '-Wno-write-strings' ]),
    ('HOSTCXXFLAGS', 'Change host C++ compiler flags',
		[ '' ]),
    ('HOSTCPPFLAGS',  'Change host C preprocessor flags', '-isystem include -DHOST'),
    ('HOSTLINKFLAGS', 'Change the flags for the host linker',
		[ '-Wl,-whole-archive' ])
)

#
# Build environment for programs on the host system.
#
host = Environment(CPPPATH   = '.',
		   CC        = '$HOSTCC',
		   CXX       = '$HOSTCXX',
		   CCFLAGS   = '$HOSTCCFLAGS',
		   CXXFLAGS  = '$HOSTCXXFLAGS',
		   CPPFLAGS  = '$HOSTCPPFLAGS',
		   LINKFLAGS = '$HOSTLINKFLAGS',
		   variables = hostVars)

host.Append(VARIANT = 'host')
host['LINKCOM'] += ' -Wl,--no-whole-archive'
Help(hostVars.GenerateHelpText(host))

#
# Output short command values per default, to
# distinguish commands from compiler warnings and errors more easy.
#
if ARGUMENTS.get('VERBOSE') is None:
    
    # Target chain command strings.
    target['CCCOMSTR']     = "  CC      $TARGET"
    target['CXXCOMSTR']    = "  CXX     $TARGET"
    target['ASCOMSTR']     = "  AS      $TARGET"
    target['ASPPCOMSTR']   = "  AS      $TARGET"
    target['LINKCOMSTR']   = "  LINK    $TARGET"
    target['ARCOMSTR']     = "  AR      $TARGET"
    target['RANLIBCOMSTR'] = "  RANLIB  $TARGET"
    
    # Host chain command strings.
    host['CCCOMSTR']       = "  HOSTCC  $TARGET"
    host['CXXCOMSTR']      = "  HOSTCXX $TARGET"
    host['ASCOMSTR']       = "  HOSTAS  $TARGET"
    host['ASPPCOMSTR']     = "  HOSTAS  $TARGET"
    host['LINKCOMSTR']     = "  LINK    $TARGET"
    host['ARCOMSTR']       = "  AR      $TARGET"
    host['RANLIBCOMSTR']   = "  RANLIB  $TARGET"

# Provide help aswell.
Help("\n"
     "VERBOSE: output verbose build commands\n"
     "    default: no\n")

#
# Prepares the given environment, using library and server dependencies.
#
def Prepare(env, libs = [], servers = []):
    
    # First create a safe copy.
    e = env.Clone()

    # Setup variant build directory, if needed.    
    try:
	e.VariantDir(e['VARIANT'], '.')
    except:
	pass
    
    # Loop all required libraries.
    for lib in libs:
        
	# Add them to the C preprocessor include path.
        e['CPPFLAGS'] += ' -isystem lib/' + lib
	e['CPPFLAGS'] += ' -include lib/' + lib + '/Default.h'
	
	# Link against the correct library variant.
	try:
	    e.Append(LIBPATH = [ '#lib/' + lib + '/' + e['VARIANT']])
	except:
	    e.Append(LIBPATH = [ '#lib/' + lib ])

	e.Append(LIBS = [ lib ])

    # Add servers to the system include path.
    for srv in servers:
	e['CPPFLAGS'] += ' -isystem srv/' + srv

    # For IPCServer.h. TODO: put this in libcommon!!!
    if len(servers) > 0:
	e['CPPFLAGS'] += ' -isystem srv'

    return e

#
# Adds a phony target to the given environment.
# Also see: http://www.scons.org/wiki/PhonyTargets
#
def PhonyTargets(env, **kw):

    for target,action in kw.items():
        env.AlwaysBuild(env.Alias(target, [], action))

#
# Copies a file from source to destination, including parent directories.
#
def copyWithParents(source, dest):

    # Lookup parents.
    parent = dest + '/' + os.path.dirname(source)

    # Create parent if needed. 
    if not os.path.exists(parent):
        os.makedirs(parent)

    # Perform copy. 
    shutil.copy(source, parent)
