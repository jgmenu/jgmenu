#!/bin/sh

# 'jgmenu init' creates/updates jgmenurc

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
color_title_bg show_title search_all_items ignore_xsettings arrow_show \
read_tint2rc tint2_rules tint2_button multi_window color_menu_fg"

available_themes="\
	archlabs_1803 \
	bunsenlabs_hydrogen \
	bunsenlabs_helium \
	bunsenlabs_lithium_rc1 \
	neon \
	greeneye"

JGMENU_EXEC_DIR=$(jgmenu_run --exec-path)

say () {
	printf "%b\n" "$@"
}

warn () {
	printf "warning: %b\n" "$@"
}

die () {
	printf "fatal: %b\n" "$@"
	exit 1
}

verbose_info () {
	test "$verbose" = "t" || return
	printf "info: %b\n" "$@"
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
    --verbose             Be more verbose\n"
}

jgmenurc_archlabs_1803 () {
cat >${config_file} <<'EOF'
stay_alive           = 1
csv_cmd              = pmenu
tint2_look           = 0
at_pointer           = 0
terminal_exec        = termite
terminal_args        = -e
menu_width           = 200
menu_padding_top     = 10
menu_padding_right   = 2
menu_padding_bottom  = 5
menu_padding_left    = 2
menu_radius          = 0
menu_border          = 1
menu_halign          = left
sub_hover_action     = 1
item_margin_y        = 5
item_height          = 30
item_padding_x       = 8
item_radius          = 0
item_border          = 0
sep_height           = 5
font                 = Ubuntu 12px
icon_size            = 24
color_menu_bg        = #2b303b 100
color_norm_bg        = #2b303b 0
color_norm_fg        = #8fa1b3 100
color_sel_bg         = #8fa1b3 60
color_sel_fg         = #2b303b 100
color_sep_fg         = #8fa1b3 40
EOF
}

jgmenurc_bunsenlabs_hydrogen () {
cat >${config_file} <<'EOF'
tint2_look          = 0
at_pointer          = 1
csv_cmd             = ob
menu_width          = 120
menu_padding_top    = 0
menu_padding_right  = 0
menu_padding_bottom = 0
menu_padding_left   = 0
menu_radius         = 1
sub_spacing         = 3
item_margin_x       = 1
item_margin_y       = 1
item_height         = 19
sep_height          = 4
sep_halign          = right
icon_size           = 0
arrow_width         = 8
color_menu_bg       = #3a3a3a 100
EOF
}

jgmenurc_bunsenlabs_helium () {
cat >${config_file} <<'EOF'
tint2_look          = 0
at_pointer          = 1
csv_cmd             = ob
menu_width          = 134
menu_padding_top    = 0
menu_padding_right  = 0
menu_padding_bottom = 0
menu_padding_left   = 0
menu_radius         = 1
sub_spacing         = 6
item_margin_x       = 1
item_margin_y       = 1
item_height         = 21
sep_height          = 4
sep_halign          = right
font                = Sans 10
icon_size           = 0
arrow_string        = ›
arrow_width         = 8
color_menu_bg       = #C8CFCB 100
color_menu_border   = #C8CFCB 8
color_norm_bg       = #C8CFCB 00
color_norm_fg       = #13071B 100
color_sel_bg        = #74998B 100
color_sel_fg        = #101010 100
color_sel_border    = #74998B 8
color_title_fg      = #101010 100
color_title_bg      = #74998B 100
color_title_border  = #74998B 8
color_sep_fg        = #101010 80
EOF
}

jgmenurc_neon () {
cat >${config_file} <<'EOF'
tint2_look          = 0
menu_margin_y       = 30
menu_width          = 272
menu_padding_top    = 100
menu_padding_right  = 10
menu_padding_bottom = 10
menu_padding_left   = 10
menu_valign         = bottom
item_radius         = 2
item_border         = 1
font		    = Roboto Condensed 9
color_menu_bg       = #cecece 90
color_menu_border   = #888888 100
color_norm_fg       = #444444 100
color_sel_bg        = #e6e6e6 100
color_sel_fg        = #444444 100
color_sel_border    = #888888 100
EOF
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
		if type "${x}" >/dev/null 2>&1
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
		if type "${x}" >/dev/null 2>&1
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
		if type "${x}" >/dev/null 2>&1
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
	cat >${greeneye_search_icon} <<'EOF'
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

neon__add_widgets () {
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

neon__add_append_items () {
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

neon__setup_theme () {
	if ! test -d "/usr/share/icons/breeze"
	then
		warn "warn: icon theme 'breeze' is required to complete this theme"
	else
		neon__add_widgets
		neon__add_append_items
		say "Theme 'neon' has been set"
	fi
}

bunsenlabs__setup_theme () {
	# not all systems support openbox menus
	if ! test -e ~/.config/openbox/menu.xml
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
		jgmenurc_archlabs_1803
		append_items
		prepend_items
		;;
	bunsenlabs_hydrogen)
		jgmenurc_bunsenlabs_hydrogen
		bunsenlabs__setup_theme
		;;
	bunsenlabs_helium)
		jgmenurc_bunsenlabs_helium
		bunsenlabs__setup_theme
		;;
	bunsenlabs_lithium)
		jgmenu_run themes bunsenlabs_lithium_rc1_config >"${config_file}"
		jgmenu_run themes bunsenlabs_lithium_rc1_prepend >"${prepend_file}"
		bunsenlabs__setup_theme
		;;
	bunsenlabs_lithium_rc1)
		jgmenu_run themes bunsenlabs_lithium_rc1_config >"${config_file}"
		jgmenu_run themes bunsenlabs_lithium_rc1_prepend >"${prepend_file}"
		bunsenlabs__setup_theme
		;;
	neon)
		jgmenurc_neon
		neon__setup_theme
		;;
	greeneye)
		create_icon_greeneye
		jgmenu_run greeneye --widgets >"${prepend_file}"
		jgmenu_run greeneye --config >"${config_file}"
		;;
	esac
}

apply_obtheme () {
	backup_config_files
	jgmenu_run obtheme "${config_file}"
}

check_nr_backups () {
	nr=$(ls -1 ~/.config/jgmenu/backup/ 2>/dev/null | wc -l)
	test "$nr" -gt 100 && warn "\
you have more than 100 backup files - consider removing a few"
}

backup_config_files () {
	local files_to_backup="${HOME}/.config/jgmenu/jgmenurc \
		${HOME}/.config/jgmenu/prepend.csv \
		${HOME}/.config/jgmenu/append.csv"
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
h, help      = show this message
m, missing   = add any missing config options to config file\n\
o, obtheme   = apply openbox theme
p, prepend   = add items at top of root-menu (e.g. web browser and terminal)\n\
q, quit      = quit init process\n\
t, theme     = create config files based on templates\n"
}

prompt () {
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
	prepend|p)
		prepend_items
		;;
	quit|q)
		return 1
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
