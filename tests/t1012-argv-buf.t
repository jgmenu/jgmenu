#!/bin/sh

test_description='test argv-buf'
. ./sharness.sh

test_argvbuf() {
	echo "$1" | ../helper/test-argv-buf > actual &&
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
argc" "a@b@@d d@e@f-f
6"

'

test_expect_success 'simple triple quote' '

test_argvbuf "strdup@\"\"\"a,aa\"\"\",\"\"\"b,bb\"\"\",\"\"\"c,cc\"\"\"
parse
print
argc" "a,aa@b,bb@c,cc
3"

'

test_expect_success 'triple quote with missing end quotes' '

printf "%b\n" "strdup@aaa,\"\"\"b,bb,ccc\nparse\nprint\nargc" >expect
test_must_fail ../helper/test-argv-buf <expect

'

test_done
