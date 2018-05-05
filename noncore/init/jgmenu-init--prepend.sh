# This shell script fragment is designed to be sourced by jgmenu-init.sh

prepend__terminals="x-terminal-emulator terminator uxterm xterm gnome-terminal \
lxterminal qterminal urxvt rxvt xfce4-terminal konsole sakura st"

prepend__browsers="firefox iceweasel chromium midori"

prepend__add () {
	printf "%b\n" "$@" >>"${prepend_file}"
}

prepend__add_browser () {
	test -e ${prepend_file} && for b in ${prepend__browsers}
	do
		if grep -i ${b} ${prepend_file} >/dev/null 2>&1
		then
			say "prepend.csv already contains a browser entry"
			return
		fi
	done
	for b in ${prepend__browsers}
	do
		if type ${t} >/dev/null 2>&1
		then
			prepend__add "Browser,${b},${b}"
			printf "Prepend %b\n" "${b}"
			break
		fi
	done
}

prepend__add_terminal () {
	test -e ${prepend_file} && for t in ${prepend__terminals}
	do
		if grep -i ${t} ${prepend_file} >/dev/null 2>&1
		then
			say "prepend.csv already contains a terminal entry"
			return
		fi
	done
	for t in ${prepend__terminals}
	do
		if type ${t} >/dev/null 2>&1
		then
			prepend__add "Terminal,${t},utilities-terminal"
			printf "Prepend %b\n" "${t}"
			break
		fi
	done
}

prepend_items () {
	#File manager,pcmanfm,system-file-manager
	prepend__add_browser
	prepend__add_terminal
	prepend__add "^sep()"
}
