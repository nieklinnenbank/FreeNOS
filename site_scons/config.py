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
import shutil
from SCons.Script import *
from autoconf import *

# Then a command:
# scons defconfig SUBCONF=raspberry_model_bplus ARCH=arm SYSTEM=rpi
# which copies raspberry_model_bplus.conf to build.conf
# scons defconfig ARCH=arm SYSTEM=rpi
# which copies config/arm/rpi/build.conf to build.conf
#
# Or if only for the host environment:
# scons HOST:CC=gcc-4.4 HOST:CCFLAGS='-Wall'
#
# Any driver or server can then just do:
# #include <FreeNOS/Config.h>
# #ifdef MY_SETTING
# #endif

local_dict = {}

def initialize(target, host, params):
    """
    Initialize configuration subsystem.
    This will create a build.conf / build.host.conf if already existing.
    """
    if not os.path.exists('build.conf'):
	shutil.copyfile('config/' + params.get('ARCH', 'x86') + '/'
				  + params.get('SYSTEM', 'pc') + '/'
			          + params.get('CONF', 'build') + '.conf', 'build.conf')

    if not os.path.exists('build.host.conf'):
	shutil.copyfile('config/host/build.conf', 'build.host.conf')

    # Apply commandline arguments for use in build.conf's
    for key in params:
	if not key.startswith('HOST:'):
	    set_value(local_dict, key, params[key])
    apply_file('build.conf', target)

    for key in params:
	if key.startswith('HOST:'):
	    set_value(local_dict, key[5:], params[key])
    apply_file('build.host.conf', host)

    # Apply the current VERSION to the environments.
    version = open('VERSION').read().strip()
    target['VERSION'] = version
    host['VERSION'] = version

    # Reapply commandline arguments. This overwrites anything from build*.conf
    for key in params:
	if key.startswith('HOST:'):
	    set_value(host, key[5:], params[key])
	else:
	    set_value(target, key, params[key])

def set_value(env, key, value):
    """
    Apply new value to a environment.
    """
    if type(value) == bool or value in ('True', 'true', 'False', 'false'):
	env[key] = bool(value)
    else:
	try:
	    env[key] = int(value)
	except:
	    env[key] = value

def apply_file(conf_file, env):
    """
    Apply a configuration on the given SCons environment
    """
    global local_dict
    result = parse_file(conf_file)

    for item in result:
	if type(result[item]) is list:
	    env[item] = eval_list(result[item], result)
	elif type(result[item]) is str:
	    env[item] = eval_string(result[item], result)
	else:
	    env[item] = result[item]

    local_dict = {}


def parse_file(conf_file):
    """
    Parses a configuration file.
    Returns a dictionary with the parsed values.
    """
    global_dict = { 'Include' : parse_file }
    config_file = eval_string(conf_file)

    execfile(config_file, global_dict, local_dict)
    return local_dict

def eval_list(lst, replace_dict = None):
    """
    Replace configuration item values in the given list.
    """
    new_lst = []

    for item in lst:
	new_lst.append(eval_string(item, replace_dict))

    return new_lst

def eval_string(string, replace_dict = None):
    """
    Replace configuration item values in the given string or list.
    """
    if not replace_dict:
	replace_dict = local_dict

    split_string = string.split('}')
    final_string = ''

    for substr in split_string:
	idx = substr.find('${')
	if idx == -1:
	    final_string += substr
	else:
	    final_string += substr[0:idx]
	    var_name = substr[idx+2:]

	    if var_name in replace_dict:
		final_string += replace_dict[var_name]

    return final_string
