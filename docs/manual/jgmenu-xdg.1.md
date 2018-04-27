% JGMENU-XDG(1)  
% Johan Malm  
% 27 April, 2018

# NAME

jgmenu-xdg.sh - generate jgmenu flavoured CSV menu data for an XDGish  
                menu  

# SYNOPSIS

`jgmenu_run xdg` \[--no-dirs] \[<*.menu file*>]

# DESCRIPTION

`jgmenu_run xdg` generates jgmenu flavoured CSV menu data for a menu  
based on XML .menu files loosely in accordance with the XDG spec:  

http://standards.freedesktop.org/menu-spec/  
http://standards.freedesktop.org/basedir-spec/  
http://standards.freedesktop.org/desktop-entry-spec/  
http://standards.freedesktop.org/desktop-entry-spec/  

`jgmenu_run xdg` is a very simple XDG implementation.  
It understands the XML elements <*Menu*>, <*Name*>, <*Directory*>  
and <*Include*><*And*><*Category*>, but ignores everything else.

The .menu file is sought in `${XDG_CONFIG_DIRS:-/etc/xdg}` with  
user configuration override in `${XDG_CONFIG_HOME:-$HOME/.config}`  

`$XDG_MENU_PREFIX` can be used to specity a .menu file. For example  
`$XDG_MENU_PREFIX=lxde-` will load lxde-applications.menu  
This can be useful if there are several .menu files on the system.  

# OPTIONS

\--no-dirs
:   ignore .menu and .directory files

# EXAMPLES

This program is designed to be run from jgmenu by settings  
`csv_cmd = xdg` in the configuration file (jgmenurc).  

It can also be run directly from the command line like this:  

    jgmenu_run xdg | jgmenu --simple

    jgmenu --csv-cmd="jgmenu_run xdg"
