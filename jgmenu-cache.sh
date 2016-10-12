#!/bin/sh

IFS="$(printf '\n\t')"


die () {
	printf "fatal: %s\n" "$1"
	exit 1
}

check_prog_exists () {
	type "$1" >/dev/null 2>&1 || die "$1 is not installed"
}

usage () {
	echo "usage: jgmenu_run cache [<options>]"
	echo ""
	echo "    --verbose"
	echo "    --menu-file=<file>"
	echo "    --theme=<theme>"
	echo "    --icon-size=<size>"
	echo ""
}

delete_cache () {
	test -z ${cache_dir} && die "no cache dir specified"
	rm -rf ${cache_dir}
}

create_menu_file () {
	menu_file=$(mktemp)
	jgmenu_run parse-pmenu >> ${menu_file}
	cat ~/.config/jgmenu/default.csv >> ${menu_file}
}

get_gtk3_theme () {
	tmp_file=$(mktemp)
	cat ~/.config/gtk-3.0/settings.ini \
	    | grep -i 'gtk-icon-theme-name' \
	    | awk -F= '{ print $2 }' \
	    | tr -d ' ' \
	    > ${tmp_file}

	icon_theme=$(cat ${tmp_file})
	rm -f ${tmp_file}
}

get_icon_theme () {
	if icon_theme=$(jgmenu_run xsettings --icon-theme)
	then
		printf "info: obtained '${icon_theme}' from xsettings\n"
		return
	fi

	icon_theme=$(jgmenu_run config --get icon_theme)
	if ! test -z ${icon_theme}
	then
		printf "info: obtained '${icon_theme}' from jgmenurc\n"
		return
	fi

	get_gtk3_theme
	if ! test -z ${icon_theme}
	then
		printf "info: obtained '${icon_theme}' from GTK's settings.ini\n"
		return
	fi

	icon_theme=Adwaita
	printf "info: using '${icon_theme}' by default\n"
}

get_icon_size () {
	icon_size=$(jgmenu_run config --get icon_size)
	if ! test -z ${icon_size}
	then
		printf "info: obtained '${icon_size}' from jgmenurc\n"
		return
	fi

	icon_size=22
	printf "info: using '${icon_size}' by default\n"
}

#
# $1 - icon directory (e.g. ~/.local/share/icons/jgmenu-cache/22)
#
create_symlinks () {
	if ! test -e ${menu_file}
	then
		die "${menu_file} does not exist"
	fi

	mkdir -p ${1} 2>/dev/null
	test -d ${1} || die "could not create cache directory"
	test -w ${1} || die "you do not have write permission to the cache directory"

	for f in $(cat "${menu_file}" | awk -F',' '{print $3}')
	do
		if test -e  ${1}/${f%.*}.png || \
		   test -e  ${1}/${f%.*}.svg || \
		   test -e  ${1}/${f%.*}.xpm
		then
			test ${verbose} = "t" && echo "[ EXISTS  ] ${f}"
		#	rm -f ${1}/${f}.*
		else
			test ${verbose} = "t" && echo "[ CREATE  ] ${f}"
			ln -s "$(jgmenu-icon-find --theme=${icon_theme} \
				 --icon-size=${icon_size} ${f})" \
			      ${1} >/dev/null 2>&1
		fi
	done
}


#
# Script starts here
#
# Global variables
#
icon_size=
icon_theme=
menu_file=
verbose=f
cache_name=jgmenu-cache
cache_dir=~/.local/share/icons/${cache_name}

check_prog_exists jgmenu_run
check_prog_exists jgmenu-parse-xdg
check_prog_exists jgmenu-icon-find
check_prog_exists jgmenu-config.sh
check_prog_exists jgmenu-xsettings

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
	--delete)
		delete_cache
		exit 0
		;;
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

test -z ${menu_file} && create_menu_file
test -z ${icon_theme} && get_icon_theme
test -z ${icon_size} && get_icon_size

if test ${icon_theme} = "jgmenu-cache"
then
	die "icon_theme cannot be called 'jgmenu-cache'"
fi

printf "Deleting old icon-cache...\n"
delete_cache

printf "Creating new icon-cache...\n"
create_symlinks ${cache_dir}/${icon_size}

echo "Inherits=${icon_theme}" >${cache_dir}/index.theme

