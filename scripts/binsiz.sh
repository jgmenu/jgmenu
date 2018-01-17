#!/bin/sh

f=binsiz.log

./scripts/version-gen.sh >>$f
printf "\n" >>$f
ls -l jgmenu jgmenu-lx jgmenu-ob jgmenu-xdg | awk '{ print $5,$9 }' >>$f
