#!/bin/sh

default_menu_file="${HOME}/.config/jgmenu/default.csv"
menu_file=""

usage () {
	printf "%b\n" "Usage: jgmenu_run csv [<menu-file>]"
	printf "%b\n" "Generate menu based on a CSV file"
	printf "%b\n" "By default, '~/.config/jgmenu/default.csv' will be used"
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

while test $# != 0
do
	case "$1" in
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

cat "${menu_file}" | jgmenu
