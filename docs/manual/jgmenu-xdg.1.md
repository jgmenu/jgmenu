% JGMENU-XDG(1)  
% Johan Malm  
% 24 September, 2017

# NAME

jgmenu-xdg.sh - generates a menu based on .menu files

# SYNOPSIS

`jgmenu_run xdg`

# DESCRIPTION

`jgmenu_run xdg` generates a menu based on XML .menu files in  
accordance with the XDG specifications:

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

NIL

# EXAMPLES

`$ jgmenu_run xdg`
