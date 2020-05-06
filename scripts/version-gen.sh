#!/bin/sh

#
# version-gen.sh is a wrapper for `git describe` and is used by the Makefile.
# It gives a version number even when there is no git repo, such as in a
# release tarball.
#
# Change "default_version" before doing a `git tag -a ...`
#

default_version="4.2.0"

v=$(git describe --dirty --abbrev=1 2>/dev/null)

if ! test -z ${v}
then
	printf "jgmenu %s" ${v}
else
	printf "jgmenu v%s" ${default_version}
fi
