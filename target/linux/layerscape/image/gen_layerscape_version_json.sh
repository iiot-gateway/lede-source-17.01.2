#!/usr/bin/env bash

write_json(){
	sed -i "/$2/ s/\(.*:\).*/\1$3/" $1
}

write_json ./backup/version.json "boardname" "\"$1\""

