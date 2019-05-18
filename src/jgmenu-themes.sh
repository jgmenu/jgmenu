#!/bin/sh

valid_args="bunsenlabs_lithium_rc1_config bunsenlabs_lithium_rc1_prepend"

say () {
	printf "%b\n" "$@"
}

die () {
	printf "fatal: %b\n" "$@"
	exit 1
}

usage () {
	say "\
usage: jgmenu_run themes <theme>_<mode>\n\
Output config files for themes\n\
<mode> is one of config, prepend and append\n"
	exit 0
}

bunsenlabs_lithium_rc1_config () {
cat <<'EOF'
tint2_look          = 0
at_pointer          = 1
csv_cmd             = pmenu
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

bunsenlabs_lithium_rc1_prepend () {
cat <<'EOF'
# BunsenLabs Main Menu

@search,,1,3,150,20,2,left,top,auto,#000000 0,Search...

^sep()
Run Program,gmrun
^sep()
Terminal,x-terminal-emulator
Web Browser,x-www-browser
File Manager,bl-file-manager
Text Editor,bl-text-editor
Media Player,bl-media-player
^sep()
Applications,^checkout(lx-apps)
^sep()
BL Pipes,^checkout(bl-pipes)
Places,^pipe(jgmenu_run ob --cmd="bl-places-pipemenu" --tag="bl-places-pipemenu")
Recent Files,^pipe(jgmenu_run ob --cmd="bl-recent-files-pipemenu -rl15" --tag="bl-recent-files-pipemenu")
^sep()
Preferences,^checkout(bl-preferences)
System,^checkout(bl-system)
^sep()
Help &amp; Resources,^pipe(jgmenu_run ob --cmd="bl-help-pipemenu" --tag="bl-help-pipemenu")
Display Keybinds,^pipe(jgmenu_run ob --cmd="bl-kb-pipemenu" --tag="bl-kb-pipemenu")
^sep()
Lock Screen,bl-lock
Exit,bl-exit

^tag(bl-pipes)
Graphics,^pipe(jgmenu_run ob --cmd="bl-graphics-pipemenu" --tag="bl-graphics-pipemenu")
Multimedia,^pipe(jgmenu_run ob --cmd="bl-multimedia-pipemenu" --tag="bl-multimedia-pipemenu")
WWW Browsers,^pipe(jgmenu_run ob --cmd="bl-x-www-browser-pipemenu" --tag="bl-x-www-browser-pipemenu")
Remote Desktop,^pipe(jgmenu_run ob --cmd="bl-remote-desktop-pipemenu" --tag="bl-remote-desktop-pipemenu")
SSH,^pipe(jgmenu_run ob --cmd="bl-sshconfig-pipemenu" --tag="bl-sshconfig-pipemenu")
LibreOffice,^pipe(jgmenu_run ob --cmd="bl-libreoffice-pipemenu" --tag="bl-libreoffice-pipemenu")

Preferences,^tag(bl-preferences)
Back,^back()
Openbox,^checkout(bl-obConfig)
Compositor,^pipe(jgmenu_run ob --cmd="bl-compositor" --tag="bl-compositor")
Conky,^pipe(jgmenu_run ob --cmd="bl-conky-pipemenu" --tag="bl-conky-pipemenu")
Tint2,^pipe(jgmenu_run ob --cmd="bl-tint2-pipemenu" --tag="bl-tint2-pipemenu")
Appearance,lxappearance
Font configuration,bl-text-editor ~/.config/fontconfig/fonts.conf
BLOB Themes Manager,bl-obthemes
Wallpaper,nitrogen
Notifications,xfce4-notifyd-config
Power Management,xfce4-power-manager-settings
dmenu,^checkout(bl-dmenuconfig)
gmrun,^checkout(bl-gmrunconfig)
Display,^checkout(bl-DisplaySettings)

Openbox,^tag(bl-obConfig)
Back,^back()
Edit menu.xml,bl-text-editor ~/.config/openbox/menu.xml
Edit rc.xml,bl-text-editor ~/.config/openbox/rc.xml
Edit autostart,bl-text-editor ~/.config/openbox/autostart
^sep()
Menu Editor,obmenu
WM Preferences,obconf
How to Edit Menu,yad --button="OK":0 --center --window-icon=distributor-logo-bunsenlabs --text-info --title="How to Edit the Menu" --filename="/usr/share/bunsen/docs/helpfile-menu.txt" --width=900 --height=700 --fontname=Monospace
^sep()
Reconfigure,openbox --reconfigure
Restart,openbox --restart

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
Printers,^pipe(jgmenu_run ob --cmd="bl-printing-pipemenu" --tag="bl-printing-pipemenu")
Task Manager (htop),x-terminal-emulator -T 'htop task manager' -e htop
Synaptic Package Manager,pkexec synaptic
Login Settings,pkexec bl-text-editor /etc/lightdm/lightdm-gtk-greeter.conf /etc/lightdm/lightdm.conf
GParted,pkexec gparted
Edit Debian Alternatives,galternatives
^sep()
About Bunsen Alternatives,yad --button="OK":0 --center --window-icon=distributor-logo-bunsenlabs --text-info --title="About Bunsen Alternatives" --filename="/usr/share/bunsen/docs/helpfile-bl-alternatives.txt" --width=900 --height=700 --fontname=Monospace

^tag(lx-apps)
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

printf "%b\n" "not a valid theme"
usage

