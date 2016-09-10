#!/bin/bash

IFS="$(printf '\n\t')"


die () {
	printf "fatal: %s\n" "$1"
	exit 1
}

usage () {
	echo "usage: jgmenu_run cache --menu-file=<file> [--theme=<theme>]"
	echo "                        [--icon-size=<size>]"
	echo ""
	echo "Creates symlinks for icons based on jgmenu csv file."
	echo ""
}

create_symlinks () {
	if ! test -e ${menu_file}
	then
		die "${menu_file} does not exist"
	fi

	mkdir -pv ${cache_dir}
	test -d ${cache_dir} || die "could not create cache directory"
	test -w ${cache_dir} || die "you do not have write permission to the cache directory"

	for f in $(cat "${menu_file}" | awk -F',' '{print $3}')
	do
		if test -e  ${cache_dir}/${f%.*}.png || \
		   test -e  ${cache_dir}/${f%.*}.svg || \
		   test -e  ${cache_dir}/${f%.*}.xpm
		then
			echo "[OVERWRITE] ${f}"
			rm -f ${cache_dir}/${f}.*
		else
			echo "[ CREATE  ] ${f}"
		fi

		ln -s "$(${find_cmd} --theme=${icon_theme} --icon-size=${icon_size} \
		       ${f})" ${cache_dir} 2>/dev/null
	done
}


#
# Script starts here
#

#
# Set default values
#
icon_size=22
icon_theme=Adwaita
menu_file=

find_cmd="./jgmenu-icon-find"
test -x ${find_cmd} || find_cmd="jgmenu-icon-find"
type ${find_cmd} &>/dev/null ||	die "jgmenu-icon-find not installed"

#
# Command line options
#
while test $# != 0
do
	case "$1" in
	-v|--verbose)
		verbose=t ;;
	--icon-size=*)
		icon_size="${1#--icon-size=}" ;;
	--theme=*)
		icon_theme="${1#--theme=}" ;;
	--menu-file=*)
		menu_file="${1#--menu-file=}" ;;
	--help)
		usage
		exit 0
		;;
	*)
		printf "error: unknown option: \`%s\'\n" $1
		usage
		exit 1
		;;
	esac
	shift
done

if test -z ${menu_file}
then
	echo "error: menu file not specified"
	usage
	exit 1
fi

cache_name=jgmenu-cache
cache_dir=~/.local/share/icons/${cache_name}/${icon_size}

create_symlinks

# TODO: Create index.theme with Inherits=

