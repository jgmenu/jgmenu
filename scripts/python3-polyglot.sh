#!/bin/sh

#
# This script is used by 'make'
#
# Usage: python3-polyglot.sh <path> <files>...
#

polyglot="#!/bin/sh\n\
''''which python3.9 >/dev/null 2>&1 && exec python3.9 \"\$0\" \"\$@\" # '''\n\
''''which python3.8 >/dev/null 2>&1 && exec python3.8 \"\$0\" \"\$@\" # '''\n\
''''which python3.7 >/dev/null 2>&1 && exec python3.7 \"\$0\" \"\$@\" # '''\n\
''''which python3.6 >/dev/null 2>&1 && exec python3.6 \"\$0\" \"\$@\" # '''\n\
''''which python3.5 >/dev/null 2>&1 && exec python3.5 \"\$0\" \"\$@\" # '''\n\
''''which python3.4 >/dev/null 2>&1 && exec python3.4 \"\$0\" \"\$@\" # '''\n\
''''exec printf 'fatal: cannot find python3 binary' # '''"

check_file_exists () {
	if ! test -e "$1"
	then
		printf "fatal: file '%s' does not exist\n" "$1"
		exit 1
	fi
}

if test $# -lt 2
then
	printf "$0: fatal: no argument specified\n"
	exit 1
fi

dir=$1
shift

while test $# != 0
do
	file_name="$1"
	printf "processing file: %s\n" "${dir}/${file_name}"
	check_file_exists "${dir}/${file_name}"

	sed "1s|#!.*python3.*||" "${dir}/${file_name}" >"${dir}/${file_name}.tmp"
	printf "%b" "${polyglot}" > ${dir}/${file_name}
	cat ${dir}/${file_name}.tmp >> ${dir}/${file_name}
	rm -f ${dir}/${file_name}.tmp

	shift
done
