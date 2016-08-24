#!/bin/bash

menu_file="/etc/xdg/menus/gnome-applications.menu"

../jgmenu-xdg ${menu_file} | \
	../jgmenu --config-file=ex03-jgmenurc --icon-size=22
