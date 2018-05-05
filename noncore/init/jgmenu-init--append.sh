# This shell script fragment is designed to be sourced by jgmenu-init.sh

append__file=~/.config/jgmenu/append.csv

append__add () {
	printf "%b\n" "$@" >>"${append__file}"
}

append_items () {
	append__add "^sep()"

	if type i3lock-fancy >dev/null 2>&1
	then
		append__add "Lock,i3lock-fancy -p,system-lock-screen"
	elif type i3lock >/dev/null 2>&1
	then
		append__add "Lock,i3lock -c 000000,system-lock-screen"
	fi

	if type systemctl >/dev/null
	then
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
