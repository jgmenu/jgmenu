#!/bin/sh

tmp_file=$(mktemp)
n_items=5000

for i in $(seq "${n_items}")
do
	echo "foo ${i},bar${i},jabber" >> ${tmp_file}
done

printf "%b\n" "$0: speed test of ${n_items} items"

start=$(date +%s.%N)

../jgmenu --csv-file="${tmp_file}" \
	--icon-size=0 \
	--vsimple \
	--die-when-loaded 2>/dev/null

dur=$(printf "%b\n" "$(date +%s.%N) - $start" | bc)
printf "Duration=%.3fs\n" "${dur}"

rm ${tmp_file}
