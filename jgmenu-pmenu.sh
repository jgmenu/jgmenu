#!/bin/sh

test -z ${JGMENU_UNITY} || jgmenu_run unity-hack &

(
jgmenu-parse-pmenu.py --append-file=$HOME/.config/jgmenu/append.csv \
		      --prepend-file=$HOME/.config/jgmenu/prepend.csv
) | jgmenu $@
