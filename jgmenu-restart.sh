#!/bin/sh

cmd=$(jgmenu_run config --get csv_cmd)
test -z "${cmd}" && cmd="jgmenu_run parse-pmenu"

killall jgmenu
/bin/sh -c "${cmd} | jgmenu --stay-alive --hide-on-startup"
