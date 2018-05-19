# This shell script fragment is designed to be sourced by jgmenu-init.sh

add_search_box () {
cat >${prepend_file} <<'EOF'
@rect,action,10,10,180,25,3,left,top,#666666 15,#000000 0,content
@search,action,10,10,180,25,3,left,top,#666666 90,#222222 3,Type to search...
EOF
}

setup_tint2_neon_theme () {
	type tint2 >/dev/null  || return
	warn "your current tint2rc will over-written if you answer 'y'"
	say "Do you wish to create a tint2 config file to match this menu [yN]"
	read answer
	if test "$answer" = "y" || test "$answer" = "Y"
	then
		mkdir -p ~/.config/tint2
		cp -f "${JGMENU_EXEC_DIR}"/tint2rc.neon ~/.config/tint2/tint2rc
		restart_tint2
	fi
}

setup_theme () {
	rm -f ${prepend_file}
	rm -f ${append_file}
	add_search_box
	setup_tint2_neon_theme
}
