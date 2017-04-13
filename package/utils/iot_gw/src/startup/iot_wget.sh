#!/bin/sh

# First try WGET directly

echo "Wget 1, $*" >> /tmp/su.log

/usr/bin/wget -q $*
err=$?

echo "Wget 2, $err" >> /tmp/su.log

if [ ! $err = 0 ]; then
	# If it fails, then try it via the NXP fire wall

	echo "Wget 2" >> /tmp/su.log

	export http_proxy="http://nww.nics.nxp.com:8080"
	/usr/bin/wget -q $*
	err=$?
fi

echo "Wget 3, $err" >> /tmp/su.log

