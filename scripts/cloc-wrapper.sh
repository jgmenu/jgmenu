#!/bin/sh

cloc . -not-match-d="tests noncore" \
	| grep '^C \|^Bourne\|^Python' \
	| awk '{ print $1, $5 }' 2>/dev/null
