#!/bin/sh

#
# DESCRIPTION: Adds full path to icon name in CSV file
#	I.e. it changes "foo,bar,folder" to:
#	"foo,bar,/usr/share/icons/Faenza/places/22/folder.png"
#
#	The script is primarily for perfance testing
#
# USAGE:
#	jgmenu_run pmenu >slow.csv
#	./scripts/expand-iconname.sh slow.csv >fast.csv
#	jgmenu_run csv fast.csv
#

if test $# -lt 1
then
	printf "%b\n" "Usage: expand-iconname <filename>"
	exit 1
fi

while IFS= read -r line
do
	name=$(echo ${line} | awk -F, '{ print $1 }')
	cmd=$(echo ${line} | awk -F, '{ print $2 }')
	icon=$(echo ${line} | awk -F, '{ print $3 }')

	if ! test -z "${name}"
	then
		printf "%b" "${name},${cmd}"
	else
		printf "\n"
		continue
	fi

	if ! test -z "${icon}"
	then
		printf "%b" ","
		jgmenu_run icon-find --theme=Faenza-Dark --icon-size=22 ${icon}
	else
		printf "\n"
		continue
	fi

done <$1
