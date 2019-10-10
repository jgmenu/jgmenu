#!/bin/sh
#
# 'jgmenu init' creates/updates jgmenurc
#

config_file=~/.config/jgmenu/jgmenurc
prepend_file=~/.config/jgmenu/prepend.csv
append_file=~/.config/jgmenu/append.csv

xdg_config_dirs="${XDG_CONFIG_HOME:-$HOME/.config} ${XDG_CONFIG_DIRS:-/etc/xdg}"
_xdg_data_dirs="$XDG_DATA_HOME $HOME/.local/share $XDG_DATA_DIRS \
/usr/share /usr/local/share /opt/share"

theme=
verbose=f
interactive=f

regression_items="max_items min_items ignore_icon_cache color_noprog_fg \
show_title search_all_items ignore_xsettings arrow_show read_tint2rc \
tint2_rules tint2_button multi_window color_menu_fg at_pointer"

available_themes="\
	archlabs_1803 \
	bunsenlabs_hydrogen \
	bunsenlabs_helium \
	bunsenlabs_lithium \
	col2 \
	col3 \
	neon \
	greeneye"

JGMENU_EXEC_DIR=$(jgmenu_run --exec-path)

say () {
	printf "%b\n" "$@"
}

warn () {
	printf "warn: %b\n" "$@"
}

die () {
	printf "fatal: %b\n" "$@"
	exit 1
}

verbose_info () {
	test "$verbose" = "t" || return
	printf "info: %b\n" "$@"
}

isinstalled () {
	which "$1" >/dev/null 2>&1
}

usage () {
	say "\
usage: jgmenu init [<options>]\n\
       jgmenu init [--config-file=<file>] --regression-check\n\
Create/amend/check config files\n\
Options include:\n\
    -h|--help             Display this message\n\
    -i|--interactive      Enter interactive mode\n\
    --config-file=<file>  Specify config file\n\
    --theme=<theme>       Create config file with a particular theme\n\
    --list-themes         Display all available themes\n\
    --regression-check    Only check for config options no longer valid\n\
    --apply-obtheme       Apply current openbox theme to menu\n\
    --verbose             Be more verbose\n"
}

append__add () {
	printf "%b\n" "$@" >>"${append_file}"
}

append__sep () {
	test -e ${append_file} && return
	append__add "^sep()"
}

append__lock () {
	if grep -i 'lock' ${append_file} >/dev/null 2>&1
	then
		say "append.csv already contains a lock entry"
		return
	fi
	if isinstalled i3lock-fancy
	then
		append__add "Lock,i3lock-fancy -p,system-lock-screen"
		say "Append i3lock-fancy"
	elif isinstalled i3lock
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
	if isinstalled systemctl
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
	backup_config_files
	append__sep
	append__lock
	append__exit
}

prepend__terminals="x-terminal-emulator terminator uxterm xterm gnome-terminal \
lxterminal qterminal urxvt rxvt xfce4-terminal konsole sakura st"

prepend__browsers="firefox iceweasel chromium midori"

prepend__file_managers="pcmanfm thunar nautilus caja"

prepend__add () {
	printf "%b\n" "$@" >>"${prepend_file}"
}

prepend__add_sep () {
	if grep -i "\^sep(" ${prepend_file} >/dev/null 2>&1
	then
		say "prepend.csv already contains a separator"
		return
	fi
	prepend__add "^sep()"
}

prepend__add_terminal () {
	# Don't add if terminal already exists in prepend.csv
	test -e ${prepend_file} && for x in ${prepend__terminals}
	do
		# Don't grep for "st" - it's contained in too many strings :)
		test "${x}" = "st" && break
		if grep -i "${x}" "${prepend_file}" >/dev/null 2>&1
		then
			say "prepend.csv already contains a terminal entry"
			return
		fi
	done
	for x in ${prepend__terminals}
	do
		if isinstalled "${x}"
		then
			prepend__add "Terminal,${x},utilities-terminal"
			printf "Prepend %b\n" "${x}"
			break
		fi
	done
}

