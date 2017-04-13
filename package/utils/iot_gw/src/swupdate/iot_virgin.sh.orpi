#!/bin/sh
# ------------------------------------------------------------------
# SW Update shell script vor virgin IoT Gateways
# - It assumes that the cloud image is already on /tmp
# ------------------------------------------------------------------
# Author:    nlv10677
# Copyright: NXP B.V. 2014. All rights reserved
# ------------------------------------------------------------------

IMAGE=iot_image.tgz

# UNZIP new SW image, prepare permissions, copy and remove
echo "Unpacking image $IMAGE"
tar -xzf /tmp/$IMAGE -C /tmp
echo "Set permissions right"
chown -R root:root /tmp/images
chmod -R +r /tmp/images/tmp
chmod -R +r /tmp/images/www
chmod -R +r /tmp/images/etc/*
chmod +x /tmp/images/usr/bin/*
chmod +x /tmp/images/usr/bin/killbyname
echo "Copy to root"
cp -pR /tmp/images/* /
rm -R /tmp/images

# Init as usual (verbose)
/usr/bin/iot_init.sh

# and finally reboot
echo "Ready to reboot"
#reboot

