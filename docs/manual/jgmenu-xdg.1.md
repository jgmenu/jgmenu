% JGMENU-XDG(1)  
% Johan Malm  
% 27 September, 2016

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

*WARNING*: `jgmenu_run xdg` is still at an experimental stage.  
It understands the XML elements <*Menu*>, <*Name*>, <*Directory*>  
and <*Include*><*And*><*Category*>, but ignores everything else.

The .menu files are sought in the following order:

  - `~/.config/jgmenu/default.menu`  
  - `${XDG_CONFIG_DIRS}/menus/*-applications.menu`  

The default value of `$XDG_CONFIG_DIRS` is "/etc/xdg".

`$XDG_MENU_PREFIX` can be used to specity a .menu file. For example  
`$XDG_MENU_PREFIX=lxde-` will load lxde-applications.menu  
This can be useful if there are several .menu files on the system.

# OPTIONS

NIL

# EXAMPLES

`$ jgmenu_run xdg`
