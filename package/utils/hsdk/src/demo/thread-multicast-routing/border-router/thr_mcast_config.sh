#!/bin/bash

if [ "$EUID" -ne 0 ]; then
    echo "Please run as root"
    exit
fi

echo "[WARNING] >> This script will overwrite /etc/smcroute.conf and /etc/smcroute.conf <<"
echo "[WARNING] >> Existing files, if any, will have the suffix .bkp <<"
echo "[WARNING] >> At the second run, the .bkp files will get overwritten too. <<"
echo

# read interface to traffic initiator
read -p "Please enter the interface to the traffic initiator: " iface
sed -i -e "s/interface.*/interface "$iface"/g" etc/radvd.conf
sed -i -e "s/phyint .* enable/phyint "$iface" enable/g" etc/smcroute.conf
sed -i -e "s/mgroup .* ff03::3ead/mgroup "$iface" ff03::3ead/g" etc/smcroute.conf
sed -i -e "s/mroute from .* group/mroute from "$iface" group/g" etc/smcroute.conf

# read source address
echo "If this is the first time you run this script, the traffic initiator will not yet have an IPv6 address from range fd01:ba55::/64."
echo "You may enter anything here, but please run again this script after addresses are properly set."
read -p "Please enter the temporary IPv6 address of the traffic initiator: fd01:ba55::" addr
sed -i -e "s/fd01:ba55::.* to/fd01:ba55::"$addr" to/g" etc/smcroute.conf

# stop daemons
if pidof systemd > /dev/null; then
    systemctl stop radvd
else
    /etc/init.d/radvd stop
fi
smcroute -k

# update configuration files
mv /etc/radvd.conf /etc/radvd.conf.bkp
mv /etc/smcroute.conf /etc/smcroute.conf.bkp
cp etc/radvd.conf /etc/
cp etc/smcroute.conf /etc/

# forwarding is required for radvd
sysctl -w net.ipv6.conf.all.forwarding=1
if pidof systemd > /dev/null; then
    systemctl start radvd
else
    /etc/init.d/radvd start
fi

# check /var/log/syslog for smcroute debug messages
smcroute -d -L debug
sleep 1
# check status of mc_forwarding
sysctl net.ipv6.conf.all.mc_forwarding

# Latest radvd FAQ 2
# How do I set up the router running radvd to automatically
# configure the interfaces to use an EUI64-based address?

# -- You don't.  The design philosophy of radvd is that it's
# not the _router's_ configuration tool, but a route advertising
# daemon.  You'll need to set up all the addresses, routes, etc.
# yourself.  These tasks are something that system initscripts
# could possibly do instead.
ip -6 addr add fd01:ba55::1 dev $iface
ip -6 route add fd01:ba55::/64 dev $iface
