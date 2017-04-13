#!/bin/sh

echo "Kill all IoT processes"

./killbyname iot_zb
./killbyname iot_sj
./killbyname iot_ci
./killbyname iot_nd
./killbyname iot_gd
./killbyname iot_dbp
./killbyname iot_su
