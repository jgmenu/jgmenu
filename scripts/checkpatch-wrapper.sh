#!/bin/sh

cmd_args="--file --terse --no-tree --max-line-length=120"
cmd_args="${cmd_args} --strict"

if test $# -lt 1
then
	printf "%s\n" "Fatal: no file specified"
	exit 1
fi

for i in "$@"
do
	if test ${i} != "src/list.h" && test ${i} != "src/xpm-color-table.h" &&
	   test ${i} != "src/flex-array.h"
	then
		perl scripts/checkpatch.pl ${cmd_args} "${i}"
	fi
done

rm -f .checkpatch-camelcase.git. 2>/dev/null
