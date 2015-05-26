#
# Uncomment to use serial port as console.
#
#/server/serial/server
#stdio /dev/serial0 /dev/serial0

#
# Use keyboard and vga text-mode as console.
#
/server/input/keyboard/server
/server/video/vga/server
/server/terminal/server
stdio /dev/tty0 /dev/tty0

#
# System Servers and Drivers.
#
/server/time/server
#/server/filesystem/proc/server
#/server/ata/server
#/server/filesystem/linn/server /dev/ata0 0x400000 /usr
#/server/pci/server
#/server/pci/detect

#
# Interactive Shell.
#
/bin/sh
