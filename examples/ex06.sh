#!/bin/sh


for i in $(seq 100)
do
	echo "item ${i},\"\"\"^pipe(for j in \$(seq 100); do echo "item \$j"; done) \"\"\" "
done | ../jgmenu --vsimple --at-pointer

