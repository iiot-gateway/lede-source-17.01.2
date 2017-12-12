#!/usr/bin/env bash

write_json(){
	sed -i "/$2/ s/\(.*:\).*/\1$3/" $1
}

bdname=$2
if [[ $bdname =~ ls1021aiot ]];then
         bdname=ls1021aiot
fi
write_json $1 "boardname" "\"$bdname\""

