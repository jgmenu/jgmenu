#!/bin/sh

test -z ${JGMENU_UNITY} || jgmenu_run unity-hack &

jgmenu-parse-pmenu.py | jgmenu $@
