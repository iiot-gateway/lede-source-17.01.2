#!/bin/sh
# ------------------------------------------------------------------
# Author:    nlv10677
# Copyright: NXP B.V. 2014. All rights reserved
# ------------------------------------------------------------------

./iot_zb &
sleep 1
./iot_sj &
sleep 1
./iot_ci &
sleep 1
./iot_nd &
sleep 1
./iot_gd &
sleep 1
./iot_dbp &
