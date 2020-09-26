
#
# VGA/keyboard console
#
/server/ps2/server &
/bin/mount --wait=/dev/ps2

/server/video/server &
/bin/mount --wait=/dev/video

/server/terminal/server &
/bin/mount --wait=/console
stdio /console/tty0 /console/tty0

#
# System Servers and Drivers.
#
/server/time/server &
/bin/mount --wait=/dev/time

/server/filesystem/tmp/server /tmp &
/bin/mount --wait=/tmp

/server/network/loopback/server &
/bin/mount --wait=/network/loopback

#
# Serial console
#
/server/serial/server &
/bin/mount --wait=/dev/serial

# This ensures we wait until all cores
# are booted by the CoreServer.
/bin/sysinfo

#
# Login prompt
#
/bin/login /dev/serial/serial0/io /dev/serial/serial0/io &
/bin/login /console/tty0 /console/tty0
