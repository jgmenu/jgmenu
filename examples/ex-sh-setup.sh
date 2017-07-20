get_theme () {
	theme=$(sh -c 'ls -1 /usr/share/icons | jgmenu --vsimple --no-spawn 2>/dev/null')
}

get_LANG () {
	lang=$(sh -c 'locale -a | grep _ | jgmenu --vsimple --no-spawn 2>/dev/null')
	test -z ${lang} || LANG=${lang}
}

restart_tint2 () {
	killall tint2 2>/dev/null
	if test -z $1
	then
		nohup tint2 >/dev/null 2>&1 &
	else
		nohup tint2 -c ${1} >/dev/null 2>&1 &
	fi
}

unset_tint2_variables () {
	unset TINT2_BUTTON_ALIGNED_X1
	unset TINT2_BUTTON_ALIGNED_X2
	unset TINT2_BUTTON_ALIGNED_Y1
	unset TINT2_BUTTON_ALIGNED_Y2
	unset TINT2_BUTTON_PANEL_X1
	unset TINT2_BUTTON_PANEL_X2
	unset TINT2_BUTTON_PANEL_Y1
	unset TINT2_BUTTON_PANEL_Y2
	unset TINT2_CONFIG
}

main () {
	if pgrep tint2 >/dev/null
	then
		printf "%b\n" "Your current tint2 will be killed"
		killall tint2 2>/dev/null
	fi
	if pgrep jgmenu >/dev/null
	then
		printf "%b\n" "Your current jgmenu will be killed"
		killall jgmenu 2>/dev/null
	fi
	rm -rf /tmp/jgmenu
	mkdir -p /tmp/jgmenu
	unset_tint2_variables
}

main
