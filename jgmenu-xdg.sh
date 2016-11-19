#!/bin/sh

get_extension () {
	i="${1}"
	extension=${i##*.}

	if test ${extension} != "menu"
	then
		printf "warning: ${i} does not have a .menu extension\n"
	fi
}

#
# Set $XDG_MENU_PREFIX if you have several .menu files on your system
# For example $XDG_MENU_PREFIX=lxde- will load lxde-applications.menu
#

if test -z ${XDG_CONFIG_DIRS}
then
	config_dirs="/etc/xdg"
else
	config_dirs="${XDG_CONFIG_DIRS}"
fi

if ! test -z ${XDG_MENU_PREFIX}
then
	file_list="${config_dirs}/menus/${XDG_MENU_PREFIX}applications.menu"
else
	file_list="${HOME}/.config/jgmenu/default.menu \
		  ${config_dirs}/menus/gnome-applications.menu \
		  ${config_dirs}/menus/lxde-applications.menu \
		  ${config_dirs}/menus/kde-applications.menu"
fi

menu_file=
for f in ${file_list}
do
	if test -e ${f}
	then
		menu_file=${f}
		break
	fi
done

# TODO Consider doing a 'find ${config_dirs} -name "*.menu"' here
#      if menu_file is still empty

if ! test -z ${menu_file}
then
	jgmenu_run parse-xdg ${menu_file} | jgmenu
	exit 0
else
	printf "%s\n" "fatal: cannot find .menu file"
	exit 1
fi
