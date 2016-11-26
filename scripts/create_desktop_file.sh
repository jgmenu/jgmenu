#!/bin/sh

# Define JGMENU_DESKTOP_EXEC and JGMEN_DESKTOP_ICON to suit.

# If you are going to use the .desktop file on Ubuntu's Unity launcher, set
# JGMENU_DESKTOP_EXEC="env JGMENU_UNITY=1 jgmenu_run <command>"

: ${desktop_dir=~/.local/share/applications}
: ${desktop_file="jgmenu.desktop"}

test -z "${JGMENU_DESKTOP_EXEC}" && JGMENU_DESKTOP_EXEC="jgmenu_run pmenu"
test -z "${JGMENU_DESKTOP_ICON}" && JGMENU_DESKTOP_ICON="start-here"

dest=${desktop_dir}/${desktop_file}

printf "%b\n" "[Desktop Entry]" >${dest}
printf "%b\n" "Encoding=UTF-8" >>${dest}
printf "%b\n" "Name=jgmenu" >>${dest}
printf "%b\n" "X-GNOME-FullName=Menu" >>${dest}
printf "%b\n" "Exec=${JGMENU_DESKTOP_EXEC}" >>${dest}
printf "%b\n" "Terminal=false" >>${dest}
printf "%b\n" "X-MultipleArgs=false" >>${dest}
printf "%b\n" "Type=Application" >>${dest}
printf "%b\n" "Icon=${JGMENU_DESKTOP_ICON}" >>${dest}
printf "%b\n" "Categories=Menu" >>${dest}
printf "%b\n" "MimeType=" >>${dest}
printf "%b\n" "StartupNotify=false" >>${dest}

