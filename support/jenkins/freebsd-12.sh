#
# Copyright (C) 2019 Niek Linnenbank
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

#
# This script installs a KVM slave node with FreeBSD 12.0.
# To use it in Jenkins, you need add the node using the example node
# XML configuration file or manually add it as a KVM slave node.
# You need to add the label 'freebsd-12' to the node to use the example build job file.
#
# Use the following settings when running the installer:
#  - hostname: freebsd-12.kvm
#  - enable SSH server (default)
#  - add 'jenkins' user (shell: csh, groups: wheel)
#
# See the README file for more details on Jenkins setup.
#

NAME="freebsd-12"
ISO="FreeBSD-12.0-RELEASE-i386-bootonly.iso"
JENKINS_PACKAGES="git openjdk11"
COMPILER_PACKAGES="gcc gcc48 gcc5 gcc6 gcc7 gcc8 gcc9 \
                   llvm35 llvm60 llvm70 llvm80"
MISC_PACKAGES="qemu scons cdrkit-genisoimage xorriso"
PACKAGES="$JENKINS_PACKAGES $COMPILER_PACKAGES $MISC_PACKAGES"

# Trace execution
set -x

# Download installer
if [ ! -e $ISO ] ; then
    wget https://download.freebsd.org/ftp/releases/i386/i386/ISO-IMAGES/12.0/$ISO
fi

# Run installer
virt-install --connect qemu:///system \
             --name $NAME \
             --memory 2048 \
             --vcpus 4 \
             --os-type freebsd \
             --os-variant freebsd10.0 \
             --disk size=16 \
             --graphics vnc \
             --cdrom $ISO

# Bring slave up
virsh start $NAME && sleep 30

# Post installer commands
ssh -t jenkins@$NAME.kvm "su -m root -c 'freebsd-update fetch && \
                          freebsd-update install && \
                          /usr/sbin/pkg; \
                          pkg update && pkg upgrade && pkg install $PACKAGES; \
                          ln -s /usr/local/openjdk11/bin/java /usr/local/bin/java; \
                          mv /usr/bin/ld.bfd /usr/bin/ld.bfd.orig'"
