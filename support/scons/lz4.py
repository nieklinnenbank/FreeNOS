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
import tempfile
import shutil

#
# Invoke the LZ4 command-line program to compress
#
def lz4_invoke(target, source, blocksize):
    lz4_cmd = 'lz4 -9 -q --content-size -B' + str(blocksize) + ' "' + str(source) + '" "' + str(target) + '"'
    r = os.system(lz4_cmd)
    if r != 0:
        raise Exception("failed to compress `" + str(target) + "': command `" + lz4_cmd + "' failed: exit code " + str(r >> 8))

#
# Compress with LZ4 using 256KiB blocks
#
def lz4_func_256k(target, source, env):
    return lz4_invoke(target[0], source[0], 5)

#
# Compress with LZ4 using 4KiB blocks
#
def lz4_func_4k(target, source, env):
    return lz4_invoke(target[0], source[0], 4096)

#
# String command representation for LZ4 builder.
#
def lz4_str(target, source, env):

    return "  LZ4  " + str(target[0])

#
# Add ourselves to the given environment.
#
def generate(env):

    builder_256k = env.Builder(action = env.Action(lz4_func_256k, lz4_str))
    builder_4k   = env.Builder(action = env.Action(lz4_func_4k, lz4_str))
    env.Append(BUILDERS = { 'Lz4Compress'   : builder_256k,
                            'Lz4Compress4K' : builder_4k })

#
# We always exist.
#
def exists(env):
    return Env.Detect('lz4')
