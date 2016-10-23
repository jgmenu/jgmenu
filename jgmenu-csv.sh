#!/bin/sh

default_menu_file="${HOME}/.config/jgmenu/default.csv"
menu_file=""

usage () {
	printf "%s\n" "Usage: jgmenu_run csv [<options>] [<menu-file>]"
}

check_file_exists () {
	if ! test -e "$1"
	then
		printf "%s\n" "fatal: file '$1' does not exist"
		exit 1
	fi
}

#
# START OF SCRIPT
#

add_pmenu=f
add_xdg=f

while test $# != 0
do
	
	case "$1" in
	--add-pmenu)
		add_pmenu=t ;;
	--add-xdg)
		add_xdg=t ;;
	--help)
		usage
		exit 0
		;;
	-*)
		printf "error: unknown option: \`%s\'\n" $1
		usage
		exit 1
		;;
	*)
		menu_file="$1" ;;
	esac
	shift
done

if test -z ${menu_file}
then
	menu_file=${default_menu_file}
fi

check_file_exists "${menu_file}"

if test ${add_pmenu} = "t" || test ${add_xdg} = "t"
then
	tmp_file=$(mktemp)
	cat ${menu_file} > ${tmp_file}
	test ${add_pmenu} = "t" && jgmenu-parse-pmenu.py >> ${tmp_file}

	# TODO: Use jgmenu_run xdg here for clever selection of .menu file
	xdg_cmd="jgmenu-parse-xdg /etc/xdg/menus/gnome-applications.menu"
	test ${add_xdg} = "t" &&  ${xdg_cmd} >> ${tmp_file}

	cat ${tmp_file} | jgmenu
else	
	cat ${menu_file} | jgmenu
fi

