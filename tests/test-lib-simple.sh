#!/bin/sh
#
# Hughly simplified version of git's test-lib.sh
# Included to test hashmap.c
#
# Test should start like this:
# test_description='Description of this test'
# . ./test-lib-simple.sh

test_count=0

while test "$#" -ne 0
do
	case "$1" in
	-v|--verbose)
		verbose=t ;;
	*)
		break ;;
	esac
	shift
done

test -z "${test_description}" && die "\$test_description not set"
printf "%b\n" "${test_description}"

if test "$verbose" = "t"
then
        exec 4>&2 3>&1
else
        exec 4>/dev/null 3>/dev/null
fi

die () {
	printf "fatal: %b\n" "$*"
	exit 1
}

say () {
	echo "$*"
}

test_cmp() {
        diff -u "$@"
}

test_ok_ () {
	: $(( test_count = test_count + 1 ))
	say "ok ${test_count}: $@"
}

test_failure_ () {
	: $(( test_count = test_count + 1 ))
	say "not ok $test_count: $1"
	shift
	echo "$@" | sed -e 's/^/	/'
}

test_run_ () {
        eval >&3 2>&4 "$1"
        return $?
}

test_expect_failure () {
	test "$#" = 2 || die "test script not 2 parameters"
	say >&3 "expecting failure: $2"
	if ! test_run_ "$2"
	then
		test_ok_ "$1"
	else
		test_failure_ "$@"
	fi
}

test_expect_success () {
	test "$#" = 2 || die "test script not 2 parameters"
	say >&3 "expecting success: $2"
	if test_run_ "$2"
	then
		test_ok_ "$1"
	else
		test_failure_ "$@"
	fi
}

