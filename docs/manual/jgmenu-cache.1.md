% JGMENU-CACHE(1)  
% Johan Malm  
% 15 September, 2016

# NAME

jgmenu-cache - creates icon cache for jgmenu

# SYNOPSIS

jgmenu_run cache \[\--menu-file=<*menu-file*> \[\--theme=<*theme*>]  
                 \[\--icon-size=<*size*>]

# DESCRIPTION

Some icons themes are slow to load on start-up. In order to avoid the  
expensive icon-lookup process, `jgmenu_run cache` creates a set of  
symlinks in `~/.local/share/icons/jgmenu-cache`. This speeds up the  
icon-load times significantly.

It is anticipated that `jgmenu_run cache` will typically be used  
without any options specified. Without options, it will search for  
the icon-theme in the following places and order:  

  - xsettings daemon

  - ~/.config/jgmenu/jgmenurc

  - ~/.config/gtk-3.0/settings.ini

The icon size will obtained from jgmenurc.

The "input file" will be a concatenation of the xdg menu, pmenu and  
the default csv menu.

An index.theme file is created in ~/.local/share/icons/jgmenu-cache/  
This enables jgmenu to find icons of the correct theme under  
${XDG_DATA_DIRS} in the case that the cache is not updated after  
the installation of new applications.

# OPTIONS

\--menu-file=<*menu-file*>  
:   specify menu-file (in jgmenu csv format)

\--theme=<*theme*>  
:   specify icon-theme

\--icon-size=<*size*>  
:   specify icon-size (in pixels)

# EXAMPLES

The command should work in most instances without any options.  

    $ jgmenu_run cache

The example below demonstrates a more specific situation.

    $ jgmenu_run cache --menu-file=~/.config/jgmenu/default.csv \
                       --theme=Numix-Circle \
                       --icon-size=22

