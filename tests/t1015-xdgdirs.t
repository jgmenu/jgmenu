#!/bin/sh

test_description='test xdg directories'
. ./sharness.sh

test_xdgdirs () {
	echo "$1" | ../helper/test-xdgdirs > actual &&
	echo "$2" > expect &&
	test_cmp expect actual
}

test_expect_success 'XDG_CONFIG_DIRS with colon separated directories' '

test_xdgdirs "dirs /etc/xdg1:/etc/xdg2
" "/etc/xdg1@/etc/xdg2@/etc/xdg@"

'

test_done