prepend__add_browser () {
	test -e ${prepend_file} && for x in ${prepend__browsers}
	do
		if grep -i "${x}" "${prepend_file}" >/dev/null 2>&1
		then
			say "prepend.csv already contains a browser entry"
			return
		fi
	done
	for x in ${prepend__browsers}
	do
		if isinstalled "${x}"
		then
			prepend__add "Browser,${x},${x}"
			printf "Prepend %b\n" "${x}"
			break
		fi
	done
}

prepend__add_file_manager () {
	test -e ${prepend_file} && for x in ${prepend__file_managers}
	do
		if grep -i "${x}" "${prepend_file}" >/dev/null 2>&1
		then
			say "prepend.csv already contains a file manager entry"
			return
		fi
	done
	for x in ${prepend__file_managers}
	do
		if isinstalled "${x}" >/dev/null 2>&1
		then
			prepend__add "File manager,${x},system-file-manager"
			printf "Prepend %b\n" "${x}"
			break
		fi
	done
}

prepend_items () {
	backup_config_files
	prepend__add_terminal
	prepend__add_browser
	prepend__add_file_manager
	prepend__add_sep
}

check_config_file () {
	if ! test -e "${config_file}"
	then
		say "info: creating config file 'jgmenurc'"
		jgmenu_run config -c >"${config_file}"
	fi
}

# Check for jgmenurc items which are no longer valid
check_regression () {
	for r in ${regression_items}
	do
		if grep "${r}" "${config_file}" >/dev/null 2>&1
		then
			warn "${r} is no longer a valid key"
		fi
	done
}

