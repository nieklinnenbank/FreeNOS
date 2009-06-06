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

# vim: set et sw=3 tw=0 fo=awqorc ft=python:
# DistTarBuilder: tool to generate tar files using SCons
# Copyright (C) 2005, 2006  Matthew A. Nicholson
# Copyright (C) 2006  John Pye
#
# This file is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License version 2.1 as published by the Free Software Foundation.
#
# This file is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

import os
import sys
import tarfile
import version
import checksum
import SCons.Builder

from build import *
from SCons.Script import *

def DistTarEmitter(target,source,env):

   source,origsource = [], source

   excludeexts = env.Dictionary().get('DISTTAR_EXCLUDEEXTS',[])
   excludedirs = env.Dictionary().get('DISTTAR_EXCLUDEDIRS',[])

   # assume the sources are directories... need to check that
   for item in origsource:
      if os.path.isdir(str(item)):
         for root, dirs, files in os.walk(str(item)):

            # don't make directory dependences as that triggers full build
            # of that directory
            if root in source:
               #print "Removing directory %s" % root
               source.remove(root)

            # loop through files in a directory
            for name in files:
               ext = os.path.splitext(name)
               if not ext[1] in excludeexts:
                  relpath = os.path.join(root,name)
                  source.append(relpath)
            for d in excludedirs:
               if d in dirs:
                  dirs.remove(d)  # don't visit CVS directories etc
      else:
         ext = os.path.splitext(str(item))
         if not ext[1] in excludeexts:
            source.append(str(item))

   return target, source

def DistTarString(target, source, env):
   """
   This is what gets printed on the console. We'll strip out the list or source 
   files, since it tends to get very long. If you want to see the contents, the 
   easiest way is to uncomment the line 'Adding to TAR file' below.
   """
   return "  TAR     " + str(target[0])

def DistTar(target, source, env):
   """tar archive builder"""

   import tarfile

   env_dict = env.Dictionary()

   if env_dict.get("DISTTAR_FORMAT") in ["gz", "bz2"]:
      tar_format = env_dict["DISTTAR_FORMAT"]
   else:
      tar_format = ""

   # split the target directory, filename, and stuffix
   base_name = str(target[0]).split('.tar')[0]
   (target_dir, dir_name) = os.path.split(base_name)

   # create the target directory if it does not exist
   if target_dir and not os.path.exists(target_dir):
      os.makedirs(target_dir)

   # open our tar file for writing
   tar = tarfile.open(str(target[0]), "w:%s" % (tar_format,))

   # write sources to our tar file
   for item in source:
      item = str(item)
      tar.add(item,'%s/%s' % (dir_name,item))

   # all done
   tar.close()

def DistTarSuffix(env, sources):
   """tar archive suffix generator"""

   env_dict = env.Dictionary()
   if env_dict.has_key("DISTTAR_FORMAT") and env_dict["DISTTAR_FORMAT"] in ["gz", "bz2"]:
      return ".tar." + env_dict["DISTTAR_FORMAT"]
   else:
      return ".tar"

#
# Registers the DistTar builder.
#
target.Append(BUILDERS =
{
    'DistTar': target.Builder(
	action = SCons.Action.Action(DistTar, DistTarString),
	suffix = DistTarSuffix,
	emitter = DistTarEmitter,
	target_factory = target.fs.Entry,
     ),
})

target.Append(
          DISTTAR_EXCLUDEEXTS=['.o','.os','.so','.a','.dll','.cc','.cache',
			       '.pyc','.cvsignore','.dblite','.log', '.gz',
			       '.bz2', '.zip', '.bak', '.BAK', '.md5', '.sha1',
			       '.tar']
        , DISTTAR_EXCLUDEDIRS=['CVS','.svn','.sconf_temp', 'dist', 'host']
)

#
# Create a release GZipped TAR archive.
#
releaseTarGz       = target.DistTar("FreeNOS-" + version.current + ".tar.gz",
				     [target.Dir('#')],
				     DISTTAR_FORMAT = "gz")

releaseTarGzMd5    = target.Checksum("FreeNOS-" + version.current + ".tar.gz.md5",
				     "FreeNOS-" + version.current + ".tar.gz")

releaseTarGzSha1   = target.Checksum("FreeNOS-" + version.current + ".tar.gz.sha1",
				     "FreeNOS-" + version.current + ".tar.gz")

#
# Create a release BZipped TAR archive.
#
releaseTarBz2      = target.DistTar("FreeNOS-" + version.current + ".tar.bz2",
				     [target.Dir("#")],
				     DISTTAR_FORMAT = "bz2")

releaseTarBz2Md5   = target.Checksum("FreeNOS-" + version.current + ".tar.bz2.md5",
				     "FreeNOS-" + version.current + ".tar.bz2")

releaseTarBz2Sha1  = target.Checksum("FreeNOS-" + version.current + ".tar.bz2.sha1",
				     "FreeNOS-" + version.current + ".tar.bz2")

#
# Create a snapshot GZipped TAR archive.
#
snapshotTarGz      = target.DistTar("FreeNOS-" + version.currentRev + ".tar.gz",
				     [target.Dir("#")],
				     DISTTAR_FORMAT = "gz")

snapshotTarGzMd5   = target.Checksum("FreeNOS-" + version.currentRev + ".tar.gz.md5",
				     "FreeNOS-" + version.currentRev + ".tar.gz")

snapshotTarGzSha1  = target.Checksum("FreeNOS-" + version.currentRev + ".tar.gz.sha1",
				     "FreeNOS-" + version.currentRev + ".tar.gz")

#
# Create a snapshot BZipped TAR archive.
#				
snapshotTarBz2     = target.DistTar("FreeNOS-" + version.currentRev + ".tar.bz2",
				    [target.Dir("#")],
				    DISTTAR_FORMAT = "bz2")

snapshotTarBz2Md5  = target.Checksum("FreeNOS-" + version.currentRev + ".tar.bz2.md5",
				     "FreeNOS-" + version.currentRev + ".tar.bz2")

snapshotTarBz2Sha1 = target.Checksum("FreeNOS-" + version.currentRev + ".tar.bz2.sha1",
				     "FreeNOS-" + version.currentRev + ".tar.bz2")


Alias("release",  [ releaseTarGz,   releaseTarGzMd5,   releaseTarGzSha1,
		    releaseTarBz2,  releaseTarBz2Md5,  releaseTarBz2Sha1 ])
Alias("snapshot", [ snapshotTarGz,  snapshotTarGzMd5,  snapshotTarGzSha1,
		    snapshotTarBz2, snapshotTarBz2Md5, snapshotTarBz2Sha1 ])
