% JGMENU-LX(1)  
% Johan Malm  
% 22 December, 2017

# NAME

jgmenu-lx - generates jgmenu flavoured CSV for an XDG menu

# SYNOPSIS

`jgmenu_run lx`

# DESCRIPTION

`jgmenu_run lx` generates jgmenu flavoured CSV text for  
freedesktop.org defined application menus, using LXDE's  
libmenu-cache.

`$XDG_MENU_PREFIX` can be used to specity a .menu file. For example  
`$XDG_MENU_PREFIX=lxde-` will load lxde-applications.menu  

# KNOWN ISSUES

xfce-applications.menu is not parsed correctly.

# EXAMPLES

This program is designed to be run from jgmenu by settings  
`csv_cmd = lx` in the configuration file (jgmenurc).  

It can also be run directly from the command line like this:  

    jgmenu_run lx | jgmenu

    jgmenu --csv-cmd="jgmenu_run lx"
