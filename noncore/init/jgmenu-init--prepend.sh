# This shell script fragment is designed to be sourced by jgmenu-init.sh

terminals="x-terminal-emulator terminator uxterm xterm gnome-terminal \
lxterminal qterminal urxvt rxvt xfce4-terminal konsole sakura st"

add_terminal () {
	test -e ${prepend_file} && for t in ${terminals}
	do
		if grep -i ${t} ${prepend_file} >/dev/null 2>&1
		then
			say "prepend.csv already contains a terminal entry"
			return
		fi
	done
	for t in ${terminals}
	do
		if type ${t} >/dev/null 2>&1
		then
			printf "%b\n" "Terminal,${t},utilities-terminal" \
				>>"${prepend_file}"
			say "'${t}' added"
			break
		fi
	done
}

prepend_items () {
	add_terminal
#Browser,firefox,firefox
#File manager,pcmanfm,system-file-manager
#Terminal,xterm,utilities-terminal
}
