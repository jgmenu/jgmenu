#!/bin/sh

test -e $PWD/ex-sh-setup.sh && . ${PWD}/ex-sh-setup.sh 2>/dev/null || { echo "fatal: not run from examples/"; exit 1; }

get_LANG
get_theme

cp ex13/append.csv /tmp/jgmenu/
cp ex13/prepend.csv /tmp/jgmenu/

cat ex13/jgmenurc | sed -e "s/.*icon_theme\s*=.*/icon_theme=$theme/" \
	> /tmp/jgmenu/jgmenurc

export TINT2_CONFIG=${PWD}/ex13/tint2rc
restart_tint2 ${TINT2_CONFIG}
jgmenu_run pmenu --append-file /tmp/jgmenu/append.csv \
		 --prepend-file /tmp/jgmenu/prepend.csv \
		 | jgmenu --config-file=/tmp/jgmenu/jgmenurc

restart_tint2
