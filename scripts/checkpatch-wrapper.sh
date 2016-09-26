#!/bin/sh

cmd_args="--file --terse --no-tree --max-line-length=120"
cmd_args="${cmd_args} --strict"

if [[ $# -lt 1 ]]
then
	echo -e "Fatal: no file specified"
	exit 1
fi

for i in "$@"
do
	if test ${i} != "list.h"
	then
		printf "Checking ${i}\n"
		perl scripts/checkpatch.pl ${cmd_args} "${i}" \
			| grep -v 'CamelCase' \
			| grep -v -i 'extern prototypes should be avoided' \
			| grep -v '^total'
	fi
done

rm -f .checkpatch-camelcase.git. 2>/dev/null
