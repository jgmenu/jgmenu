#!/bin/sh

usage () {
	printf "usage: jgmenu_run lx\n\n"
	printf "Generate XDG compliant menu using LXDE's libmenu-cache.\n\n"
	printf "Set \$XDG_MENU_PREFIX to specity a .menu file. The following example will\n"
	printf "run a menu based on lxde-applications.menu:\n\n"
	printf "    XDG_MENU_PREFIX=lxde- jgmenu_run lx\n\n"
	printf "This can be useful if there are several .menu files on the system.\n"
	printf "If XDG_MENU_PREFIX is not set, some predefined prefixes (e.g. lxde- and\n"
	printf "gnome-) will be used. Please note that xfce-applications.menu does not\n"
	printf "seem to work with libmenu-cache\n\n"
	exit 0
}

set_prefix () {
	# Omitting xfce as it doesn't seem to work with libmenu-cache
	prefixes="lxde- lxqt- gnome-"

	for p in ${prefixes}
	do
		if test -e "/etc/xdg/menus/${p}applications.menu"
		then
			export XDG_MENU_PREFIX="${p}"
			return
		fi
	done
}

test "$1" = "--help" && usage
test -z ${XDG_MENU_PREFIX+x} && set_prefix

echo "info: parsing ${XDG_MENU_PREFIX}applications.menu"
jgmenu-parse-lx | jgmenu $@
