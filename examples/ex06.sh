#!/bin/sh

(

echo "one,one"
echo "two,^checkout(foo)"

echo "foo,^tag(foo)"
for i in $(seq 100)
do
	echo "item ${i},\"\"\"^pipe(for j in \$(seq 100); do echo "item \$j"; done) \"\"\" "
done
) | ../jgmenu --vsimple --at-pointer

