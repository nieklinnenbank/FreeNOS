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

import os
import os.path
from SCons.Action import *

def linnfs_generate(target, source, env):
    """
    Generate an LinnFS filesystem image.
    """
    rootfs_path = env.Dir(env['ROOTFS']).srcnode().path

    os.system("build/host/server/filesystem/linn/create '" + str(target[0]) + "' -s -n 16384 -d '" + rootfs_path + "'")

#
# Prints out a user friendly command-line string.
#
def linnfs_string(target, source, env):
    return "  LINN " + str(target[0])

#
# Add ourselves to the given environment.
#
def generate(env):
    builder = env.Builder(action = env.Action(linnfs_generate, linnfs_string))
    env.Append(BUILDERS = { 'LinnImage' : builder })

#
# We always exist.
#
def exists(env):
    return True
