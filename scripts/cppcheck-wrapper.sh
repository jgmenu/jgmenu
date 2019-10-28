#!/bin/sh
#
# Usage: cppcheck-wrapper.sh <file>

# Notes on suppressions
# - readdir_r is deprecated. See readdir_r(3)
# - We allow strtok if threadsafety is not a consideration
# - With the C89 kernel style, variableScope throws warnings, but not a problem
# - vsnprintf() is used in accordance with the man page, so not sure what the
#   issue is
# - list.h and hashmap.{c,h} are copied from git/linux, so it's easier to keep
#   them complete. It also makes the unit test work properly
suppressions="\
	--suppress=readdirCalled:src/desktop.c \
	--suppress=readdirCalled:src/icon-find.c \
	--suppress=missingIncludeSystem \
	--suppress=variableScope \
	--suppress=nullPointer:src/jgmenu-obtheme.c:58 \
	--suppress=unusedFunction:src/hashmap.c \
	--suppress=unusedFunction:src/hashmap.h \
	--suppress=unusedFunction:src/list.h"

enable="\
	--enable=warning \
	--enable=style \
	--enable=performance \
	--enable=portability \
	--enable=information \
	--enable=missingInclude"

# When the whole program is scanned, we can also check for unused functions
test $# -eq 0 && enable="${enable} --enable=unusedFunction"

if test $# -eq 0; then
	c=src/*.c
	h=src/*.h
	files="$c $h"
else
	files="$@"
fi

cppcheck \
	--inconclusive \
	-DVERSION=3.4 \
	-I src/ \
	--std=c99 \
	--std=posix \
	--library=/usr/share/cppcheck/cfg/gnu.cfg \
	--quiet \
	${enable} \
	${suppressions} \
	${files} \
	2>&1 \
	| grep -v 'Unmatched suppression'

