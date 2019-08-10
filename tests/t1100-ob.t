#!/bin/sh

test_description='test openbox menu parser'
. ./sharness.sh

test_ob() {
	cat ../t1100/"$2" >expect
	../../jgmenu-ob ../t1100/"$1" >actual
	test_cmp expect actual
}

test_ob_cmd() {
	cat ../t1100/"$2" >expect
	../../jgmenu-ob --cmd="cat ../t1100/$1" >actual
	test_cmp expect actual
}

test_expect_success 'test openbox module with file input (menu1.xml)' '
test_ob "menu1.xml" "menu1.csv"
'

test_expect_success 'test openbox module with command input (cat menu1.xml)' '
test_ob_cmd "menu1.xml" "menu1.csv"
'

test_expect_success 'test openbox module icon support (menu2.xml)' '
test_ob "menu2.xml" "menu2.csv"
'

test_expect_success 'test openbox module pipemenu with inline menus (menu3.xml)' '
test_ob "menu3.xml" "menu3.csv"
'

test_expect_success 'test openbox module pipemenu with inline menu as first node (menu4.xml)' '
test_ob "menu4.xml" "menu4.csv"
'

test_done
