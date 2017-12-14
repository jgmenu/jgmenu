#!/bin/sh

config_file=~/.config/jgmenu/jgmenurc

usage () {
	printf "usage: jgmenu_run config --get <key>\n"
	printf "       jgmenu_run config --set <key> <value>\n"
}

# $1 - key
get_variable () {
	test -z "$1" && exit 1
	test -e ${config_file} || exit 1
	v=$(cat ${config_file} 2>/dev/null | grep -i ${1} | awk -F= '{printf $2}')
	# Using @ as sed delimitor as | and / might be used in $v
	v_trimmed=$(printf "%s" "${v}" | sed -e 's@^\s*@@' -e 's@\s*$@@')
	printf "%s\n" "${v_trimmed}"
}

# $1 - key
# $2 - value
set_variable () {
	if grep "^$1" ${config_file} >/dev/null
	then
		sed "s|^$1.*|$1 = $2|" "${config_file}" >"${config_file}.tmp"
		cp -p ${config_file}.tmp ${config_file}
		rm -f ${config_file}.tmp
	else
		printf "%b\n" "$1 = $2" >> ${config_file}
	fi
}

case "$1" in
--get)
	get_variable "^$2" ;;
--set)
	set_variable "$2" "$3" ;;
--help)
	usage
	exit 0
	;;
esac

