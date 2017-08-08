# Copyrigh 2017 NXP
#!/bin/sh
# setup IOT setting for LS1012A
source /etc/profile

apachectl start
/usr/bin/iot_zb &
Thread_KW_Tun /dev/ttySC1 fslthr0 0 25 9600 &

/usr/bin/node /root/node-red/red.js &> /root/log/node.og &
/root/Alexa/bootalexa.sh &
/root/Alexa/bootcontroller.sh &
sleep 1
/root/Alexa/bootaws.sh &
