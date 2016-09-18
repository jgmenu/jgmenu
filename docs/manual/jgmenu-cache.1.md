% JGMENU-CACHE(1)  
% Johan Malm  
% 15 September, 2016

# NAME

jgmenu-cache - creates icon cache for jgmenu

# SYNOPSIS

jgmenu-cache \--menu-file=<*menu-file*> \[\--theme=<*theme*>]
\[\--icon-size=<*size*>]

# DESCRIPTION

Some icons themes are slow to load on start-up. In order to avoid the
expensive icon-lookup process, jgmenu-cache creates a set of symlinks in
`~/.local/share/icons/jgmenu`. This speeds up the icon-load times
significantly.

  - create icon-cache (see example below)

  - set `icon_theme=jgmenu` in your jgmenurc

# OPTIONS

\--menu-file=<*menu-file*>
:   specify menu-file (in jgmenu csv format)

\--theme=<*theme*>
:   specify icon-theme

\--icon-size=<*size*>
:   specify icon-size (in pixels)

# EXAMPLES

    $ jgmenu-cache --menu-file=~/.config/jgmenu/default.csv \
                   --theme=Numix-Circle \
                   --icon-size=22

