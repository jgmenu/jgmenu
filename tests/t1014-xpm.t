#!/bin/sh

test_description='test xpm loader'

. ./sharness.sh

if ! type cmp >/dev/null 2>&1 
then
     skip_all='cmp is required'
     test_done
fi

rm -f ../t0008/*.png
../helper/test-xpm ../t0008/*.xpm 2>/dev/null

for s in gimp mc python
do
	test_expect_success "${s}.xpm" "cmp ../t0008/${s}.expect ../t0008/${s}.xpm.png"
done

test_done
