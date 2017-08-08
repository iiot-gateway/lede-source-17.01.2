# Copyrigh 2017 NXP
#!/bin/sh
cd /root/Alexa/

while [ 1 ]
do
	ntpdate 1.cn.pool.ntp.org
	rdate -s stdtime.gov.hk
	python awshome.py
done


