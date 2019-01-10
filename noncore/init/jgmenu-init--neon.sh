# This shell script fragment is designed to be sourced by jgmenu-init.sh

add_neon_widgets () {
cat >${prepend_file} <<'EOF'
# Search box
@rect,,10,10,252,25,2,left,top,#666666 15,#000000 0,content
@search,,10,10,252,25,2,left,top,#666666 90,#222222 3,Type to search...

# Icon 1
@rect,^root(fav),25,40,42,42,2,left,top,#000000 0,#000000 0,
@icon,,30,45,32,32,2,left,top,#e6e6e6 100,#444444 90,/usr/share/icons/breeze/actions/32/bookmark-new.svg

# Icon 2
@rect,^root(pmenu),85,40,42,42,2,left,top,#000000 0,#000000 0,
@icon,,90,45,32,32,2,left,top,#e6e6e6 100,#444444 90,/usr/share/icons/breeze/actions/32/view-list-icons.svg

# Icon 3
@rect,^root(history),145,40,42,42,2,left,top,#000000 0,#000000 0,
@icon,,150,45,32,32,2,left,top,#e6e6e6 100,#444444 90,/usr/share/icons/breeze/actions/32/appointment-new.svg

# Icon 4
@rect,^root(exit),205,40,42,42,2,left,top,#000000 0,#000000 0,
@icon,,210,45,32,32,2,left,top,#e6e6e6 100,#444444 90,/usr/share/icons/breeze/actions/32/system-log-out.svg
EOF
}

add_neon_append_items () {
cat >>${append_file} <<'EOF'

^tag(fav)
Terminal,uxterm,utilities-terminal
Browser,firefox,firefox
File manager,pcmanfm,system-file-manager

^tag(history)
foo
bar

^tag(exit)
Lock,i3lock -c 000000,system-lock-screen
Exit to prompt,openbox --exit,system-log-out
Suspend,systemctl -i suspend,system-log-out
Reboot,systemctl -i reboot,system-reboot
Poweroff,systemctl -i poweroff,system-shutdown

EOF
}

setup_theme () {
	rm -f ${prepend_file}
	rm -f ${append_file}
	if ! test -d "/usr/share/icons/breeze"
	then
		warn "warn: icon theme 'breeze' is required to complete this theme"
	else
		add_neon_widgets
		add_neon_append_items
		say "Theme 'neon' has been set"
	fi
}
