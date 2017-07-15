#!/bin/sh

(
printf "%b\n" 'root,^tag(root)'
printf "%b\n" 'a,^checkout(a)'
printf "%b\n" 'b,^checkout(b)'
printf "%b\n" 'c (width check),^checkout(c)'
printf "%b\n" 'a,^tag(a)'
printf "%b\n" 'aa,^checkout(aa)'
printf "%b\n" 'ab,^checkout(ab)'
printf "%b\n" 'a1'
printf "%b\n" 'a2'
printf "%b\n" 'a3'
printf "%b\n" 'aa,^tag(aa)'
printf "%b\n" 'aa1'
printf "%b\n" 'aa1'
printf "%b\n" 'ab,^tag(ab)'
printf "%b\n" 'ab1'
printf "%b\n" 'ab1'
printf "%b\n" 'b,^tag(b)'
printf "%b\n" 'b1'
printf "%b\n" 'b2'

printf "%b\n" 'c,^tag(c)'
printf "%b\n" 'c1 - e abcde,^checkout(cc)'

printf "%b\n" 'cc,^tag(cc)'
printf "%b\n" 'cc1 - t abcdefghijklmnopqrst,^checkout(ccc)'

printf "%b\n" 'ccc,^tag(ccc)'
printf "%b\n" 'ccc1 - n abcdefghijklmnopqrstuvwxyzabcdefghijklmn,^checkout(cccc)'

printf "%b\n" 'cccc,^tag(cccc)'
printf "%b\n" 'cccc1 - s abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrs,^checkout(ccccc)'

printf "%b\n" 'ccccc,^tag(ccccc)'
printf "%b\n" 'ccccc1 - abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrs,^checkout(cccccc)'

printf "%b\n" 'cccccc,^tag(cccccc)'
printf "%b\n" 'cccccc1 - abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz,^checkout(cccccc)'

) | ../jgmenu --simple --config-file=./ex03/jgmenurc
