# This shell script fragment is designed to be sourced by jgmenu-init.sh

append__add () {
	printf "%b\n" "$@" >>"${append_file}"
}

append__lock () {
	if grep -i 'lock' ${append_file} >/dev/null 2>&1
	then
		say "append.csv already contains a lock entry"
		return
	fi
	if type i3lock-fancy >/dev/null 2>&1
	then
		append__add "Lock,i3lock-fancy -p,system-lock-screen"
		say "Append i3lock-fancy"
	elif type i3lock >/dev/null 2>&1
	then
		append__add "Lock,i3lock -c 000000,system-lock-screen"
		say "Append i3lock"
	fi
}

append__exit () {
	if grep -i 'exit' ${append_file} >/dev/null 2>&1
	then
		say "append.csv already contains an exit entry"
		return
	fi
	if type systemctl >/dev/null
	then
		say "Append exit options (systemctl)"
		append__add "Exit,^checkout(exit),system-shutdown"
		append__add "^tag(exit)"
		if pgrep openbox >/dev/null
		then
			append__add "exit to prompt,openbox --exit,system-log-out"
		fi
		append__add "Suspend,systemctl -i suspend,system-log-out"
		append__add "Reboot,systemctl -i reboot,system-reboot"
		append__add "Poweroff,systemctl -i poweroff,system-shutdown"
	fi
}

append_items () {
	append__add "^sep()"
	append__lock
	append__exit
}
