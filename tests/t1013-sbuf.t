#!/bin/sh

test_description='test string buffer routines'
. ./sharness.sh

test_sbuf() {
	echo "$1" | ../helper/test-sbuf > actual &&
	echo "$2" > expect &&
	test_cmp expect actual
}

test_expect_success 'addstr' '

test_sbuf "addstr ABCDEFGHIJKLMNO
print
len
bufsiz
addstr P
print
len
bufsiz
" "ABCDEFGHIJKLMNO
15
16
ABCDEFGHIJKLMNOP
16
17"

'

test_expect_success 'prepend' '

test_sbuf "cpy bar
print
len
bufsiz
prepend foo
print
len
bufsiz
" "bar
3
4
foobar
6
10"

'

test_done
