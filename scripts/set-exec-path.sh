#!/bin/sh

#
# This script is used by 'make'
#
# Usage: set-exec-path.sh <file> <libexecdir>
#

if test $# -lt 2
then
	printf "$0: fatal: no argument specified\n"
	exit 1
fi

if ! test -e "$1"
then
	printf "fatal: file '%s' does not exist\n" "$1"
	exit 1
fi

cp -p $1 $1.tmp
sed "s|: \${JGMENU_EXEC_DIR=.*|: \${JGMENU_EXEC_DIR=$2}|" "$1.tmp" >"$1"
rm -f $1.tmp
