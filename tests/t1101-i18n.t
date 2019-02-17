#!/bin/sh

test_description='test i18n'
. ./sharness.sh

# $1: menu file
# $2: po file
# $3: translated file
test_i18n() {
	cat t1101/"$3" >expect
	cat t1101/"$1" | ../jgmenu-i18n t1101/"$2" >actual
	test_cmp expect actual
}


test_expect_success 'test i18n module with Swedish po file' '
test_i18n "menu" "sv" "menu.sv"
'

test_done
