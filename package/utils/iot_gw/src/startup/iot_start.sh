#!/bin/sh
# ------------------------------------------------------------------
# Author:    nlv10677
# Copyright: NXP B.V. 2014. All rights reserved
# ------------------------------------------------------------------

/etc/init.d/iot_zb_initd  start > /dev/null
/etc/init.d/iot_sj_initd  start > /dev/null
/etc/init.d/iot_ci_initd  start > /dev/null
/etc/init.d/iot_nd_initd  start > /dev/null
/etc/init.d/iot_gd_initd  start > /dev/null
/etc/init.d/iot_dbp_initd start > /dev/null
/etc/init.d/iot_su_initd  start > /dev/null
