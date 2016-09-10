#!/bin/sh

file_list="${HOME}/.config/jgmenu/default.csv"

menu_file=
for f in ${file_list}
do
	if test -e ${f}
	then
		menu_file=${f}
		break
	fi
done

if ! test -z ${menu_file}
then
	cat ${menu_file} | jgmenu
	exit 0
else
	printf "fatal: cannot find .csv file\n"
	exit 1
fi
