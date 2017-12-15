#!/bin/sh

if ! type bc >/dev/null
then
	printf "fatal: need 'bc' for this test\n"
	exit 1
fi

start=$(date +%s.%N)
jgmenu_run pmenu >/dev/null
dur1=$(printf "%b\n" "$(date +%s.%N) - $start" | bc)

start=$(date +%s.%N)
jgmenu_run xdg >/dev/null
dur2=$(printf "%b\n" "$(date +%s.%N) - $start" | bc)

start=$(date +%s.%N)
dur3=$(printf "%b\n" "$(date +%s.%N) - $start" | bc)

printf "pmenu:%.3fs;   " ${dur1}
printf "xdg:%.3fs;   " ${dur2}
printf "nothing:%.3fs\n" ${dur3}

