#!/bin/sh

applications_dir="${XDG_DATA_HOME:-$HOME/.local/share}/applications"
desktop_file="${applications_dir}/${1}"

_exit () {
	printf "%b\n" "$@"
	exit 0
}

test "$#" != 1 && _exit "usage: jgmenu_run hide-app <.desktop file>"
test -e "${desktop_file}" && _exit "info: file already exists"

mkdir -p "${applications_dir}"
printf "%b\n" "[Desktop Entry]" >>"${desktop_file}"
printf "%b\n" "Type=Application" >>"${desktop_file}"
printf "%b\n" "NoDisplay=true" >>"${desktop_file}"

rm -rf "${HOME}/.cache/menus/"
