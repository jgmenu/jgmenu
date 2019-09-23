#!/bin/bash

tmpfile="clang-format-tmpfile"

iterators=(
	list_for_each_entry
	list_for_each_entry_from
	list_for_each_entry_reverse
	list_for_each_entry_safe
	list_for_each_entry_safe_from
	list_for_each_entry_safe_reverse
)

usage () {
	printf '%b' "\
Usage: clang-format-wrapper.sh [<file>]\n\
Produces diff of changes.\n\
"
}

strip_space_before_foreach_iterators () {
	local s
	for ((i=0;i<${#iterators[@]};i++)); do
		s=${iterators[i]}
		g_content=${g_content//$s (/$s(}
	done
}

format_file () {
	echo $1
	g_content=$(clang-format $1)
	strip_space_before_foreach_iterators
	printf '%s\n' "${g_content}" >$tmpfile
	diff $1 $tmpfile
}

format_all_files () {
	for f in ./src/*.c
	do
		format_file "$f"
	done
}

args () {
	case $1 in
		-h)	usage ;;
		*)	format_file "$1" ;;
	esac
}

main () {
	if [[ $# -gt 0 ]]; then
		args "$@"
	else
		format_all_files
	fi
}

main "$@"
