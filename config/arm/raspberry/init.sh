#
# Use serial port as console.
#
/server/serial/server &
stdio /dev/serial/serial0/io /dev/serial/serial0/io

#
# System Servers and Drivers.
#
#/server/i2c/grovepi/server
/server/usb/controller/server &
/server/usb/hub/server &
/server/filesystem/tmp/server /tmp &

#
# Serial Console
#
/bin/login /dev/serial/serial0/io /dev/serial/serial0/io
