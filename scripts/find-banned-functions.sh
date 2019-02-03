#!/bin/sh

#
# Search for functions which have been banned from our code base,
# because they're too easy to misusei, and even if used correctly,
# complicate audits.
#

funcs="sprintf vsprintf strcpy strncpy strcat strncat"

for i in ${funcs}
do
	git grep -n ${i} -- *.c
done
