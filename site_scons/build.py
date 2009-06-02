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

#
# Allow cross compilation.
#
try:
    cross = os.environ['CROSS']
except:
    cross = ""

#
# Command-line options.
# TODO: -Werror
#
cmdVars = Variables()
cmdVars.AddVariables(
    ('CC',        'Set the C compiler to use',   cross + 'gcc'),
    ('CXX',       'Set the C++ compiler to use', cross + 'g++'),
    ('LINK',      'Set the linker to use',       cross + 'ld'),
    ('CCFLAGS',   'Change C compiler flags',
		[ '-O0', '-g3', '-nostdinc', '-Wall', '-fno-builtin' ]),
    ('CXXFLAGS',  'Change C++ compiler flags',
		[ '-fno-rtti', '-fno-exceptions', '-nostdinc' ]),
    ('CPPFLAGS',  'Change C preprocessor flags', '-isystem include'),
    ('LINKFLAGS', 'Change the flags for the linker',
		[ '--whole-archive', '-nostdlib', '-T', 'kernel/arch/x86/user.ld' ])
)

#
# Define the default build environment.
#
target = DefaultEnvironment(CPPPATH   = '.',
		            ENV       = {'PATH' : os.environ['PATH'],
                    	                 'TERM' : os.environ['TERM'],
                        	         'HOME' : os.environ['HOME']},
			    variables = cmdVars)
Help(cmdVars.GenerateHelpText(target))

#
# Temporary environment for flat binary program files.
#
bintarget = target.Clone()
bintarget.Append(LINKFLAGS = [ '--oformat', 'binary' ])

#
# Build environment for programs on the host system.
#
host = Environment(variables = cmdVars)

#
# Prepares the given environment, using library and server dependencies.
#
def Prepare(env, libs = [], servers = []):
    
    # First create a safe copy.
    e = env.Clone()
    
    # Add libraries to the system include path and linker.
    for lib in libs:
        e['CPPFLAGS'] += ' -isystem lib/' + lib
	e['CPPFLAGS'] += ' -include lib/' + lib + '/Default.h'
	
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
