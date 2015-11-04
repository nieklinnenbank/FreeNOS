#
# Use serial port as console.
#
/server/serial/server
stdio /dev/serial0 /dev/serial0

#
# System Servers and Drivers.
#
# /server/i2c/grovepi/server

#
# Serial Console
#
/bin/login /dev/serial0 /dev/serial0
