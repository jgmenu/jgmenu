#!/bin/sh

config_file=~/.config/jgmenu/jgmenurc

usage () {
	printf "usage: jgmenu_run config --get <key>\n"
}

# $1 - key
get_variable () {
	test -z $1 && exit 1
	cat ${config_file} | grep -i ${1} | awk -F= '{printf $2}' | tr -d " "
	printf "\n"
}

case "$1" in
--get)
	get_variable "^$2" ;;
--help)
	usage
	exit 0
	;;
esac

