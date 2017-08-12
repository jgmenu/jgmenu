#!/bin/sh

f=$(mktemp)
widgets=~/.config/jgmenu/widgets

usage () {
	printf "usage: jgmenu_run ob\n\n"
	printf "Generate menu based on Openbox menu-file located at:\n\n"
	printf "    ~/.config/openbox/menu.xml\n\n"
	exit 0
}

test "$1" = "--help" && usage

jgmenu --icon-size=0 --csv-cmd="jgmenu_run parse-ob"
