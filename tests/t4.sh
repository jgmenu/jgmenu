#!/bin/bash

tmp_file=$(mktemp)
N_ITEMS=100000

printf "Creating temporary file with ${N_ITEMS} items...\n"

for i in $(seq "${N_ITEMS}")
do
	echo "foo ${i},bar${i}" >> ${tmp_file}
done

printf "Press key to run jgmenu\n"
read a

cat ${tmp_file} | ../jgmenu

rm ${tmp_file}
