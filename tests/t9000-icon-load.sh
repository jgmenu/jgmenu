#!/bin/sh

#
# We don't measure the time to parse the XDG menu, but just the
# start-up time with and without icons.
#

tmp=$(mktemp)

jgmenu_run pmenu >${tmp}

start=$(date +%s.%N)
cat ${tmp} | jgmenu --die-when-loaded
dur1=$(printf "%b\n" "$(date +%s.%N) - $start" | bc)

start=$(date +%s.%N)
cat ${tmp} | jgmenu --icon-size=0 --die-when-loaded
dur2=$(echo "$(date +%s.%N) - ${start}" | bc)

printf "With icons:%.3fs;   " ${dur1}
printf "Without icons:%.3fs\n" ${dur2}

rm -f ${tmp}
