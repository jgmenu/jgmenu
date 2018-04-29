# This shell script fragment is designed to be sourced by jgmenu-init.sh

setup_theme () {
	# not all systems support openbox menus
	if ! test -e ~/.config/openbox/menu.xml
	then
		echo "set csv_cmd=pmenu"
		#TODO: jgmenu_run config set csv_cmd pmenu
	fi
}
