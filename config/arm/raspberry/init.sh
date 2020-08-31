#
# Use serial port as console.
#
/server/serial/server &
/bin/mount --wait=/dev/serial
stdio /dev/serial/serial0/io /dev/serial/serial0/io

#
# System Servers and Drivers.
#
#/server/usb/controller/server &
#/server/usb/hub/server &
/server/filesystem/tmp/server /tmp &
/server/network/loopback/server &

# This ensures we wait until all cores
# are booted by the CoreServer.
sysinfo

#
# Serial Console
#
/bin/login /dev/serial/serial0/io /dev/serial/serial0/io
