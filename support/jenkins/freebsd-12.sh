#!/bin/sh
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
# This script installs a slave node with FreeBSD 12.0.
# To use it in Jenkins, simple schedule the associated FreeBSD 12.0 build job.
#
# See the README file for more details on Jenkins setup.
#

NAME="freebsd-12"
JENKINS_PACKAGES="git openjdk11 bash"
COMPILER_PACKAGES="gcc gcc48 gcc5 gcc6 gcc7 gcc8 gcc9 \
                   llvm60 llvm70 llvm80 llvm90"
MISC_PACKAGES="qemu scons cdrkit-genisoimage xorriso"
PACKAGES="$JENKINS_PACKAGES $COMPILER_PACKAGES $MISC_PACKAGES"
RELEASE="`uname -r|cut -f 1,2 -d -`"
CHROOT="freebsd32"
CHROOTDIR="/usr/chroot/$CHROOT"
CHROOTURL="http://ftp.nl.freebsd.org/pub/FreeBSD/releases/i386/i386/$RELEASE/base.txz"

# Trace execution and stop on errors
set -x
set -e

# Set system hostname
sed "s/bazinga.localdomain/$NAME/" /etc/defaults/rc.conf > /tmp/rc.conf
mv /tmp/rc.conf /etc/defaults/rc.conf
sed "s/bazinga.localdomain/$NAME.localdomain/" /etc/hosts > /etc/hosts.bak
sed "s/bazinga/$NAME/" /etc/hosts.bak > /etc/hosts
rm /etc/hosts.bak
hostname -s $NAME

# Update system to latest patches
freebsd-update -F --not-running-from-cron fetch
freebsd-update -F --not-running-from-cron install
/usr/sbin/pkg || true
pkg update -f
pkg upgrade -y

# Create chroot
if [ ! -d $CHROOTDIR ] ; then
    mkdir -p $CHROOTDIR
    cd $CHROOTDIR
    fetch $CHROOTURL
    tar xpf base.txz
    rm base.txz
fi

# Inherit users and home directories from host
cd $CHROOTDIR
cp -Rp /usr/home $CHROOTDIR/usr
cp /etc/passwd $CHROOTDIR/etc
cp /etc/master.passwd $CHROOTDIR/etc
cp /etc/group $CHROOTDIR/etc
chroot $CHROOTDIR /usr/sbin/pwd_mkdb -p /etc/master.passwd

# Extract generated source archive
rm -rf $CHROOTDIR/usr/home/vagrant/FreeNOS
tar xf ~vagrant/src.tar -C $CHROOTDIR/usr/home/vagrant
rm -f ~vagrant/src.tar
rm -f $CHROOTDIR/home
ln -s /usr/home $CHROOTDIR/home

# Ensure DNS configuration is used
cp /etc/resolv.conf $CHROOTDIR/etc/

# Update chroot system to latest patches
chroot $CHROOTDIR freebsd-update -F --not-running-from-cron fetch
chroot $CHROOTDIR freebsd-update -F --not-running-from-cron install
chroot $CHROOTDIR sh -c 'ASSUME_ALWAYS_YES=yes /usr/sbin/pkg || true'
chroot $CHROOTDIR sh -c 'ASSUME_ALWAYS_YES=yes pkg update -f'
chroot $CHROOTDIR sh -c 'ASSUME_ALWAYS_YES=yes pkg upgrade -y'

# Install required packages for development
chroot $CHROOTDIR pkg install -y $PACKAGES

# Disable the FreeBSD linker, use the GNU linker
if [ -e $CHROOTDIR/usr/bin/ld.bfd ] ; then
    mv $CHROOTDIR/usr/bin/ld.bfd $CHROOTDIR/usr/bin/ld.bfd.orig
fi

# Configure SSH daemon
if [ "`grep sshd_enable=\"YES\" $CHROOTDIR/etc/defaults/rc.conf|wc -l`" -eq "0" ] ; then
   echo 'sshd_enable="YES"' >> $CHROOTDIR/etc/defaults/rc.conf
fi
if [ "`grep 'Port 2222' $CHROOTDIR/etc/ssh/sshd_config|wc -l`" -eq "0" ] ; then
   echo 'Port 2222' >> $CHROOTDIR/etc/ssh/sshd_config
fi

# Install SSH init script
cat > /etc/rc.d/sshd_chroot << EOF
#!/bin/sh
mount -t devfs devfs $CHROOTDIR/dev || true
chroot $CHROOTDIR /etc/rc.d/sshd \$1
EOF
chmod +x /etc/rc.d/sshd_chroot

# Start SSH daemon
/etc/rc.d/sshd_chroot start || true
