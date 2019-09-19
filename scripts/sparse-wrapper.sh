#!/bin/sh

if ! test -e src/jgmenu.c ; then
	printf '%b\n' "not run from top level directory of jgmenu repo"
	exit 1
fi

export CC="cgcc"
export CFLAGS="-Wsparse-all -Wno-transparent-union"

make
