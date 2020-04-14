#!/bin/bash

tmpfile=".tmp-clang-format"
g_insitu=f

iterators=(
	list_for_each_entry
	list_for_each_entry_from
	list_for_each_entry_reverse
	list_for_each_entry_safe
	list_for_each_entry_safe_from
	list_for_each_entry_safe_reverse
)

usage () {
	printf '%s\n' "\
Usage: clang-format-wrapper.sh [<file>]
Produces diff of changes"
}

strip_space_before_foreach_iterators () {
	local s
	for ((i=0;i<${#iterators[@]};i++)); do
		s=${iterators[i]}
		g_content=${g_content//$s (/$s(}
	done
}

format () {
	g_content=$(clang-format $1)
	strip_space_before_foreach_iterators
	printf '%s\n' "${g_content}" >$tmpfile
	diff $1 $tmpfile
	[[ $g_insitu = t ]] && mv -f $tmpfile $1
}

main () {
	for arg
	do
		opt=${arg%%=*}
		var=${arg#*=}
		case "$opt" in
		-h)	usage ;;
		-i)	g_insitu=t ;;
		*)	format "$opt" ;;
		esac
	done
}

main "$@"
