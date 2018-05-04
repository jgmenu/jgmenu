# This shell script fragment is designed to be sourced by jgmenu-init.sh

prepend_add () {
	printf "%b\n" "$@" >>"${prepend_file}"
}

add_browser () {
	test -e ${prepend_file} && for b in ${browsers}
	do
		if grep -i ${b} ${prepend_file} >/dev/null 2>&1
		then
			say "prepend.csv already contains a browser entry"
			return
		fi
	done
	for b in ${browsers}
	do
		if type ${t} >/dev/null 2>&1
		then
			prepend_add "Browser,${b},${b}"
			printf "Prepend %b\n" "${b}"
			break
		fi
	done
}

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
			prepend_add "Terminal,${t},utilities-terminal"
			printf "Prepend %b\n" "${t}"
			break
		fi
	done
}

prepend_items () {
	#File manager,pcmanfm,system-file-manager
	add_browser
	add_terminal
	prepend_add "^sep()"
}
