#!/bin/sh

set_prefix () {
	# Omitting xfce as it doesn't seem to work with libmenu-cache
	prefixes="lxde- lxqt- gnome-"

	for p in ${prefixes}
	do
		if test -e "/etc/xdg/menus/${p}applications.menu"
		then
			echo "found ${p}"
			export XDG_MENU_PREFIX="${p}"
			return
		fi
	done
}

test -z ${XDG_MENU_PREFIX+x} && set_prefix

echo "info: parsing ${XDG_MENU_PREFIX}applications.menu"
jgmenu-parse-lx | jgmenu $@
