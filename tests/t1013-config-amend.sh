#!/bin/sh

if ! test -e ./helper/filter-out
then
	echo "fatal: need ./helper/filter-out"
	exit 1
fi

test_description='test jgmenu-config.py amend'
. ./test-lib-simple.sh

filter_out () {
	cat ../noncore/config/jgmenurc | ./helper/filter-out "$@" >actual
}

test_config_amend() {
	cat ../noncore/config/jgmenurc >expect
	filter_out $1
	../noncore/config/jgmenu-config.py amend --file actual
	test_cmp expect actual
}

test_expect_success 'filter out lines 14 16 17' '
test_config_amend "14 16 17"
'

# The first three lines lines are comments, so won't replace those
test_expect_failure 'filter out comment lines 1 2 3' '
test_config_amend "1 2 3"
'

test_expect_success 'filter out lines 5 6 7' '
test_config_amend "5 8 10"
'
