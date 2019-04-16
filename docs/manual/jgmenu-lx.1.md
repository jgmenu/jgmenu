% JGMENU-LX(1)  
% Johan Malm  
% 16 Apr, 2019

# NAME

jgmenu-lx - generate jgmenu flavoured CSV menu data for an XDG menu  

# SYNOPSIS

`jgmenu_run lx`

# DESCRIPTION

`jgmenu_run lx` generates jgmenu flavoured CSV menu data for  
freedesktop.org defined application menus, using LXDE's  
libmenu-cache.

# ENVIRONMENT VARIABLES  

`XDG_MENU_PREFIX`
:   This can be used to specity a .menu file. For example,  
       `XDG_MENU_PREFIX=lxde-` will load lxde-applications.menu  

JGMENU_NO_DIRS
:   If set, applications will be listed without a directory  
       structure  

JGMENU_NO_PEND
:   Do not read append.csv or prepend.csv  

# KNOWN ISSUES

xfce-applications.menu is not parsed correctly.

# EXAMPLES

This program is designed to be run from jgmenu by settings  
`csv_cmd = lx` in the configuration file (jgmenurc).  

It can also be run directly from the command line like this:  

    jgmenu_run lx | jgmenu --simple

    jgmenu --csv-cmd="jgmenu_run lx"
