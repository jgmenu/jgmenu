#!/bin/sh

(
jgmenu-parse-pmenu.py --append-file=$HOME/.config/jgmenu/append.csv \
		      --prepend-file=$HOME/.config/jgmenu/prepend.csv
) | jgmenu $@
