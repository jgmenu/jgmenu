#!/bin/sh

test_description='test argv-buf'
. ./test-lib-simple.sh

test_argvbuf() {
	echo "$1" | ./helper/test-argv-buf > actual &&
	echo "$2" > expect &&
	test_cmp expect actual
}

test_expect_success 'simple parse' '

test_argvbuf "strdup@aaa,bbb,ccc
parse
print
argc" "aaa@bbb@ccc
3"

'

test_expect_success 'complicated parse' '

test_argvbuf "strdup@a,b,,d d, e ,f-f
parse
print
argc" "a@b@@d d@ e @f-f
6"

'
