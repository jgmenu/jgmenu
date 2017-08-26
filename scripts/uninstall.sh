#!/bin/sh

rm -f ~/bin/jgmenu
rm -f ~/bin/jgmenu_run

rm -rf ~/lib/jgmenu/
rmdir ~/lib 2>/dev/null

rm -f ~/share/man/man1/jgmenu*
rm -f ~/share/man/man7/jgmenu*

rmdir -p ~/share/man/man1/ 2>/dev/null
rmdir -p ~/share/man/man7/ 2>/dev/null
