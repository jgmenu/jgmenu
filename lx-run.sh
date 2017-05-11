#!/bin/sh

gcc -g -Wall `pkg-config --cflags --libs glib-2.0 libmenu-cache` \
    -o jgmenu-parse-lx jgmenu-parse-lx.c sbuf.c util.c || exit 1

killall jgmenu >/dev/null
#export XDG_MENU_PREFIX=lxde-
export XDG_MENU_PREFIX=lxqt-
#export XDG_MENU_PREFIX=xfce-
#export XDG_MENU_PREFIX=gnome-
./jgmenu-parse-lx | jgmenu
