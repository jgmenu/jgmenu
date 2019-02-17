#!/bin/sh

execpath=$(jgmenu_run --exec-path)
cycles=10

if ! type bc >/dev/null 2>&1
then
	printf "fatal: need 'bc' for this test\n"
	exit 1
fi

printf "%b\n" "Running tests ${cycles} times..."

start=$(date +%s.%N)
for i in $(seq "${cycles}"); do
	${execpath}/jgmenu-pmenu.py >/dev/null 2>&1
done
dur_pmenu=$(printf "%b\n" "$(date +%s.%N) - $start" | bc)

start=$(date +%s.%N)
for i in $(seq "${cycles}"); do
	${execpath}/jgmenu-lx >/dev/null 2>&1
done
dur_lx=$(printf "%b\n" "$(date +%s.%N) - $start" | bc)

start=$(date +%s.%N) 2>&1
dur_nothing=$(printf "%b\n" "$(date +%s.%N) - $start" | bc)

printf "pmenu:%.3fs;   " ${dur_pmenu}
printf "lx:%.3fs;   " ${dur_lx}
printf "nothing:%.3fs\n" ${dur_nothing}

