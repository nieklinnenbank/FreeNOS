#
# Use serial port as console.
#
/server/serial/server &
/bin/mount --wait=/dev/serial
stdio /dev/serial/serial0/io /dev/serial/serial0/io

#
# System Servers and Drivers.
#
/server/filesystem/tmp/server /tmp &
/bin/mount --wait=/tmp

/server/network/loopback/server &
/bin/mount --wait=/network/loopback

/server/network/sun8i/server &
/bin/mount --wait=/network/sun8i

/server/mpiproxy/server sun8i &

# This ensures we wait until all cores
# are booted by the CoreServer.
sysinfo

# Try to get an IP address with DHCP
dhcpc sun8i

#
# Serial Console
#
/bin/login /dev/serial/serial0/io /dev/serial/serial0/io
