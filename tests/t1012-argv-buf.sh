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

test_expect_success 'complex parse' '

test_argvbuf "strdup@a,b,,d d, e ,f-f
parse
print
argc" "a@b@@d d@ e @f-f
6"

'

test_expect_success 'simple triple quote' '

test_argvbuf "strdup@aaa,\"\"\"b,bb\"\"\",ccc
parse
print
argc" "aaa@b,bb@ccc
3"

'

test_expect_failure 'triple quote with missing end quotes' '

test_argvbuf "strdup@aaa,\"\"\"b,bb,ccc
parse
print
argc"

'
