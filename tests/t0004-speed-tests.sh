#!/bin/bash

tmp_file=$(mktemp)
N_ITEMS=100

printf "Creating temporary file with ${N_ITEMS} items...\n"

for i in $(seq "${N_ITEMS}")
do
#	echo "foo ${i},bar${i}" >> ${tmp_file}
#	echo "foo ${i},bar${i},/usr/share/icons/Adwaita/22x22/apps/utilities-terminal.png" >> ${tmp_file}
	echo "foo ${i},bar${i},/usr/share/icons/hicolor/scalable/apps/leafpad.svg" >> ${tmp_file}
done

printf "Press key to run jgmenu\n"
read a

cat ${tmp_file} | ../jgmenu --icon-size=48 --config-file=

rm ${tmp_file}
