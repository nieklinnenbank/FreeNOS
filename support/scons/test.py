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

from SCons.Script import *
import subprocess
import multiprocessing
import shlex
import sys
import os
import os.path

def timeoutChecker(proc, timeout):
    time.sleep(timeout)
    print "Timeout occured (" + str(timeout) + " sec) -- aborting"
    proc.terminate()
    sys.exit(1)

def writeTap(testname, data, env):
    """
    Write TAP test output
    """
    outfile = env['BUILDROOT'] + '/tap/' + testname + '.tap'

    try:
        os.makedirs(os.path.dirname(outfile))
    except Exception as e:
        pass

    f = open(outfile, 'w')
    f.write(data)
    f.close()

def runQemu(target, source, env):
    """
    Run the FreeNOS autotester /test/run under qemu and collect TAP results.
    """

    # Needed to workaround SCons problem with the pickle module.
    # See: http://stackoverflow.com/questions/24453387/scons-attributeerror-builtin-function-or-method-object-has-no-attribute-disp
    import imp

    del sys.modules['pickle']
    del sys.modules['cPickle']

    sys.modules['pickle'] = imp.load_module('pickle', *imp.find_module('pickle'))
    sys.modules['cPickle'] = imp.load_module('cPickle', *imp.find_module('cPickle'))

    import pickle
    import cPickle

    # Launch qemu process
    proc = subprocess.Popen(shlex.split(env['QEMU']), stdout=subprocess.PIPE, stdin=subprocess.PIPE)

    # Launch a timeout process which will send a SIGTERM
    # to the process after a certain amount of time
    ch = multiprocessing.Process(target = timeoutChecker, args=(proc, 60 * 3))
    ch.start()

    # Give input to login and start the autotester.
    proc.stdin.write("root\n/test/run /test --tap\n")

    # Buffer TAP output
    tap=""

    while True:
        line = proc.stdout.readline()
        if line == '':
            break

        line = line.strip()

        print "line: " + line

        if line == '# Finish /test/run':
            proc.terminate()
            ch.terminate()
            return

        elif "# Start" in line:
            tap=line

        elif line.startswith("# Finish"):
            print "Writing: " + line[9:]
            writeTap(line[9:], tap, env)
            tap=""
        else:
            tap += line + "\n"

    print "Unexpected end of test output"
    proc.terminate()
    ch.terminate()
    sys.exit(1)

#
# Run the FreeNOS autotester inside qemu
#
def QemuTest(env, **kw):

    # Generate an environment, if not given.
    if not env:
        env = DefaultEnvironment()

    # Make sure to pass the whole environment to the command.
    env.Append(ENV = os.environ)
    env.Append(QEMU = kw['qemu'])
    env.AlwaysBuild(env.Alias('qemu_test', [], runQemu))

#
# Add ourselves to the given environment.
#
def generate(env):
    env.AddMethod(QemuTest)

#
# We always exist.
#
def exists(env):
    return True
