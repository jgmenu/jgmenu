% JGMENU(1)  
% Johan Malm  
% 12 September, 2016

# NAME

jgmenu - menu for X11

# SYNOPSIS

jgmenu \[\--no-spawn] \[\--checkout=<*tag*>] \[\--config-file=<*file*>]
\[\--icon-size=<*size*>]

# DESCRIPTION

jgmenu is a small menu application for X11, intended to be used with openbox
and tint2. It has a number of helper-scripts and wrappers. See *jgmenu_run*(1)
for further details.

jgmenu reads a list of new-line ('\\n') separated items from standard input
(stdin) and creates a menu. Each line is parsed into *description*, *command*
and *icon*, using comma as a field separator. Empty lines and lines beginning
with '#' are ignored. When the user selects an item (by left-clicking or
pressing enter), the `command` of their selection is executed as a new process
(i.e. not a child process to jgmenu).

For example:

    printf "Terminal,xterm\nWeb Browser,firefox" | jgmenu

The following mark-up is supported in the *command* field:

  - ^tag() - define a submenu

  - ^checkout() - check-out a submenu

  - ^sub() - draw a "submenu" arrow.

Icons can be displayed by:

  - Adding the command line argument `--icon-size=<size>` to jgmenu or  
    setting `icon_size=<size>` in jgmenurc

  - Add a third field to the stdin data; for example:

        Terminal,xterm,utilities-terminal
        Firefox,firefox,firefox

Some icons themes are slow to load on start-up. In order to improve start-up
times:

  - create icon-cache (see `jgmenu-cache` (1))

  - set `icon_theme=jgmenu` in your jgmenurc

# OPTIONS

\--no-spawn  
:   redirect command to stdout rather than executing it

\--checkout=<*tag*>  
:   checkout submenu <*tag*> on startup

\--config-file=<*file*>  
:   read config file. If not specified, the default file
    ~/.config/jgmenu/jgmenurc will be read

\--icon-size=<*size*>  
:   specify icon size (0 by default)


# CONFIGURATION SETTINGS
The default configuration file location is ~/.config/jgmenu/jgmenurc



# EXAMPLES

### Example 1:

    cat << EOF >> menu.txt
    Terminal,xterm
    File Manager,pcmanfm
    Settings,^checkout(settings)

    Settings,^tag(settings)
    Set background image,nitrogen
    EOF

    jgmenu < menu.txt

    # OR
    cat menu.txt | jgmenu

### Example 2:

    cat << EOF >> menu.sh
    #!/bin/bash
    (
    echo -e "Terminal,xterm"
    echo -e "File Manager,pcmanfm"
    ) | jgmenu
    EOF

    chmod +x menu.sh
    ./menu.sh

# SEE ALSO

`jgmenu_run` (1)  
`jgmenu-cache` (1)

The jgmenu source code and all documentation can be downloaded from  
<http://github.com/johanmalm/jgmenu/>
