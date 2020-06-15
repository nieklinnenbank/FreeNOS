#
# Copyright (C) 2020 Niek Linnenbank
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

import os
import os.path
from SCons.Action import *

def strip_generate(target, source, env):
    """
    Strip debugging symbols from input files (if any)
    """
    for f in source:
        if os.access(str(f), os.X_OK):
            strip_cmd = env['STRIP'] + ' -o ' + str(target[0]) + ' ' + str(source[0])
            r = os.system(strip_cmd)
            if r != 0:
                print("WARNING: failed to strip `" + str(source[0]) + "': command `" + strip_cmd + "' failed: exit code " + str(r>>8))
                break

def strip_string(target, source, env):
    """
    Prints out a user friendly command-line string.
    """
    return "  STRIP " + str(target[0])

def generate(env):
    """
    Registers the Strip builder
    """
    builder = env.Builder(action = env.Action(strip_generate, strip_string))
    env.Append(BUILDERS = { 'Strip' : builder })

def exists(env):
    return True
