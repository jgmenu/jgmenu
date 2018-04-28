#!/bin/sh

# 'jgmenu init' creates/updates jgmenurc

jgmenurc=~/.config/jgmenu/jgmenurc
jgmenurc_bak=${jgmenurc}.$(date +%Y%m%d%H%M)
xdg_config_dirs="${XDG_CONFIG_HOME:-$HOME/.config} ${XDG_CONFIG_DIRS:-/etc/xdg}"
theme=

regression_items="max_items min_items ignore_icon_cache color_noprog_fg \
color_title_bg show_title search_all_items ignore_xsettings arrow_show \
read_tint2rc tint2_rules tint2_button multi_window"

say () {
	printf "%b\n" "$@"
}

usage () {
	say "\
usage: jgmenu init [<options>]\n\
Create/amend config files\n\
Options include:\n\
    --config-file=<file>  Specify config file\n\
    --theme=<theme>       Create config file with a particular theme\n\
                          Valid themes include: bunsenlabs"
}

# Check for jgmenurc items which are no longer valid
check_regression () {
	for r in ${regression_items}
	do
		if grep ${r} ${jgmenurc} >/dev/null 2>&1
		then
			printf "%b\n" "warning: ${r} is no longer a valid key"
		fi
	done
}

check_menu_package_installed () {
	local menu_package_exists=
	for d in $xdg_config_dirs
	do
		if test -e $d/menus/*.menu
		then
			menu_package_exists=t
		fi
	done
	if test "$menu_package_exists" = "t"
	then
		echo "info: menu package(s) exist"
	else
		say "warn: no menu package installed"
	fi
}

backup_jgmenurc () {
	test -e ${jgmenurc} && cp -p ${jgmenurc} ${jgmenurc_bak}
}

# START OF SCRIPT

while test $# != 0
do
	case "$1" in
	--config-file=*)
		jgmenurc="${1#--config-file=}" ;;
	--theme=*)
		theme="${1#--theme=}" ;;
	--help)
		usage
		exit 0
		;;
	*)
		printf "fatal: unknown option: '%s'\n" $1
		usage
		exit 1
		;;
	esac
	shift
done

mkdir -p ~/.config/jgmenu
backup_jgmenurc
check_menu_package_installed

JGMENU_EXEC_DIR=$(jgmenu_run --exec-path)
if ! test -z ${theme}
then
	if test "${theme}" = "bunsenlabs"
	then
		. ${JGMENU_EXEC_DIR}/jgmenu-init--bunsenlabs.sh
		set_theme_bunsenlabs
	else
		printf "warn: cannot find theme '${theme}'\n"
	fi
else
	if ! test -e ${jgmenurc}
	then
		say "info: creating jgmenurc"
		cp ${JGMENU_EXEC_DIR}/jgmenurc ${jgmenurc}
	else
		jgmenu_run config amend --file "${jgmenurc}"
	fi
fi

check_regression
