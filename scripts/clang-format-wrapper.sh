#!/bin/bash

iterators=(
	list_for_each_entry
	list_for_each_entry_from
	list_for_each_entry_reverse
	list_for_each_entry_safe
	list_for_each_entry_safe_from
	list_for_each_entry_safe_reverse
)

strip_space_before_foreach_iterators () {
	local s
	for ((i=0;i<${#iterators[@]};i++)); do
		s=${iterators[i]}
		content=${content//$s (/$s(}
	done
}

format_file () {
	echo $1
	content=$(clang-format $1)
	strip_space_before_foreach_iterators
	printf '%b\n' "${content}" >tmp
	diff $1 tmp
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
