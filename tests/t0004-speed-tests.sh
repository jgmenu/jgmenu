#!/bin/sh

tmp_file=$(mktemp)
N_ITEMS=1000

printf "Creating temporary file with ${N_ITEMS} items...\n"

for i in $(seq "${N_ITEMS}")
do

# NO ICON
#	echo "foo ${i},bar${i}" >> ${tmp_file}

# PNG
#	echo "foo ${i},bar${i},firefox" >> ${tmp_file}

# SVG
	echo "foo ${i},bar${i},jabber" >> ${tmp_file}
done

printf "Press key to run jgmenu\n"
read a

cat ${tmp_file} | ../jgmenu --icon-size=22 --config-file=t0004-jgmenurc

rm ${tmp_file}
