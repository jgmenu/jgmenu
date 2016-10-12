#!/bin/sh

printf "%s\n" "Checking shell scripts for bashisms..."

find . -name "*.sh" | xargs perl ./scripts/checkbashisms.pl --force 2>&1 \
	| grep -v "^could not find"

printf "%s\n" "Searching for files staring with '#!/bin/bash'..."

for f in $(find . -name "*.sh" | xargs)
do
	if cat ${f} | grep '^#!/bin/bash' >/dev/null 2>&1
	then
		printf "%b\n" "${f}"
	fi
done
