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

import os
import binascii

#
# Produces an C symbol from a given binary.
#
def binary_func(target, source, env):

    # Open source and target files.
    fin  = open(str(source[0]), "r")
    fout = open(str(target[0]), "w")

    # Read input file.
    data   = binascii.hexlify(fin.read())
    symbol = os.path.basename(str(source[0]))
    symbol = symbol.replace(".", "_")

    # Output header.
    fout.write("/*\n" +
               " * This file is auto-generated from " + str(source[0]) + "\n" +
               " */\n"
               "\n"
               "unsigned char __attribute__((aligned(4096))) " +
                             "__attribute__((__section__(\".binary\"))) " +
                symbol + "[] = { ")

    # Loop data.
    i = 0
    while i < len(data):
        fout.write("0x" + data[i] + data[i+1])

        if i < len(data) - 2:
            fout.write(",")
        i += 2

    # Output footer.
    fout.write("};\n\n")

    # All done.
    fin.close()
    fin.close()

def binary_str(target, source, env):

    return "  GEN " + str(target[0])

#
# Add ourselves to the given environment.
#
def generate(env):

    builder =  env.Builder(action  = env.Action(binary_func, binary_str))
    env.Append(BUILDERS = { 'Binary' : builder })

#
# We always exist.
#
def exists(env):

    return True
