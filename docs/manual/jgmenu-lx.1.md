% JGMENU-LX(1)
% Johan Malm
% 29 September, 2019

# NAME

jgmenu-lx - generate jgmenu flavoured CSV menu data

# SYNOPSIS

`jgmenu_run lx`

# DESCRIPTION

`jgmenu_run lx` generates jgmenu flavoured CSV menu data for freedesktop.org
defined application menus, using LXDE's libmenu-cache.

# ENVIRONMENT VARIABLES

`XDG_MENU_PREFIX`

:   This can be used to specity a .menu file. For example,
    `XDG_MENU_PREFIX=lxde-` will load lxde-applications.menu

`JGMENU_NO_DIRS`

:   If set, applications will be listed without a directory
    structure

`JGMENU_NO_PEND`

:   Do not read append.csv or prepend.csv

`JGMENU_NAME_FORMAT`

:   See `csv_name_format` in jgmenu(1)

# KNOWN ISSUES

xfce-applications.menu is not parsed correctly.
