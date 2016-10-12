#!/bin/sh

menu_file=

ask_user_for_input () {
	tmp_file=$(mktemp)

	for i in ./t0006/*
	do
		echo "${i#./},${i#./}" >> ${tmp_file}
	done

	menu_file=$(cat ${tmp_file} | \
		    jgmenu --no-spawn --config-file=t0006-jgmenurc 2>/dev/null)

	rm -f ${tmp_file}
}


ask_user_for_input

if ! test -z ${menu_file}
then
	../jgmenu-xdg "${menu_file}" | ../jgmenu
fi
