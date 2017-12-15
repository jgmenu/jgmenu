#!/bin/sh

test -e $PWD/ex-sh-setup.sh && . ${PWD}/ex-sh-setup.sh 2>/dev/null || { echo "fatal: not run from examples/"; exit 1; }

get_LANG
get_theme

cp ex11/append.csv /tmp/jgmenu/
cp ex11/prepend.csv /tmp/jgmenu/
cp ex11/flame-light.svg /tmp/jgmenu/

cat ex11/tint2rc | sed -e "s/.*launcher_icon_theme\s*=.*/launcher_icon_theme = $theme/" \
	> /tmp/jgmenu/tint2rc

export TINT2_CONFIG=/tmp/jgmenu/tint2rc
restart_tint2 ${TINT2_CONFIG}
jgmenu_run pmenu --append-file /tmp/jgmenu/append.csv \
		 --prepend-file /tmp/jgmenu/prepend.csv \
		 | jgmenu --config-file=/tmp/jgmenu/jgmenurc

restart_tint2
