#!/bin/sh

if test $# -lt 1
then
	cmd="./jgmenu --die-when-loaded"
else
	cmd="$@"
fi

valgrind --leak-check=full \
	 --leak-resolution=high \
	 --num-callers=20 \
	 --log-file=valgrind.log \
	 --suppressions=scripts/valgrind.supp \
	 --gen-suppressions=all \
	 ${cmd}

printf "nr_lines="; cat valgrind.log | wc -l
