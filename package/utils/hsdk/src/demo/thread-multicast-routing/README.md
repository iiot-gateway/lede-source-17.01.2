# border-router

## smcroute install

```bash
# git clone https://github.com/troglobit/smcroute.git
# cd smcroute/

// Get latest tag name
# latestTag=$(git describe --tags $(git rev-list --tags --max-count=1))

// Checkout latest tag. AoT latest is 2.1.1
# git checkout $latestTag

# ./autogen.sh
# ./configure (for version 2.1.1, IPv6 is enabled by default)
# ./configure --enable-ipv6 (for version <= 2.1.0)
# make; make install
```

## smcroute TODO which impacts our scenario

Add IPv6 support for (*, G), i.e., on-demand, routing rules.
In SMCRoute 1.99.0 IPv4 support for source-less multicast routes was
added. Adding support for IPv6 should be fairly straight forward, but
needs figuring out the kernel interface and extensive testing.


## dev

```bash
root@imx6ulevk:~/host_sdk/hsdk/demo/thread-multicast-routing/border-router# ./thr_mcast_config.sh
root@imx6ulevk:~/host_sdk/hsdk/demo/thread-multicast-routing/border-router# ./thr_mcast_config.sh (this time the initiator will have an IPv6 address)

root@imx6ulevk:~/host_sdk/hsdk/demo# ./make_tun.sh
# Define DEBUG = 1 in Thread_KW_Tun.c
root@imx6ulevk:~/host_sdk/hsdk/demo# make
root@imx6ulevk:~/host_sdk/hsdk/demo/bin# ./Thread_KW_Tun /dev/ttymxc1 fslthr0 1 15
# Join other nodes to the network and then read the **sender** section.
```


# sender

### ICMP echo
```bash
# ping6 -I eth42 -t 6 ff03::3ead (eth42 - interface to the border-router)
> ping ff03::3ead%42 (42 - ID of the interface to the border-router)
```

### CoAP
```bash
# sysctl -w net.ipv6.conf.all.accept_ra_rt_info_max_plen=64  # not important, just to establish the fd01:0:0:3ead::/64 route from RA
# pip install txthings --upgrade
# pip install service_identity --upgrade  # not important, just to get rid of the warning when starting txThings
```

Modify `/usr/local/lib/python2.7/dist-packages/txthings/coap.py` -> there are two calls to `resetUnrecognized()`, comment them and add `pass` instead; this is because of https://github.com/mwasilak/txThings/issues/16 .

```bash
# python tempGET.py
```
