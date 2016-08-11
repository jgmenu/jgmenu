#!/bin/bash

#
# Simple, temporary helper-script to create icon-cache whilst I think about
# a sensible user-interface for jgmenu_run and jgmenu-cache.
#
# It requires jgmenurc to specify icon_theme=jgmenu-cache
#

die () {
	echo "fatal: $1"
	exit 1
}

ask_user_for_theme () {
	tmp_file=$(mktemp)

	if ! test -e ${icon_dir}
	then
		die "icon directory does not exist"
	else
		cd ${icon_dir}
	fi

	for i in ./*
	do
		echo "${i#./},${i#./}" >> ${tmp_file}
	done

	icon_theme=$(cat ${tmp_file} | jgmenu --no-spawn 2>/dev/null)

	rm -f ${tmp_file}
}

check_prog_exist () {
	type "$1" &>/dev/null || die "$1 is not installed"
}


#
# Global variables
#
icon_size=22
icon_dir=/usr/share/icons
icon_theme=
menu_file=~/.config/jgmenu/default.csv
cache_dir=~/.local/share/icons/jgmenu-cache

check_prog_exist jgmenu
check_prog_exist jgmenu-cache
check_prog_exist pmenu-jgmenu.py

ask_user_for_theme

rm -rf ${cache_dir}

jgmenu-cache --menu-file=${menu_file} --theme=${icon_theme} --icon-size=${icon_size}

menu_file=$(mktemp)
pmenu-jgmenu.py > ${menu_file}
jgmenu-cache --menu-file=${menu_file} --theme=${icon_theme} --icon-size=${icon_size}
rm -f ${menu_file}

echo "Inherits=${icon_theme}" > ${cache_dir}/index.theme

