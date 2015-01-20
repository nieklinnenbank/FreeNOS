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

from SCons.SConf  import *
from SCons.Script import *

#
# SConf helper function.
#
def TryCompileC(context):

    # Print out a debug message.
    context.Message('Checking for ' + context.env['TRYFLAG'] + ' ... ')

    # Use the simpelest C program possible.
    source_file = "int main(int argc, char **argv)" \
		  "{" \
		  "    return 0;" \
		  "}\n"

    # Try to compile and link it.
    result = context.TryLink(source_file, '.c')

    # Return the result status.
    context.Result(result)
    return result

#
# Checks if the compiler in the given environment supports
# the given command-line CFLAGS, and unsets it if not.
#
# Thanks to loonycyborg on #scons for his help!
#
def CheckCCFlags(env):

    cflags = env['CCFLAGS'][:]

    # Loop all CFLAGS.
    for flag in cflags:

	# Setup a temporary environment.
        conf = Configure(env.Clone(),
    			 custom_tests = { 'TryCompileC' : TryCompileC })
        conf.env.Replace(CCFLAGS   = flag)
        conf.env.Replace(LINKFLAGS = flag)
        conf.env.Replace(TRYFLAG   = flag)

	# Try to link a dummy program.
	result = conf.TryCompileC()

	# If it failed, mark to remove the CFLAG.
        if not result:
	    env['CCFLAGS'].remove(flag)

	# Done. Try next.
        conf.Finish()

#
# SConf helper function.
#
def TryCompileCXX(context):

    # Print out a debug message.
    context.Message('Checking for ' + context.env['TRYFLAG'] + ' ... ')

    # Use the simpelest C program possible.
    source_file = "int main(int argc, char **argv)" \
		  "{" \
		  "    return 0;" \
		  "}\n"

    # Try to compile and link it.
    result = context.TryCompile(source_file, '.cpp')

    # Return the result status.
    context.Result(result)
    return result

#
# Checks if the compiler in the given environment supports
# the given command-line CPPFLAGS, and unsets it if not.
#
# Thanks to loonycyborg on #scons for his help!
#
def CheckCXXFlags(env):

    cppflags = env['CXXFLAGS'][:]

    # Loop all CPPFLAGS.
    for flag in cppflags:

	# Setup a temporary environment.
        conf = Configure(env.Clone(),
    			 custom_tests = { 'TryCompileCXX' : TryCompileCXX })
        conf.env.Replace(CXXFLAGS  = flag)
        conf.env.Replace(LINKFLAGS = flag)
        conf.env.Replace(TRYFLAG   = flag)

	# Try to link a dummy program.
	result = conf.TryCompileCXX()

	# If it failed, mark to remove the CXXFLAGS.
        if not result:
	    env['CXXFLAGS'].remove(flag)

	# Done. Try next.
        conf.Finish()
