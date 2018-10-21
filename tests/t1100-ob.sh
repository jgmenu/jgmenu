#!/bin/sh

test_description='test openbox menu parser'
. ./test-lib-simple.sh

test_ob() {
	cat t1100/"$2" >expect
	../jgmenu-ob t1100/"$1" >actual
	test_cmp expect actual
}

test_expect_success 'test openbox menu1' '
test_ob "menu1.xml" "menu1.csv"
'

