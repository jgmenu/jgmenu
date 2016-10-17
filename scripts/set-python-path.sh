#!/bin/sh

#
# This script is used by 'make'
#
# Usage: set-python-path.sh <python-file-without-dot-py>
#

executables="python2 python python2.7"
shebang_string=

if test $# -lt 1
then
	printf "fatal: set-python-path.sh: no argument specified\n"
	exit 1
fi

file_name=$1
if ! test -e "${file_name}.py"
then
	printf "fatal: file '%s' does not exist\n" "${file_name}.py"
	exit 1
fi

for i in ${executables}
do
	shebang_string="/usr/bin/${i}"
	if type ${shebang_string} >/dev/null 2>&1
	then
		break
	fi
done

sed -e "1s/#!.*python/#!${shebang_string}/" "${file_name}.py" >"${file_name}"
chmod +x ${file_name}
