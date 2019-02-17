#!/bin/sh

menu_file=

ask_user_for_input () {
	tmp_file=$(mktemp)
	cd ../tests/t0006/menus
	for i in ./*
	do
		echo "${i#./},${i#./}" >> ${tmp_file}
	done
	menu_file=$(cat ${tmp_file} | jgmenu --no-spawn --config-file= --simple 2>/dev/null)
	cd ../..
	rm -f ${tmp_file}
}

ask_user_for_input
export XDG_MENU_PREFIX="${menu_file%applications\.menu}"
printf "XDG_MENU_PREFIX=%b\n" "${XDG_MENU_PREFIX}"
export XDG_CONFIG_DIRS="a:b:c:::d:$PWD/../tests/t0006:e:f"
../jgmenu --csv-cmd="$PWD/../jgmenu-lx" --simple --config-file=
