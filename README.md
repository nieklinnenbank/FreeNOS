About {#mainpage}
=====

FreeNOS (Free Niek's Operating System) is an experimental microkernel based
operating system for learning purposes written in C++. You may use the code
as you wish under the terms of the GPLv3.

Visit the project website at http://www.FreeNOS.org for more information.

Features
========

* Intel x86 (PC) and ARMv6/ARMv7 architectures (Raspberry Pi 1,2,3, Allwinner H2+/H3)
* Virtual memory
* Simple task scheduling
* Inter Process Communication (IPC)
* Symmetric Multi Processing with MPI support (via console and ethernet)
* Devices:
    * VGA/Keyboard consoles (also supported by Ed's libteken http://80386.nl/projects/libteken/)
    * i8250 serial UART
    * PCI host controller
    * CMOS RTC clock
    * ATA host controller
    * Allwinner H2+/H3 ethernet controller
    * Loopback ethernet controller
* Filesystems:
    * Virtual file system (VFS)
    * Temporary file system (TmpFS)
    * Linnenbank file system (LinnFS)
* Networking (IP, UDP, ICMP, ARP)
* POSIX, ANSI C libraries
* Dynamic and Shared memory
* Fully automatic autotester
* Automated continuous integration using jenkins (http://www.jenkins.io) and Vagrant (http://www.vagrantup.com)
* All sources documented with Doxygen (http://www.doxygen.org/)
* User and kernel code written from scratch in C++
* Very small microkernel (~2K lines of C++ code including a tiny part in assembly)
* Builds with recent GCC (http://gcc.gnu.org/), LLVM (http://www.llvm.org/) and SCons (http://www.scons.org/) versions on POSIX systems

Host Setup
==========

First install all required build dependencies. FreeNOS needs SCons, an C++ compiler and
for Intel targets a tool to generate ISO images. Follow the instructions below to install
the build dependencies on your host OS.

*Ubuntu*

Update your system repository cache and install the required development tools using:

    $ sudo apt-get update
    $ sudo apt-get install build-essential scons genisoimage xorriso qemu-system binutils-multiarch u-boot-tools liblz4-tool

If your Ubuntu host is 64-bit, you need to install the GCC multilib package
to cross compile for the 32-bit architecture:

    $ sudo apt-get install gcc-multilib g++-multilib

Alternatively, you can install the LLVM/Clang compiler as well:

    $ sudo apt-get install clang

*FreeBSD*

Update your system repository cache and install the required development tools using:

    % su -
    # pkg update
    # pkg install qemu scons cdrkit-genisoimage xorriso gcc u-boot-tools liblz4

On FreeBSD, make sure that the latest version of the GNU linker (from pkg) is used:

    # mv /usr/bin/ld.bfd /usr/bin/ld.bfd.orig

Building FreeNOS
================

To download and extract the released source code on your host OS, run the
following commands. Replace 'x.x.x' with the version of FreeNOS:

    $ wget http://www.FreeNOS.org/pub/FreeNOS/source/FreeNOS-x.x.x.tar.gz
    $ tar zxf FreeNOS-x.x.x.tar.gz

To get the latest development source, you can clone the GIT archive:

    $ git clone https://github.com/nieklinnenbank/FreeNOS

To build FreeNOS using default settings (Intel, using GCC with debugging enabled), run:

    $ scons

To build FreeNOS with all full build commands printed on the console, set the
VERBOSE build variable to True:

    $ scons VERBOSE=True

To build FreeNOS with compiler optimizations enabled, set DEBUG to False.
Note that will make debugging using GDB more difficult:

    $ scons DEBUG=False

Instead of providing build variables on the command line, you can
also change the 'build.conf' configuration file for the target. The build configuration
file contains build variables, such as compiler flags and parameters for the target.
Similary, the 'build.host.conf' file contains build variables for the host OS programs.
See the example build.conf and build.host.conf files for more details.

Additionally, any environment variables set in the shell will be automatically converted
to identical build variables when running scons.

To build for the Raspberry Pi 2, copy the example build configuration file and run SCons:

    $ cp config/arm/raspberry2/build.conf .
    $ scons

To cleanup your build directory, use:

    $ scons -c

Or use the following command to also remove temporary SCons files:

    $ rm -rf build .sconf_temp .sconsign.dblite

Running FreeNOS
===============

To run the autotester of FreeNOS on the host OS, run:

    $ scons test

To run the same autotester on FreeNOS under Qemu, run:

    $ scons qemu_test

To start FreeNOS in a Qemu virtual machine with a serial console,
use the following command:

    $ scons qemu

To debug FreeNOS using GDB, you need to build with the build variable DEBUG=True.
Debugging symbols must available in the compiled programs which the debugger needs to
translate between source code, CPU instructions and vice versa. Compiler optimizations
are disabled with DEBUG=True, which gives reduced performance but improved
debugging experience. By default, the DEBUG build variable is already set to True.

In case you have configured your build.conf with DEBUG=False, you can pass it on
the commandline (or edit your build.conf to set DEBUG to True):

    $ scons DEBUG=True

Ensure that your host OS has GDB available for debugging the target architecture (Intel or ARM).
For Ubuntu:

    $ sudo apt-get install gdb-multiarch

Open two terminals. In the first terminal, launch FreeNOS in Qemu with the internal gdbserver enabled:

    $ scons qemu_debug

In the second terminal, start GDB and connect to the Qemu internal gdbserver using:

    $ gdb-multiarch ./build/intel/pc/kernel/intel/pc/kernel
    (gdb) target remote localhost:1234

You can now use standard GDB commands to interactively debug the FreeNOS kernel for intel.
Similarly, you can also debug a user program (./build/intel/pc/bin/XXX). Note that when
debugging user programs, the mapped virtual memory changes frequently due to scheduling.
When you set a breakpoint on a virtual memory address, for example a function, it can happen
that another program is scheduled and executes on the same virtual address which incorrectly
triggers the breakpoint. In order to only trigger the breakpoint for a selected user program,
you can put a condition on a breakpoint that matches the process name:

    (gdb) condition 1 $_streq((char *)0xe0000000, "./server/datastore/server")

The above command puts a condition on the breakpoint with index number 1 that says it should
only halt execution when the program name string equals "./server/datastore/server".

intel/pc
--------

For Intel targets, you can also launch Qemu using the graphical VGA console (using SDL):

    $ scons qemu_sdl

To test FreeNOS on real hardware, write the boot ISO to a bootable device, such as an USB disk.
Insert the USB disk to your host OS and find the appropriate device name:

    $ fdisk -l

Alternatively, use the dmesg command to find the device name. Write the boot ISO
using the following command, where sdX is the name of the USB disk device.
Note: make sure to select the correct device to prevent data loss:

    $ sudo dd if=build/intel/pc/boot.iso of=/dev/sdX
    $ sudo sync

Insert the USB disk device to the target system and make sure to choose it as
boot device when the computer starts (e.g. via BIOS). You should see the FreeNOS VGA console.

arm/raspberry
-------------

To build FreeNOS for running on the Raspberry Pi 1, use the provided build.conf file
and rebuild the system:

    $ cp config/arm/raspberry/build.conf .
    $ scons

To test FreeNOS for the raspberry pi 1 under Qemu, rebuild the system
with the QEMU_BUILD build variable set to True:

    $ scons QEMU_BUILD=True
    $ scons qemu

To test on a real Raspberry Pi using SD card, download the latest Raspbian 'Lite' image
from https://www.raspberrypi.org/downloads/raspbian/ and unzip it. Insert the SD card to
your reader/writer slot on your host OS and find the appropriate device name using:

    $ fdisk -l

Alternatively, use the dmesg command to find the device name. Write the Raspbian image to the
SD card using the following command, where sdX is the name of the SD card device.
Note: make sure to select the correct device to prevent data loss:

    $ sudo dd if=/path/to/raspbian/image.img of=/dev/sdX
    $ sudo sync

Re-insert the SD-card to auto mount it on your host OS. Copy the FreeNOS raspberry pi kernel executable
to the '/boot' partition on the Raspbian image using (on Ubuntu):

    $ sudo cp build/arm/raspberry/kernel.img /media/boot/kernel.img
    $ sudo sync
    $ sudo umount /media/boot

You will need a USB-to-TTL-serial cable to connect to the Raspberry Pi UART.
Any USB-to-TTL-serial cable will work, for example the Adafruit USB-to-TTL-serial cable:

    https://www.adafruit.com/product/954

Connect your USB-to-serial cable to the Raspberry Pi using the GPIO pins:

    [ ] [ ]
    [ ] [X]  Pin 4  (V5)     <---  Red (only needed if powered via USB)
    [ ] [X]  Pin 6  (Ground) <---  Black
    [ ] [X]  Pin 8  (RX)     <---  White
    [ ] [X]  Pin 10 (TX)     <---  Green
    [ ] [ ]
    [ ] [ ]
    [ ] [ ]
    [ ] [ ]
    [ ] [ ]

Also see the following tutorial by Adafruit on connecting the serial cable to the Raspberry Pi:

    https://learn.adafruit.com/adafruits-raspberry-pi-lesson-5-using-a-console-cable/connect-the-lead

Connect your Raspberry Pi to the host to power it via USB or use an external power adapter.
On the host OS, launch a serial console client program such as minicom and connect it to the
USB serial device (baudrate 115200):

    $ minicom -D /dev/ttyUSB0

You should now see the FreeNOS console output.

arm/raspberry2
--------------

Follow the same instructions as for the Raspberry Pi 1 above, but use the Raspberry Pi 2
configuration file instead to build FreeNOS:

    $ cp config/arm/raspberry2/build.conf .
    $ scons

When copying the kernel executable to the SD card '/boot' partition, use the 'kernel7.img' filename
to make sure the Raspberry Pi bootloader correctly starts the kernel (ARMv7):

    $ cp build/arm/raspberry/kernel.img /media/boot/kernel7.img

Additionally, for raspberry pi 2 and newer, add the following to the /boot/config.txt
file to ensure the first UART is available on GPIO pins 8 and 10:

    dtoverlay=pi3-miniuart-bt
    enable_uart=1

arm/sunxi-h3
------------

### U-Boot on SD Card ###

FreeNOS has support for ARM boards with Allwinner H3 System-on-chips such as the Orange Pi PC
and Orange Pi Zero (H2+ is a H3 variant). To build FreeNOS for the Allwinner H3, copy the
provided configuration file:

    $ cp config/arm/sunxi-h3/build.conf .
    $ scons

The kernel image in U-Boot format can be copied to an SD card with U-Boot installed:

    $ cp build/arm/sunxi-h3/kernel/arm/sunxi-h3/kernel.ub /media/sdcard/kernel.ub

To install U-Boot mainline on the SD-card, clone the source and select the proper
configuration for your board (Orange Pi PC: orangepi_pc_defconfig, Orange Pi Zero: orangepi_zero_defconfig):

    $ git clone https://gitlab.denx.de/u-boot/u-boot u-boot-git
    $ cd u-boot-git
    $ ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- make mrproper
    $ ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- make orangepi_pc_defconfig

To change the default configuration, enter the Kconfig interactive editor using:

    $ ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- make menuconfig

Before building, you need to select the following configuration item:

    Device Tree Control > Provider for DTB for DT Control > Embedded DTB

To build the U-Boot binary, simply use make without any arguments:

    $ ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- make

The file u-boot-sunxi-with-spl.bin is now ready to be written to the SD card:

    $ sudo dd if=u-boot-sunxi-with-spl.bin of=/dev/sdXXX bs=1024 seek=8 conv=notrunc

Insert the SD card in the target board with the UART console connected and enter the following commands
in the U-Boot interactive console to load and start FreeNOS:

    => setenv bootm_boot_mode sec
    => fatload mmc 0:1 0x400fffc0 kernel.ub
    14757888 bytes read in 670 ms (21 MiB/s)
    => bootm 0x400fffc0

You may also choose to download the FreeNOS kernel image via the network. To use a static IP address and server use:

    => setenv image kernel.ub
    => setenv image_addr 0x400fffc0
    => setenv bootm_boot_mode sec
    => setenv ipaddr 172.16.10.120
    => setenv serverip 172.16.10.1
    => tftp $image_addr $serverip:$image
    => bootm $image_addr

To retrieve an IP address using DHCP, use the 'dhcp' command prior to downloading and booting:

    => setenv bootm_boot_mode sec
    => dhcp
    => tftp 0x400fffc0 kernel.ub
    => bootm 0x400fffc0

### U-Boot on SPI Flash ###

Alternatively, the Orange Pi Zero board contains a small SPI flash which can also be used to install U-Boot.
This can be done using the Allwinner Sunxi Tools via a special FEL mode via USB. First clone and build the sunxi-tools:

    $ git clone https://github.com/linux-sunxi/sunxi-tools
    $ cd sunxi-tools

Connect your board via USB-cable to your PC and verify that FEL mode works:

    $ sunxi-fel ver

When you have build U-Boot using the previous steps, write the U-Boot binary to the flash with:

    $ ./sunxi-fel -v -p spiflash-write 0 ../u-boot/u-boot-sunxi-with-spl.bin

With this change the board will not enter FEL mode anymore. In order to re-write the SPI flash, you can erase
the flash using Armbian. Download the latest Armbian image for Orange Pi Zero at https://www.armbian.com/orange-pi-zero/.
Mount the image and edit the file /boot/armbianEnv.txt. Add the following entries to enable /dev/mtd0:

    spi-jedec-nor
    param_spinor_spi_bus=0

Start the board from the modified Armbian image and run the following commands to erase the SPI flash:

    $ sudo apt-get install mtd-utils
    $ sudo flash_erase /dev/mtd0 0 0200000

### U-Boot on Qemu/TFTP ###

An alternative for testing the boot process using U-Boot is with Qemu. You can start U-Boot via Qemu as the kernel
to be loaded using -kernel and provide tftp= argument for -netdev to enable the integrated TFTP server in Qemu.
The following commands can be used to download the FreeNOS image via TFTP and boot it:

    $ qemu-system-arm -M orangepi-pc -kernel /path/to/u-boot/u-boot -nographic \
         -net nic,id=net0 -netdev user,id=hub0port0,tftp=/path/to/FreeNOS/
    ...
    => setenv bootm_boot_mode sec
    => dhcp
    => tftp 0x400fffc0 build/arm/sunxi-h3/kernel/arm/sunxi-h3/kernel.ub
    => bootm 0x400fffc0

Using FreeNOS
=============

When FreeNOS starts the system will print bootup output and present the system
login prompt. Currently the login will accept any username value and does not
ask for a password. The FreeNOS interactive console prompt looks like the following:

    (localhost) / #

FreeNOS has a UNIX-like interface and you may enter any of the commands available
in the /bin directory:

    (localhost) / # ls bin

For example, the 'ps' command prints a list of all processes running in the system:

    (localhost) / # ps

You can also run the fully automatic autotester inside Qemu or on real hardware
using the following command:

    (localhost) / # /test/run

To view some information about the FreeNOS version and hardware settings
you can use the 'sysinfo' command:

    (localhost) / # sysinfo

The '/' in the prompt indicates the current active directory. Change it with the 'cd'
built-in shell command:

    (localhost) / # cd /tmp
    (localhost) /tmp #

The FreeNOS shell has several built-in commands, use the 'help' command
to view all the built-in shell commands:

    (localhost) / # help

FreeNOS on Allwinner H2+/H3 boards support networking. You can see the current state
of the network stack with:

    (localhost) / # netctl

To assign a static IP address you can write the IPV4 address file in the corresponding device:

    (localhost) / # write /network/sun8i/ipv4/address 172.16.10.90

To obtain an address via DHCP, you can start the DHCP client with:

    (localhost) / # dhcpc sun8i

When the device has a valid IPV4 address you can send out an ICMP ping using:

    (localhost) / # netping -i sun8i 172.16.10.1

Example application program for calculating prime numbers is the /bin/prime command.
To let it compute all prime numbers up to 1024 and output the prime number results use:

    (localhost) / # prime --stdout 1024

The prime command also has a multicore capable variant variant called 'mpiprime' which uses
the Message Passing Interface (MPI) library. The mpiprime program can compute the prime numbers
in parallel using multiple cores. To run it via the console and let the shell measure the time taken use:

    (localhost) / # time mpiprime 2000000

You can compare the time result versus the time take of the single core program
where it computes the same number of primes:

    (localhost) / # time prime 2000000

Additionally, it is possible on the Allwinner H2+/H3 (arm/sunxi-h3) target to start MPI programs
via the network on multiple nodes running FreeNOS. You can do that by starting the corresponding
MPI program which is compiled on your host OS and uses the MPI library host code to communicate with
the remote nodes via ethernet. You need to provide a configuration file that specifies the list of
IP addresses and core identifiers. For example, see the file config/host/mpihosts.txt which is configured
to use the local Qemu program for testing. Start the QEMU instance with MPI enabled using:

    $ scons qemu_mpi

In another terminate, start the MPI ping test program that communicates with the Qemu instance with:

    $ ./build/host/bin/mpiping/mpiping ./config/host/mpihosts.txt

You can also start the MPI variant of the prime program via the network with:

    $ ./build/host/bin/mpiprime/mpiprime ./config/host/mpihosts.txt 3000000

Provide your own configuration file with an arbitrary list of IP addresses to run
MPI programs on your own compute cluster.

Jenkins Continuous Integration
==============================

Automated with Vagrant
----------------------
The installation and configuration of continuous integration for FreeNOS is fully automated
using Vagrant (https://www.vagrantup.com/). Vagrant is an open source program which automates
the creation and configuration of virtual machines of various types of backends, for example
Virtual Box and libvirt / KVM. FreeNOS provides a few script files which can be used by Vagrant
to create the Jenkins master and slave nodes automatically, configure them and start build jobs.

Install Vagrant from the official website at https://www.vagrantup.com/ or via your OS package
manager. For example, on Ubuntu Linux:

    $ sudo apt-get install vagrant

Vagrant must have a backend virtual machine hypervisor to run the actual VM's. This can be
done using any of the supported backend, for example VirtualBox or libvirt/KVM. For full details
on how to setup Vagrant for your VM backend, please visit: https://www.vagrantup.com/docs/installation/.

To install and use libvirt / KVM using Vagrant on Ubuntu Linux, first ensure that hardware virtualization
extensions for your processor is enabled in the BIOS of your computer. After that, use the following
commands to install libvirt, KVM and Vagrant libvirt support:

    $ sudo apt-get install vagrant-libvirt libvirt-bin libvirt-dev qemu-kvm qemu-utils qemu

On Ubuntu 20.04:

    $ sudo apt-get install libvirt-clients libvirt-daemon-system qemu-kvm vagrant-libvirt

Add yourself to the libvirt usergroup in order to use the libvirt installation:

    $ sudo usermod -a -G libvirt my_userid

Test if libvirt with KVM is working properly:

    $ virsh list

If you do not get any errors, libvirt with KVM should be working.

To bring up the master machine, install it and start jenkins, use:

    $ cd /path/to/FreeNOS
    $ cd support/jenkins
    $ vagrant up master

After installation completes, open your webbrowser at http://localhost:8888/ to use Jenkins.
The default username and password are: admin, admin.

To bring up the Ubuntu slave use:

    $ vagrant up ubuntu1804

Similarly, bring up the FreeBSD 12.0 slave with:

    $ vagrant up freebsd12

When you wish to automatically bring up all the machines, install and configure them and also
automatically run the jobs, simply use the following command. Note that this will consume
lots of CPU and RAM:

    $ vagrant up

After making changes to the FreeNOS code, it is possible to re-run the jenkins jobs by
provisioning the slaves again with:

    $ vagrant provision freebsd12
    $ vagrant provision ubuntu1804

This will ensure the slaves are fully updated to the latest OS and compilers and runs
the Jenkins jobs for all available configurations.

Note for windows users with Vagrant: do not set core.autocrlf to true in git, as otherwise the
source files will get \r\n characters added, leading to errors in the bash scripts.

Jenkins Master (Manual Install)
-------------------------------

The following sections describe how to install Jenkins manually on your host OS for continuous
integration of FreeNOS. Install Jenkins on your host OS using your favorite package manager or
from the official website (https://jenkins.io/). Follow the installation wizard instructions and
after installation go to the Jenkins web interface at: http://localhost:8080

After installation, navigate to: Manage Jenkins > Manage Plugins
Make sure the following plugins are installed. Choose the plugins from the 'Available' tab to find the plugins
which are not yet installed:

- Matrix Project Plugin (https://wiki.jenkins-ci.org/display/JENKINS/Matrix+Project+Plugin)
- Matrix Combinations Plugin (https://wiki.jenkins-ci.org/display/JENKINS/matrix+combinations+plugin)
- Node and Label Parameter Plugin (https://wiki.jenkins-ci.org/display/JENKINS/NodeLabel+Parameter+Plugin)
- Libvirt Slaves Plugin (https://wiki.jenkins.io/display/JENKINS/Libvirt+Slaves+Plugin)
- SSH Plugin (https://wiki.jenkins-ci.org/display/JENKINS/SSH+plugin)
- Workspace Cleanup Plugin (https://wiki.jenkins-ci.org/display/JENKINS/Workspace+Cleanup+Plugin)

After finishing Jenkins configuration, install KVM and Libvirt on your host OS. For Ubuntu use:

    $ sudo apt-get install qemu-kvm libvirt-bin virtinst

Connect Jenkins to libvirt via SSH, navigate to: Manage Jenkins > Configure System
Choose 'Add a new cloud' and select 'Hypervisor Type': QEMU,
'Hypervisor Host': localhost and choose your desired SSH username.

Press 'Advanced' to choose authentication parameters and press 'Test Connection' to
verify Jenkins can access libvirt via SSH. For Ubuntu, you need to ensure the SSH user
is in the 'kvm' and 'libvirtd' groups:

    $ sudo usermod -a -G kvm,libvirtd jenkins

Jenkins need to find the KVM guests by their hostname. You can either manually configure
the DNS hostname to IP translation in the /etc/hosts file or automatically using the
internal libvirt DNSMasq server.

To setup automatic DNS translation in KVM, first edit the 'default' libvirt network interface
to add the '.kvm' local-only domain:

    $ virsh net-edit default

Add the following line inside the <network>....</network> tag:

    <domain name='kvm' localOnly='yes'/>

To use the libvirt internal DNSMasq server on your host, you may change the /etc/resolv.conf
file to add the following:

    nameserver 192.168.122.1

If your host OS uses NetworkManager for networking, add the following files:

    $ sudo -s
    # cat > /etc/NetworkManager/conf.d/localdns.conf
    [main]
    dns=dnsmasq

    # cat > /etc/NetworkManager/dnsmasq.d/libvirt_dnsmasq.conf
    server=/kvm/192.168.122.1

Ensure that all KVM guests have their hostname set in the .kvm domain (e.g. someguest.kvm).
Restart libvirt and optionally NetworkManager to apply the changes:

    $ sudo /etc/init.d/libvirt-bin restart
    $ sudo /etc/init.d/NetworkManager restart

You should now be able to resolve the hostnames of your guests in the .kvm domain after they
are started, for example:

    $ virsh start ubuntu-1804
    $ host ubuntu-1804.kvm

Also visit the following page for more details on this automatic DNS setup for KVM:
https://liquidat.wordpress.com/2017/03/03/howto-automated-dns-resolution-for-kvmlibvirt-guests-with-a-local-domain/

FreeBSD 12.0 Slave (Manual Install)
-----------------------------------

Run the example installation script in ./support/jenkins/freebsd-12.sh from the FreeNOS sources
to setup the KVM guest with FreeBSD 12.0. Also see the comments in the installation script for more info:

    jenkins@host$ cd support/jenkins
    jenkins@host$ ./freebsd-12.sh

To configure the FreeBSD 12.0 slave in Jenkins for building FreeNOS, you first need to
make sure the Jenkins user can login to the slave using SSH. Optionally, you can
configure this with public key authentication:

    jenkins@host$ virsh start freebsd-12
    jenkins@host$ ssh-keygen
    jenkins@host$ ssh-copy-id jenkins@freebsd-12.kvm

Test if the Jenkins user can login to the FreeBSD KVM guest with SSH (via password or key):

    jenkins@host$ ssh jenkins@freebsd-12.kvm

Copy the example node configuration XML file to the Jenkins installation directory:

    jenkins@host$ mkdir /var/lib/jenkins/nodes/freebsd-12-test
    jenkins@host$ cp freebsd-12.node.xml /var/lib/jenkins/nodes/freebsd-12-test/config.xml

Also copy the job configuration XML file to the Jenkins installation directory:

    jenkins@host$ mkdir /var/lib/jenkins/jobs/FreeNOS-freebsd12-test
    jenkins@host$ cp freebsd-12.job.xml /var/lib/jenkins/jobs/FreeNOS-freebsd12-test/config.xml

Restart the Jenkins server to use the new configuration files:

    $ sudo /etc/init.d/jenkins restart

Jenkins still needs authentication credentials to connect to the slave. Navigate to:

    Manage Jenkins > Manage Nodes > freebsd-12 > Configure

In the sub-menu 'Secondary launch method', under 'Launch agents via SSH',
select existing SSH credentials to let jenkins use it to login to the slave node
or add a new credentials with the 'Add' button. If you generated an SSH key for
the jenkins user on the host OS, insert the SSH private key or provide username/password.

Press the 'Save' button to finish and use 'Launch Agent' to test the connection.

You can now press the 'Schedule a Build' button on the 'FreeNOS-freebsd12' build job to test.

Ubuntu 18.04 Slave
------------------

Follow the same instructions as the FreeBSD 12.0 Slave above and just
replace the slave name with 'ubuntu-1804'.

Authors
=======

The FreeNOS code has been written from scratch by Niek Linnenbank.
People who contributed to FreeNOS are:

* Dan Rulos (AmayaOS)
* Coen Bijlsma (libparse, srv/time)
* Ed Schouten (libteken)
* Alexander Schrijver (OpenBSD patches)
