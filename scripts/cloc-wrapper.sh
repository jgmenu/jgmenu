#!/bin/sh

versions="master v0.8 v0.7 v0.6 v0.5 v0.4 v0.3 v0.2 v0.1.2"

for v in $versions
do
	git checkout "${v}"
	printf "%b\n" "version ${v}"
	cloc . -not-match-d="tests noncore" \
		| grep '^C \|^Bourne\|^Python' \
		| awk '{ print $1, $5 }' 2>/dev/null
done
