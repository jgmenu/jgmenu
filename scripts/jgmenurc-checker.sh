#!/bin/sh
#
# Check jgmenurc files within the jgmenu repo for
#   - completeness
#   - options that are no longer valid
#

files="\
	noncore/config/jgmenurc \
	noncore/init/jgmenurc.neon \
	noncore/init/jgmenurc.bunsenlabs_hydrogen \
	noncore/init/jgmenurc.archlabs_1803"

printf "%b\n" "*** check config files for completeness ***"

for f in ${files}
do
	printf "%b\n" "check: '${f}'"
	if ! test -e ${f}
	then
		printf "%b\n" "info: file '${f}' does not exist"
		continue
	fi
	jgmenu_run config amend --file="${f}"
done

printf "%b\n" "*** check config files for options that are no longer valid ***"

for f in $(find . -name "*jgmenurc*")
do
	printf "%b\n" "check: '${f}'"
	jgmenu_run init --config-file="${f}" --regression-check
done
