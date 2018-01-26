#!/bin/sh
#
# This script is used by 'make'
# Usage: create_desktop_file.sh <prefix>
#
# Define JGMENU_DESKTOP_EXEC and JGMEN_DESKTOP_ICON to suit.
#
# If you are going to use the .desktop file on Ubuntu's Unity launcher, set
# JGMENU_DESKTOP_EXEC="env JGMENU_UNITY=1 jgmenu_run <command>"
#

if test $# -lt 1
then
	printf "$0: fatal: no argument specified\n"
	exit 1
fi

#
# In order to stay XDG compliant, we don't install to
# ~/share/applications/
#
if test $1 = "$HOME"
then
	desktop_dir=~/.local/share/applications
else
	desktop_dir="$1/share/applications"
fi

test -e ./config.mk && . ./config.mk

: ${desktop_file="jgmenu.desktop"}
: ${JGMENU_DESKTOP_EXEC="jgmenu_run"}
: ${JGMENU_DESKTOP_ICON="start-here"}

mkdir -p ${desktop_dir}

dest=${desktop_dir}/${desktop_file}

printf "%b\n" "[Desktop Entry]" >${dest}
printf "%b\n" "Type=Application" >>${dest}
printf "%b\n" "Encoding=UTF-8" >>${dest}
printf "%b\n" "Name=jgmenu" >>${dest}
printf "%b\n" "GenericName=Application Menu" >>${dest}
printf "%b\n" "GenericName[sv]=Program Meny" >>${dest}
printf "%b\n" "GenericName[ru]=Меню приложений" >>${dest}
printf "%b\n" "Comment=Displays menu for launching installed applications" >>${dest}
printf "%b\n" "Comment[sv]=Visar meny för installerade program" >>${dest}
printf "%b\n" "Comment[ru]=Отображает меню для запуска установленных приложений" >>${dest}
printf "%b\n" "Exec=${JGMENU_DESKTOP_EXEC}" >>${dest}
printf "%b\n" "Terminal=false" >>${dest}
printf "%b\n" "Icon=${JGMENU_DESKTOP_ICON}" >>${dest}
printf "%b\n" "Categories=Menu" >>${dest}
printf "%b\n" "MimeType=" >>${dest}
printf "%b\n" "StartupNotify=false" >>${dest}
