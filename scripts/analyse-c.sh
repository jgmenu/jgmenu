#!/bin/sh

#
# Look for C functions which can cause problems if used carelessly.
#

funcs="sprintf strcpy strncpy strtok strcat strncat scanf"

for i in ${funcs}
do
	git grep -n ${i} -- *.c
done
