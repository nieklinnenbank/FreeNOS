
#
# VGA/keyboard console
#
/server/input/keyboard/server
/server/video/vga/server
/server/terminal/server
stdio /dev/tty0 /dev/tty0

#
# System Servers and Drivers.
#
/server/time/server

#
# Login prompt
#
/bin/login /dev/tty0 /dev/tty0

#
# Serial console
#
/server/serial/server
/bin/login /dev/serial0 /dev/serial0
