#!/bin/sh

value=$(grep "^menu-opacity" ~/.config/compton.conf)
value=${value#*=}
value=${value%;}

: $(( value *= 100 ))

printf '%b\n' "Set menu background to 000000 $value"
jgmenu_run config -s ~/.config/jgmenu/jgmenurc -k color_menu_bg -v "000000 $value"
