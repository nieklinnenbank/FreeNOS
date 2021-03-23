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
import time
import xml.etree.ElementTree as XmlParser

def timeoutChecker(proc, timeout):
    time.sleep(timeout)
    print("Timeout occured (" + str(timeout) + " sec) -- aborting")
    proc.terminate()
    proc.kill()
    sys.exit(1)

def writeXml(testname, iteration, data, env):
    """
    Write XML test output
    """
    outfile = env['BUILDROOT'] + '/xml/' + testname + '.' + str(iteration) + '.xml'

    try:
        os.makedirs(os.path.dirname(outfile))
    except Exception as e:
        pass

    f = open(outfile, 'w')
    f.write(data)
    f.close()

def stopTester(testproc, timeoutproc, reason, exitCode = 1):
    """
    Print stop message, terminates the test and timeout processes and exits the program.
    """
    print("Stopping test: " + reason)
    testproc.poll()
    print("proc.returncode = " + str(testproc.returncode))

    if testproc.returncode is None:
        print("Killing test process with PID " + str(testproc.pid))
        testproc.terminate()
        testproc.kill()

    try:
        print("Sending SIGTERM to timeout process with PID " + str(timeoutproc.pid))
        timeoutproc.terminate()
    except:
        pass

    try:
        print("Sending SIGKILL to timeout process with PID " + str(timeoutproc.pid))
        os.kill(timeoutproc.pid, signal.SIGKILL)
    except:
        pass

    if exitCode != 0:
        sys.exit(exitCode)

def runTester(target, source, env):
    """
    Run the FreeNOS autotester and collect XML results.
    """

    # Needed to workaround SCons problem with the pickle module.
    # See: http://stackoverflow.com/questions/24453387/scons-attributeerror-builtin-function-or-method-object-has-no-attribute-disp
    if 'pickle' in sys.modules and 'cPickle' in sys.modules:
        import imp

        del sys.modules['pickle']
        del sys.modules['cPickle']

        sys.modules['pickle'] = imp.load_module('pickle', *imp.find_module('pickle'))
        sys.modules['cPickle'] = imp.load_module('cPickle', *imp.find_module('cPickle'))

        import pickle
        import cPickle

    cmd = str(env['TESTCMD'])
    cmd = env.subst(cmd)

    # Launch process
    proc = subprocess.Popen(shlex.split(cmd), stdout=subprocess.PIPE, stdin=subprocess.PIPE, bufsize=0)

    # Launch a timeout process which will send a SIGTERM
    # to the process after a certain amount of time
    ch = multiprocessing.Process(target = timeoutChecker, args=(proc, env['TESTTIMEOUT']))
    ch.start()

    # When running from the FreeNOS interactive console, first wait for
    # the /bin/login prompt and give the login input values. After that,
    # start the autotester.
    if 'TESTPROMPT' in env:
        output=""
        while True:
            c = proc.stdout.read(1).decode('ascii', 'ignore')
            if c == '' and proc.poll() is not None:
                break

            sys.stdout.write(c)
            sys.stdout.flush()
            output += c

            if env['TESTPROMPT'] in output:
                time.sleep(1)
                proc.stdin.write(("root\n/test/run /test --xml --iterations " + \
                                  str(env['TESTITERATIONS']) + "\n").encode('utf-8'))
                proc.stdin.flush()
                break

    # Buffer XML output
    xml_data=""
    xml_testname=""
    iterations = 0

    while True:
        line = proc.stdout.readline().decode('ascii', 'ignore')
        if line == '' and proc.poll() is not None:
            break

        line = line.strip()

        sys.stdout.write(line + os.linesep)
        sys.stdout.flush()

        if line.startswith('<!-- Completed'):
            iterations += 1

            if line.startswith('<!-- Completed OK'):
                if iterations == env['TESTITERATIONS']:
                    stopTester(proc, ch, "Success", 0)
                    return
            else:
                stopTester(proc, ch, "Test terminated with failures")

        elif "<!-- Start" in line:
            xml_testname=line.split(' ')[2]
            xml_data=""

        elif line.startswith("<!-- Finish"):
            if line.split(' ')[3] != 'OK':
                try:
                    XmlParser.fromstring(xml_data)
                except:
                    xml_data = '<?xml version="1.0" encoding="UTF-8" ?>\r\n' + \
                               '<testsuites id="' + xml_testname + '" name="' + xml_testname + '">\r\n' + \
                               '<testsuite id="' + xml_testname + '" name="' + xml_testname + '" tests="1">\r\n' + \
                               '<testcase id="' + xml_testname + '.XMLParse" name="Terminated with errors (XML invalid)">\r\n' + \
                               '<failure message="' + xml_testname + ' terminated with errors (XML invalid)" type="ERROR" />\r\n' + \
                               '</testcase>\r\n' + \
                               '</testsuite>\r\n' + \
                               '</testsuites>\r\n'

            writeXml(xml_testname, iterations, xml_data, env)
            xml_data=""
            xml_testname=""
        else:
            xml_data += line + "\n"

    stopTester(proc, ch, "Unexpected end of test output")

def setupTester(env, **kw):

    # Make sure to pass the whole environment to the command.
    env.Append(ENV = os.environ)

    # Register SCons builder which always needs to run
    for target,action in kw.items():
        env.Append(TESTCMD = str(action))
        env.AlwaysBuild(env.Alias(target, [], runTester))

#
# Run the FreeNOS autotester inside qemu
#
def AutoTester(env, **kw):
    env = env.Clone()
    env.SetDefault(TESTTIMEOUT=60*7)
    env.SetDefault(TESTITERATIONS=1)
    env.SetDefault(TESTPROMPT="\nlogin: ")
    setupTester(env, **kw)

#
# Run the FreeNOS autotester inside qemu, with many iterations
#
def AutoTesterLoop(env, **kw):
    env = env.Clone()
    env.SetDefault(TESTTIMEOUT=60*40)
    env.SetDefault(TESTITERATIONS=75)
    env.SetDefault(TESTPROMPT="\nlogin: ")
    setupTester(env, **kw)

#
# Run FreeNOS autotester in a host OS local process
#
def LocalTester(env, **kw):
    env = env.Clone()
    env.SetDefault(TESTTIMEOUT=60*7)
    env.SetDefault(TESTITERATIONS=1)
    setupTester(env, **kw)

#
# Add ourselves to the given environment.
#
def generate(env):
    env.AddMethod(AutoTester)
    env.AddMethod(AutoTesterLoop)
    env.AddMethod(LocalTester)

#
# We always exist.
#
def exists(env):
    return True
