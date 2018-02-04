#!/bin/sh

while :
do
	clear
	ps aux \
		| grep jgmenu \
		| grep -v "ps_jgmenu\|grep jgmenu" \
		| awk '{ print $2, $11, $12 }'
	sleep 1
done
