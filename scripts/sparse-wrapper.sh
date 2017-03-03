#!/bin/sh

makefile_backup=Makefile.sparse

sparse_options="-Wsparse-all -Wno-transparent-union"

die () {
	echo "fatal: $1"
	exit 1
}


type sparse >/dev/null 2>&1 || die "you need to install sparse before running this"
test -e jgmenu.c || die "not run from top level directory of jgmenu repo"
test -e ${makefile_backup} && die "${makefile_backup} already exists" 

cp -p Makefile.inc ${makefile_backup}

sed --in-place "s/^CC.*/CC = cgcc ${sparse_options}/" Makefile.inc

make clean
make 2>&1 | grep -v 'glib-autocleanups.h'

cp -pf "${makefile_backup}" Makefile.inc
rm -f "${makefile_backup}"
