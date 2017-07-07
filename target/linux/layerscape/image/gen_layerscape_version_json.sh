#!/usr/bin/env bash

write_json(){
	sed -i "/$2/ s/\(.*:\).*/\1$3/" $1
}

write_json $1 "boardname" "\"$2\""

