#!/bin/sh

valid_args="\
	archlabs_1803_config \
	bunsenlabs_hydrogen_config \
	bunsenlabs_helium_config \
	bunsenlabs_lithium_config \
	bunsenlabs_lithium_prepend \
	col2_config \
	col2_prepend \
	col3_config \
	neon_config \
	neon_append \
	neon_prepend"

say () {
	printf '%b\n' "$@"
}

die () {
	printf 'fatal: %b\n' "$@" >&2
	exit 1
}

usage () {
	printf '%b' "Usage: jgmenu_run themes <theme>_<mode>
Output config files for themes
<mode> is one of config, prepend and append" >&2
	exit 0
}

archlabs_1803_config () {
cat <<'EOF'
stay_alive           = 1
tint2_look           = 0
position_mode        = fixed
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

bunsenlabs_hydrogen_config () {
cat <<'EOF'
tint2_look          = 0
position_mode       = pointer
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

bunsenlabs_helium_config () {
cat <<'EOF'
tint2_look          = 0
position_mode       = pointer
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

bunsenlabs_lithium_config () {
cat <<'EOF'
tint2_look          = 0
csv_cmd             = apps
position_mode       = pointer
menu_width          = 134
menu_padding_top    = 24
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
color_menu_bg       = #22373f 100
color_menu_border   = #C8CFCB 8
color_norm_bg       = #C8CFCB 00
color_norm_fg       = #d3dae3 100
color_sel_bg        = #bc4b4f 100
color_sel_fg        = #ffffff 100
color_sel_border    = #bc4b4f 100
color_title_fg      = #d3dae3 100
color_title_bg      = #22373f 100
color_title_border  = #74998B 8
color_sep_fg        = #535353 100
sep_markup =
hover_delay         = 30
csv_name_format     = %n
EOF
}

bunsenlabs_lithium_prepend () {
cat <<'EOF'
# BunsenLabs Main Menu

@text,,6,6,150,20,2,left,top,auto,#000000 0,<span size="large">🔍︎</span>
@search,,24,3,150,20,2,left,top,auto,#000000 0,Type to Search

^sep()
Run Program,gmrun
^sep()
Terminal,x-terminal-emulator,utilities-terminal
Web Browser,x-www-browser,web-browser
File Manager,bl-file-manager,system-file-manager
Text Editor,bl-text-editor,text-editor
Media Player,bl-media-player,multimedia-player
^sep()
Applications,^checkout(apps)
^sep()
BL Utilities,^checkout(bl-utilities),applications-utilities
Places,^pipe(jgmenu_run ob --cmd="bl-places-pipemenu" --tag="bl-places-pipemenu"),folder
Recent Files,^pipe(jgmenu_run ob --cmd="bl-recent-files-pipemenu -rl15" --tag="bl-recent-files-pipemenu")
^sep()
Preferences,^checkout(bl-preferences),preferences-system
System,^checkout(bl-system),applications-system
^sep()
#Help &amp; Resources,^pipe(jgmenu_run ob --cmd="bl-help-pipemenu" --tag="bl-help-pipemenu")
Help &amp; Resources,^checkout(bl-help-menu),help-contents
Display Keybinds,^pipe(jgmenu_run ob --cmd="bl-kb-pipemenu" --tag="bl-kb-pipemenu")
^sep()
Lock Screen,bl-lock,system-lock-screen
Exit,bl-exit,system-shutdown

. /usr/share/bunsen/configs/menu-includes/help-menu

^tag(bl-utilities)
Back,^back()
Take Screenshot,^pipe(bl-scrot-pipemenu)
SSH,^pipe(jgmenu_run ob --cmd="bl-sshconfig-pipemenu" --tag="bl-sshconfig-pipemenu")
Remote Desktop,^pipe(jgmenu_run ob --cmd="bl-remote-desktop-pipemenu" --tag="bl-remote-desktop-pipemenu")
BLOB Themes Manager,bl-obthemes
# These two utilities are available if you uncomment the line.
# Pipe menu to install and use Dropbox:
#Dropbox,^pipe(jgmenu_run ob --cmd="bl-dropbox-pipemenu" --tag="bl-dropbox-pipemenu")
# Utility to set language if login greeter does not offer that option:
#Choose Language,bl-setlocale

^tag(bl-preferences)
Back,^back()
BunsenLabs,^checkout(bl-blConfig)
Openbox,^checkout(bl-obConfig)
jgmenu,^checkout(bl-jgmenuConfig)
Keybinds,^checkout(bl-xbindkeysConfig)
Compositor,^pipe(jgmenu_run ob --cmd="bl-compositor" --tag="bl-compositor")
Conky,^pipe(jgmenu_run ob --cmd="bl-conky-pipemenu" --tag="bl-conky-pipemenu"),conky-manager
Tint2,^pipe(jgmenu_run ob --cmd="bl-tint2-pipemenu" --tag="bl-tint2-pipemenu"),tint2
Appearance,lxappearance,preferences-desktop-theme
Font configuration,bl-text-editor ~/.config/fontconfig/fonts.conf,preferences-desktop-font
Wallpaper,nitrogen,preferences-desktop-wallpaper,wallpaper
Notifications,xfce4-notifyd-config,notifyconf
Power Management,xfce4-power-manager-settings,xfce4-power-manager-settings
dmenu,^checkout(bl-dmenuconfig)
gmrun,^checkout(bl-gmrunconfig)
Display,^checkout(bl-DisplaySettings)

BunsenLabs Session,^tag(bl-blConfig)
Back,^back()
Edit autostart,bl-text-editor ~/.config/bunsen/autostart
Edit environment,bl-text-editor ~/.config/bunsen/environment
^sep(Manuals)
bunsenlabs-session,x-terminal-emulator -T 'man bunsenlabs-session' -e man bunsenlabs-session
xdg-autostart,x-terminal-emulator -T 'man bl-xdg-autostart' -e man bl-xdg-autostart

Openbox,^tag(bl-obConfig),openbox
Back,^back()
Edit bl-rc.xml,bl-text-editor ~/.config/openbox/bl-rc.xml
^sep()
WM Preferences,obconf --config-file ~/.config/openbox/bl-rc.xml
^sep()
Reconfigure,openbox --reconfigure
Restart,openbox --restart

jgmenu,^tag(bl-jgmenuConfig)
Back,^back()
Edit Menu Content,bl-text-editor ~/.config/jgmenu/prepend.csv
Edit Menu Settings,bl-text-editor ~/.config/jgmenu/jgmenurc
^sep()
Sync Theme w. Openbox,JGMENU_RCXML=$HOME/.config/openbox/bl-rc.xml jgmenu init --apply-obtheme
^sep(Help)
man page,x-terminal-emulator -T 'man jgmenu' -e man jgmenu
tutorial,x-terminal-emulator -T 'man jgmenututorial' -e man jgmenututorial

Keybinds,^tag(bl-xbindkeysConfig)
Back,^back()
Edit .xbindkeysrc,bl-text-editor ~/.xbindkeysrc
Restart,sh -c 'pkill -x xbindkeys; xbindkeys_autostart'
^sep()
# Next entry requires tk
#Show Keybinds,xbindkeys_show
^sep(Help)
man page,x-terminal-emulator -T 'man xbindkeys' -e man xbindkeys

dmenu,^tag(bl-dmenuconfig)
Back,^back()
Edit Start-up Script,bl-text-editor ~/.config/dmenu/dmenu-bind.sh
^sep(Help)
man page,x-terminal-emulator -T 'man dmenu' -e man dmenu

gmrun,^tag(bl-gmrunconfig)
Back,^back()
Edit Config File,bl-text-editor ~/.gmrunrc
^sep(Help)
man page,x-terminal-emulator -T 'man gmrun' -e man gmrun

Display,^tag(bl-DisplaySettings)
Back,^back()
ARandR Screen Layout Editor,arandr
^sep(Help)
man xrandr,x-terminal-emulator -T 'man xrandr' -e man xrandr

System,^tag(bl-system)
Back,^back()
Printers,^pipe(jgmenu_run ob --cmd="bl-printing-pipemenu" --tag="bl-printing-pipemenu"),printer
Task Manager (htop),x-terminal-emulator -T 'htop task manager' -e htop,htop
Install Selected Packages,^pipe(bl-install-pipemenu)
Synaptic Package Manager,pkexec synaptic,synaptic
Login Settings,pkexec bl-text-editor /etc/lightdm/lightdm-gtk-greeter.conf /etc/lightdm/lightdm.conf,lightdm-settings
Login Interface,lightdm-gtk-greeter-settings-pkexec,lightdm-settings
GParted,pkexec gparted,gparted
Set Default Browser,"""x-terminal-emulator -T 'Select Default Browser' -e sh -c 'sudo update-alternatives --config x-www-browser; sleep 5'""",web-browser
Edit Debian Alternatives,galternatives,galternatives
^sep()
About Bunsen Alternatives,yad --button="OK":0 --center --window-icon=distributor-logo-bunsenlabs --text-info --title="About Bunsen Alternatives" --filename="/usr/share/bunsen/docs/helpfile-bl-alternatives.txt" --width=900 --height=700 --fontname=Monospace,distributor-logo-bunsenlabs

^tag(apps)
EOF
}

col2_config () {
cat <<'EOF'
tint2_look           = 0
columns              = 2
menu_width           = 500
menu_height_min      = 470
menu_height_max      = 470
menu_padding_top     = 50
menu_padding_right   = 40
menu_padding_bottom  = 40
menu_padding_left    = 40
menu_radius          = 0
menu_border          = 0
menu_halign          = center
menu_valign          = center
item_margin_y        = 5
item_height          = 46
item_padding_x       = 8
item_radius          = 0
item_border          = 2
sep_height           = 5
font                 = UbuntuMono 12
icon_size            = 18
icon_text_spacing    = 3
color_menu_bg        = #2B303B 100
color_norm_bg        = #1C2023 0
color_norm_fg        = #ffffff 100
color_sel_bg         = #8fa1b3 0
color_sel_fg         = #ffffff 100
color_sel_border     = #111111 100
color_sep_fg         = #919BA0 40
csv_no_dirs          = 1
csv_name_format      = %n\n<span size="x-small">%g</span>
EOF
}

col2_prepend () {
cat <<'EOF'
# Search box
@rect,,23,10,454,35,3,left,top,#111111 90,#000000 0,
@search,,30,15,454,25,3,left,top,#ffffff 100,#222222 3,Type to search
EOF
}

col3_config () {
cat <<'EOF'
position_mode = center
columns = 3
menu_width = 650
menu_height_min = 410
menu_height_max = 410
menu_padding_top = 15
menu_padding_right = 15
menu_padding_bottom = 15
menu_padding_left = 15
menu_radius = 15
menu_border = 4
#menu_halign = center
#menu_valign = center
color_menu_bg = #000000 65
color_menu_border = #eeeeee 20
csv_no_dirs = 1
csv_name_format = %n
EOF
}

neon_config () {
cat <<'EOF'
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

neon_prepend () {
cat <<'EOF'
# Search box
@rect,,10,10,252,25,2,left,top,#666666 15,#000000 0,content
@search,,10,10,252,25,2,left,top,#666666 90,#222222 3,Type to search...

# Icon 1
@rect,^root(fav),25,40,42,42,2,left,top,#000000 0,#000000 0,
@icon,,30,45,32,32,2,left,top,#e6e6e6 100,#444444 90,/usr/share/icons/breeze/actions/32/bookmark-new.svg

# Icon 2
@rect,^root(apps),85,40,42,42,2,left,top,#000000 0,#000000 0,
@icon,,90,45,32,32,2,left,top,#e6e6e6 100,#444444 90,/usr/share/icons/breeze/actions/32/view-list-icons.svg

# Icon 3
@rect,^root(history),145,40,42,42,2,left,top,#000000 0,#000000 0,
@icon,,150,45,32,32,2,left,top,#e6e6e6 100,#444444 90,/usr/share/icons/breeze/actions/32/appointment-new.svg

# Icon 4
@rect,^root(exit),205,40,42,42,2,left,top,#000000 0,#000000 0,
@icon,,210,45,32,32,2,left,top,#e6e6e6 100,#444444 90,/usr/share/icons/breeze/actions/32/system-log-out.svg

^tag(apps)
EOF
}

neon_append () {
cat <<'EOF'

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

test $# -lt 1 && usage

for a in ${valid_args}
do
	if test "$1" = "${a}"
	then
		$1
		exit 0
	fi
done

say "not a valid theme"
usage

