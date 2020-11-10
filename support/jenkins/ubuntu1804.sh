#!/bin/bash
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

JENKINS_PACKAGES="git default-jre"
COMPILER_PACKAGES="gcc-4.8 gcc-4.8-multilib g++-4.8 g++-4.8-multilib \
                   gcc-5 gcc-5-multilib g++-5 g++-5-multilib \
                   gcc-6 gcc-6-multilib g++-6 g++-6-multilib \
                   gcc-7 gcc-7-multilib g++-7 g++-7-multilib \
                   gcc-8 gcc-8-multilib g++-8 g++-8-multilib \
                   gcc-9 gcc-9-multilib g++-9 g++-9-multilib \
                   gcc-10 gcc-10-multilib g++-10 g++-10-multilib \
                   clang \
                   clang-3.9 \
                   clang-4.0 \
                   clang-5.0 \
                   clang-6.0 \
                   clang-7 \
                   clang-8 \
                   clang-9 \
                   clang-10 \
                   gcc-5-arm-linux-gnueabi g++-5-arm-linux-gnueabi \
                   gcc-6-arm-linux-gnueabi g++-6-arm-linux-gnueabi \
                   gcc-7-arm-linux-gnueabi g++-7-arm-linux-gnueabi \
                   gcc-8-arm-linux-gnueabi g++-8-arm-linux-gnueabi"
MISC_PACKAGES="build-essential scons genisoimage xorriso binutils-multiarch u-boot-tools valgrind liblz4-tool"
PACKAGES="$JENKINS_PACKAGES $COMPILER_PACKAGES $MISC_PACKAGES"

# Include common functions
source common.sh

# Disable interactive dialogs for APT
export DEBIAN_FRONTEND=noninteractive

# Set hostname
echo ubuntu1804 > /etc/hostname

# Auto select mirror
sed -i 's/us.archive.ubuntu.com/nl.archive.ubuntu.com/' /etc/apt/sources.list

# Remove hardcoded DNS servers
cat /etc/netplan/01-netcfg.yaml | grep -v 'nameservers:' | grep -v 'addresses: ' > /etc/netplan/01-netcfg.yaml.new
mv /etc/netplan/01-netcfg.yaml.new /etc/netplan/01-netcfg.yaml
netplan apply
sleep 5

# Ensure GRUB will not complain about out-of-sync installed bootloaders
firstdisk="`readlink -f /dev/disk/by-uuid/* | sort | head -n 1`"
echo "grub-pc grub-pc/install_devices multiselect $firstdisk" > /tmp/input.txt
debconf-set-selections /tmp/input.txt
rm -f /tmp/input.txt

# Update system to latest patches
add-apt-repository -y ppa:ubuntu-toolchain-r/test
run_command_retry "apt-get update"
run_command_retry "apt-get dist-upgrade -y"

# Use Qemu from APT if not provided
if [ -z "$QEMU_URL" ] ; then
  PACKAGES="$PACKAGES qemu-system"
elif [ ! -e /usr/local/bin/qemu-system-arm ] ; then
    # Compile Qemu from source
    run_command_retry "apt-get install -y build-essential pkg-config libglib2.0-dev libpixman-1-dev bison flex"
    rm -rf qemu-git
    git clone --depth=1 -b $QEMU_BRANCH $QEMU_URL qemu-git
    cd qemu-git
    ./configure --prefix=/usr/local --target-list=arm-softmmu,i386-softmmu

    if [ ! -z "$SLAVE_CPUS" ] ; then
        make -j$SLAVE_CPUS
    else
        make -j5
    fi
    make install
    make clean
    cd ..
    rm -rf qemu-git
fi

# Extract generated source archive
rm -rf ~vagrant/FreeNOS
tar xf ~vagrant/src.tar -C ~vagrant
rm -f ~vagrant/src.tar

# Install all packages needed for development
run_command_retry "apt-get install -y $PACKAGES"

# Node is ready. Enable port 2222
if [ "`grep 'Port 2222' /etc/ssh/sshd_config|wc -l`" -eq "0" ] ; then
    echo 'Port 22' >> /etc/ssh/sshd_config
    echo 'Port 2222' >> /etc/ssh/sshd_config
    service sshd reload
fi
