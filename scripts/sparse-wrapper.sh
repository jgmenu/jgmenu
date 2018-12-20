#!/bin/sh

test -e jgmenu.c || die "not run from top level directory of jgmenu repo"
export CC="cgcc"
export CFLAGS="-Wsparse-all -Wno-transparent-union"
make