check_menu_package_installed () {
	# shellcheck disable=SC2039
	local menu_package_exists=
	for d in $xdg_config_dirs
	do
		for file in "${d}"/menus/*.menu
		do
			test -e "${file}" && menu_package_exists=t
			break
		done
	done
	if test "$menu_package_exists" = "t"
	then
		verbose_info "menu package(s) exist"
	else
		warn "no menu package installed (suggest package 'lxmenu-data' or similar)"
	fi
}

lx_installed () {
	test -e "${JGMENU_EXEC_DIR}"/jgmenu-lx
}

check_lx_installed () {
	if lx_installed
	then
		verbose_info "the lx module is installed"
	else
		warn "the lx module is not installed"
	fi
}

check_search_for_unicode_files () {
	for d in $_xdg_data_dirs
	do
		test -d "${d}"/applications/ || continue
		ls "${d}"/applications/*.desktop >/dev/null 2>&1 || continue
		for f in "${d}"/applications/*.desktop
		do
			if file -i "${f}" | grep -v 'utf-8\|ascii\|symlink'
			then
				say "${f}"
				unicode_found=y
			fi
		done
	done

	test "${unicode_found}" = "y" && warn "\
unicode files are not XDG compliant and may give unpredicted results"
}

icon_theme_last_used_by_jgmenu () {
	icon_theme=$(grep -i 'Inherits' ~/.cache/jgmenu/icons/index.theme)
	icon_size=$(grep -i 'Size' ~/.cache/jgmenu/icons/index.theme)
	printf "last time, icon-theme '%s-%s' was used\n" "${icon_theme#Inherits=}" \
		"${icon_size#Size=}"
}

get_icon_theme () {
	for d in $_xdg_data_dirs
	do
		test -d "${d}"/icons || continue
		ls -1 "${d}"/icons
	done | jgmenu --vsimple --center --no-spawn 2>/dev/null
}

print_available_themes () {
	for t in ${available_themes}
	do
		printf "%b\n" "${t}"
	done
}

get_theme () {
	print_available_themes | jgmenu --vsimple --center --no-spawn 2>/dev/null
}

# not currently used
restart_jgmenu () {
	say "Restarting jgmenu..."
	killall jgmenu >/dev/null 2>&1
	nohup jgmenu >/dev/null 2>&1 &
}

create_icon_greeneye () {
	greeneye_search_icon="${HOME}/.config/jgmenu/greeneye-search.svg"
	test -e "${greeneye_search_icon}" && return
	cat >"${greeneye_search_icon}" <<'EOF'
<!-- /usr/share/icons/breeze-dark/actions/22/system-search.svg -->
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 22 22">
  <defs id="defs3051">
    <style type="text/css" id="current-color-scheme">
      .ColorScheme-Text {
	color:#eff0f1;
      }
      </style>
  </defs>
 <path 
    style="fill:currentColor;fill-opacity:1;stroke:none" 
    d="M 9 3 C 5.6759952 3 3 5.6759952 3 9 C 3 12.324005 5.6759952 15 9 15 C 10.481205 15 11.830584 14.465318 12.875 13.582031 L 18.292969 19 L 19 18.292969 L 13.582031 12.875 C 14.465318 11.830584 15 10.481205 15 9 C 15 5.6759952 12.324005 3 9 3 z M 9 4 C 11.770005 4 14 6.2299952 14 9 C 14 11.770005 11.770005 14 9 14 C 6.2299952 14 4 11.770005 4 9 C 4 6.2299952 6.2299952 4 9 4 z "
    class="ColorScheme-Text"
    />
</svg>
EOF
}

fallback_if_no_openbox () {
	# not all systems support openbox menus
	if ! test -e ~/.config/openbox/menu.xml
	then
		jgmenu_run config -s "${config_file}" -k csv_cmd -v pmenu
	fi
}

fallback_if_no_lx () {
	# not all systems support the lx module
	if ! lx_installed
	then
		jgmenu_run config -s "${config_file}" -k csv_cmd -v pmenu
	fi
}

set_theme () {
	test $# -eq 0 && die "set_theme(): no theme specified"
	backup_config_files
	rm -f "${prepend_file}" "${append_file}"

	case "$1" in
	archlabs_1803)
		jgmenu_run themes archlabs_1803_config >"${config_file}"
		append_items
		prepend_items
		;;
	bunsenlabs_hydrogen)
		jgmenu_run themes bunsenlabs_hydrogen_config >"${config_file}"
		fallback_if_no_openbox
		;;
	bunsenlabs_helium)
		jgmenu_run themes bunsenlabs_helium_config >"${config_file}"
		fallback_if_no_openbox
		;;
	bunsenlabs_lithium)
		jgmenu_run themes bunsenlabs_lithium_config >"${config_file}"
		jgmenu_run themes bunsenlabs_lithium_prepend >"${prepend_file}"
		fallback_if_no_lx
		;;
	neon)
		jgmenu_run themes neon_config >"${config_file}"
		jgmenu_run themes neon_prepend >"${prepend_file}"
		jgmenu_run themes neon_append >"${append_file}"
		test -d "/usr/share/icons/breeze" || warn "icon theme 'breeze' is required"
		;;
	greeneye)
		create_icon_greeneye
		jgmenu_run greeneye --widgets >"${prepend_file}"
		jgmenu_run greeneye --config >"${config_file}"
		;;
	col2)
		jgmenu_run themes col2_config >"${config_file}"
		jgmenu_run themes col2_prepend >"${prepend_file}"
		;;
	col3)
		jgmenu_run themes col3_config >"${config_file}"
		;;
	esac
}

apply_obtheme () {
	backup_config_files
	jgmenu_run obtheme "${config_file}"
}

apply_gtktheme () {
	backup_config_files
	if ! test -e "${JGMENU_EXEC_DIR}"/jgmenu-gtktheme.py
	then
		warn "The gtktheme module is not installed on your system"
		return
	fi
	jgmenu_run gtktheme 2>/dev/null
}

check_nr_backups () {
	nr=$(find ~/.config/jgmenu/backup/ 2>/dev/null | wc -l)
	test "$nr" -gt 100 && warn "\
you have more than 100 backup files - consider removing a few"
}

backup_config_files () {
	# shellcheck disable=SC2039
	local files_to_backup="${HOME}/.config/jgmenu/jgmenurc \
		${HOME}/.config/jgmenu/prepend.csv \
		${HOME}/.config/jgmenu/append.csv"
	# shellcheck disable=SC2039
	local backup_dir

	backup_dir="${HOME}/.config/jgmenu/backup/$(date +%Y%m%d%H%M%S%N)"
	test -d "${backup_dir}" && die "duplicate backup directory"
	mkdir -p "${backup_dir}"
	say "Backing up config files..."
	for f in ${files_to_backup}
	do
		test -e "${f}" || continue
		cp -p "${f}" "${backup_dir}"
	done
}

analyse () {
	say "Check for obsolete config options..."
	check_regression
	say "Check installed menu packages..."
	check_menu_package_installed
	say "Check for lx module..."
	check_lx_installed
	say "Check for unicode files..."
	check_search_for_unicode_files
	return 0
}

initial_checks () {
	check_nr_backups
	check_config_file
}

# NOT YET IMPLEMENTED:
#d, dpi     = scan system for dpi settings and set config file
#g, gen     = choose CSV generator (i.e. the thing that produces the menu content)\n\
#i, icon    = set icon theme\n\
#s, search  = add search box\n\
#u, undo    = revert back to previous set of config files\n\
#y, polybar = sync with polybar settings
print_commands () {
	printf "%b" "\
*** commands ***\n\
a, append    = add items at bottom of root-menu (e.g. lock and exit)\n\
c, check     = run a number of jgmenu related checks on system\n\
g, gtktheme  = apply gtk theme (optional package)
h, help      = show this message
m, missing   = add any missing config options to config file\n\
o, obtheme   = apply openbox theme
p, prepend   = add items at top of root-menu (e.g. web browser and terminal)\n\
q, quit      = quit init process\n\
R, reset     = delete all config files and create a default jgmenurc\n\
t, theme     = create config files based on templates\n"
}

prompt () {
	# shellcheck disable=SC2039
	local cmd=

	printf "%b" "What now> "
	read -r cmd
	case "$cmd" in
	append|a)
		append_items
		;;
	check|c)
		analyse
		;;
	help|h)
		print_commands
		;;
	missing|m)
		backup_config_files
		jgmenu_run config -a "${config_file}"
		;;
	obtheme|o)
		apply_obtheme
		;;
	gtktheme|g)
		apply_gtktheme
		;;
	prepend|p)
		prepend_items
		;;
	quit|q)
		return 1
		;;
	reset|R)
		backup_config_files
		rm -f "${config_file}" "${prepend_file}" "${append_file}"
		check_config_file
		;;
	theme|t)
		set_theme "$(get_theme)"
		;;
	clear)
		clear ;;
	'')
		;;
	*)
		warn "'$cmd' is not a recognised command"
		print_commands
		;;
	esac
}

await_user_command () {
	print_commands
	while :
	do
		prompt || break
	done
}

while test $# != 0
do
	case "$1" in
	-i|--interactive)
		interactive=t ;;
	--config-file=*)
		config_file="${1#--config-file=}" ;;
	--theme=*)
		theme="${1#--theme=}" ;;
	--apply-obtheme)
		apply_obtheme
		exit 0
		;;
	--apply-gtktheme)
		apply_gtktheme
		exit 0
		;;
	--list-themes)
		print_available_themes
		exit 0
		;;
	--regression-check)
		check_regression
		exit 0
		;;
	--verbose)
		verbose=t ;;
	-h|--help)
		usage
		exit 0
		;;
	*)
		printf "fatal: unknown option: '%s'\n" "$1"
		usage
		exit 1
		;;
	esac
	shift
done

mkdir -p "${HOME}/.config/jgmenu"
test -z "${theme}" || { set_theme "$theme" ; exit 0 ; }
initial_checks
if test "${interactive}" = "t"
then
	await_user_command
else
	say "Run 'jgmenu init -h' for usage message"
	say "Run 'jgmenu init -i' to enter interactive mode"
fi
