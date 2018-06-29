#!/bin/sh
#
# Inspired by the {a,b}l-places-pipemenu scripts found in
# ArchLabs and BunsenLabs
#

: ${JGMENU_RESOURCE_OPENER=xdg-open}

path=${1:-$HOME}
path=${path%/}

test -d "$path" || { echo "$0: '$path' is not a directory" >&2 ; exit 1 ; }

for i in $path/*
do
	test -e "$i" || continue
	shortname=${i##*/}
	if test -d "$i"
	then
		directories_menu="${directories_menu}
${shortname},^pipe(jgmenu_run places ${path}/${shortname})"
	else
		files_menu="$files_menu
${shortname},${JGMENU_RESOURCE_OPENER} ${path}/${shortname}"
	fi
done

printf "%b\n" "Browse here...,${JGMENU_RESOURCE_OPENER} ${path}"
printf "%b\n" "^sep()"
printf "%b\n" "${directories_menu}"
printf "%b\n" "${files_menu}"

if test -z "${directories_menu}" && test -z "${files_menu}"
then
	printf "%b\n" '<empty>'
fi
