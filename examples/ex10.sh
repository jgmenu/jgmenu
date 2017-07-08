#!/bin/sh

tint2dirs="/usr/share/tint2 /usr/local/share/tint2"
tint2dir=

for d in ${tint2dirs}
do
	if test -d ${d}
	then
		tint2dir=${d}
		break;
	fi
done
echo ${tint2dir}
test -z ${tint2dir} && { echo "fatal: cannot find tint2rc files"; exit 1; }
cd ${tint2dir}

unset TINT2_BUTTON_ALIGNED_X1
unset TINT2_BUTTON_ALIGNED_X2
unset TINT2_BUTTON_ALIGNED_Y1
unset TINT2_BUTTON_ALIGNED_Y2
unset TINT2_BUTTON_PANEL_X1
unset TINT2_BUTTON_PANEL_X2
unset TINT2_BUTTON_PANEL_Y1
unset TINT2_BUTTON_PANEL_Y2
unset TINT2_CONFIG

pgrep tint2 >/dev/null && printf "%b\n" "Your current tint2 will be killed"
pgrep jgmenu >/dev/null && printf "%b\n" "Your current jgmenu will be killed"
printf "%b\n" "Use F10 to exit menus"

for f in ./*.tint2rc
do
	printf "%b\n" "${tint2dir}/${f##./}"
	printf "%b\n" "Press ENTER to run tint2+jgmenu, or CTRL+C to abort"
	read a
	killall tint2 2>/dev/null
	killall jgmenu 2>/dev/null
	nohup tint2 -c ${f} 2>/dev/null &
	TINT2_CONFIG=${f} jgmenu --csv-cmd="jgmenu_run parse-pmenu" \
				 --config-file=i_dont_exist 2>/dev/null
done


killall tint2 2>/dev/null
killall jgmenu 2>/dev/null
nohup tint2 &
