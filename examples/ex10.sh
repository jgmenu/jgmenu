#!/bin/sh

tint2dir="/usr/share/tint2"

set | grep TINT && { echo "run from shell without TINT environment variables"; exit 1; }
test -d ${tint2dir} ||  { echo "cannot find directory with tint2rc files"; exit 1; }

cd ${tint2dir}

for f in ./*.tint2rc
do
	printf "%b\n" "Testing file ${f}. Press F10 for next menu."
	killall tint2 2>/dev/null
	killall jgmenu 2>/dev/null
	nohup tint2 -c ${f} 2>/dev/null &
	TINT2_CONFIG=${f} jgmenu --csv-cmd="jgmenu_run parse-pmenu" --config-file=i_dont_exist
done

nohup tint2 &
