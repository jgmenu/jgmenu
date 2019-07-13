#!/bin/sh

test_description='test xpm loader'

. ./sharness.sh

if ! type cmp >/dev/null 2>&1 
then
     skip_all='cmp is required'
     test_done
fi

rm -f ../t1014/*.png
../helper/test-xpm ../t1014/*.xpm 2>/dev/null

for s in gimp mc python
do
	test_expect_success "${s}.xpm" "cmp ../t1014/${s}.expect ../t1014/${s}.xpm.png"
done
rm -f ../t1014/*.png

test_done
