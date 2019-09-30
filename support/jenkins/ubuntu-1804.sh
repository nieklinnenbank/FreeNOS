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
# This script installs a KVM slave node with Ubuntu 18.04.
# To use it in Jenkins, you need add the node using the example node
# XML configuration file or manually add it as a KVM slave node.
# You need to add the label 'ubuntu-1804' to the node to use the example build job file.
#
# Use the following settings when running the installer:
#  - hostname: ubuntu-1804.kvm
#  - add 'jenkins' user (with sudo rights)
#
# After installation:
#  - install SSH server: $ sudo apt-get install openssh-server
#
# See the README file for more details on Jenkins setup.
#

NAME="ubuntu-1804"
UBUNTU_RELEASE="18.04.2"
ISO="ubuntu-$UBUNTU_RELEASE-desktop-amd64.iso"
JENKINS_PACKAGES="git default-jre"
COMPILER_PACKAGES="gcc-4.8 gcc-4.8-multilib g++-4.8 g++-4.8-multilib \
                   gcc-5 gcc-5-multilib g++-5 g++-5-multilib \
                   gcc-6 gcc-6-multilib g++-6 g++-6-multilib \
                   gcc-7 gcc-7-multilib g++-7 g++-7-multilib \
                   gcc-8 gcc-8-multilib g++-8 g++-8-multilib \
                   clang-3.9 \
                   clang-4.0 \
                   clang-5.0 \
                   clang-6.0 \
                   clang-7 \
                   clang-8 \
                   gcc-5-arm-linux-gnueabi g++-5-arm-linux-gnueabi \
                   gcc-6-arm-linux-gnueabi g++-6-arm-linux-gnueabi \
                   gcc-7-arm-linux-gnueabi g++-7-arm-linux-gnueabi \
                   gcc-8-arm-linux-gnueabi g++-8-arm-linux-gnueabi"
MISC_PACKAGES="build-essential scons genisoimage xorriso qemu-system binutils-multiarch"
PACKAGES="$JENKINS_PACKAGES $COMPILER_PACKAGES $MISC_PACKAGES"

# Download installer
if [ ! -e $ISO ] ; then
    wget http://releases.ubuntu.com/$UBUNTU_RELEASE/$ISO
fi

# Trace execution
set -x

# Run installer
virt-install --connect qemu:///system \
             --name $NAME \
             --memory 2048 \
             --vcpus 4 \
             --os-type linux \
             --os-variant ubuntu16.04 \
             --disk size=40 \
             --graphics vnc \
             --cdrom $ISO

# Bring slave up
virsh start $NAME && sleep 30

# Post installer commands
ssh -t jenkins@$NAME.kvm "sudo apt-get update && \
                          sudo apt-get dist-upgrade && \
                          sudo apt-get install $PACKAGES"
