#!/bin/sh

# Create a new TUN interface for Thread interaction.
ip -6 tuntap add mode tun fslthr0
# Assign it a global IPv6 address.
ip -6 addr add FD01::2 dev fslthr0
# Add route to default address of Serial TUN embedded interface.
ip -6 route add FD01::1 dev fslthr0
# Add route to Unique Local /64 Prefix via fslthr0.
ip -6 route add FD01:0000:0000:3EAD::/64 dev fslthr0
# The interface is ready.
ip link set fslthr0 up

# Enable IPv6 routing on host.
sysctl -w net.ipv6.conf.all.forwarding=1
