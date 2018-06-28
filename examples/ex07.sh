#!/bin/sh

(
printf "%b\n" 'item a'
printf "%b\n" 'item b'
printf "%b\n" 'sub1,^root(sub1)'
printf "%b\n" 'sub2,^root(sub2)'
printf "%b\n" ''
printf "%b\n" '^tag(sub1)'
printf "%b\n" 'back,^back()'
printf "%b\n" 'item 1.a'
printf "%b\n" 'item 1.b'
printf "%b\n" 'places-pipe,^pipe(jgmenu_run places)"'
printf "%b\n" ''
printf "%b\n" '^tag(sub2)'
printf "%b\n" 'back,^back()'
printf "%b\n" 'item 2.a'
printf "%b\n" 'item 2.b'
printf "%b\n" ''
) | XDG_DATA_HOME=$(pwd)/ex07 ../jgmenu --simple --config-file=./ex07/jgmenurc
