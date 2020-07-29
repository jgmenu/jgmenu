#!/bin/sh

applications_dir="${XDG_DATA_HOME:-$HOME/.local/share}/applications"
desktop_file="${applications_dir}/${1}"

say () {
	printf '%b\n' "$@"
}

_exit () {
	say "$@"
	exit 0
}

test "$#" != 1 && _exit "usage: jgmenu_run hide-app <.desktop file>"
test -e "${desktop_file}" && _exit "info: file already exists"

mkdir -p "${applications_dir}"
say '[Desktop Entry]\nType=Application\nNoDisplay=true' >"${desktop_file}"

