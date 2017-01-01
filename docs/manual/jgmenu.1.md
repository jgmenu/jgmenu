% JGMENU(1)  
% Johan Malm  
% 31 December, 2016

# NAME

jgmenu - A simple X11 menu

# SYNOPSIS

jgmenu \[\--no-spawn] \[\--checkout=<*tag*>] \[\--config-file=<*file*>]  
       \[\--icon-size=<*size*>] \[\--at-pointer]

# DESCRIPTION

Jgmenu is a small menu application for Linux/BSD, intended to be  
used with openbox and tint2. It has a number of helper-scripts and  
wrappers. See *JGMENU_RUN*(1) for further details.  

jgmenu reads a list of new-line ('\\n') separated items from standard  
input (stdin) and creates a menu. Each line is parsed into  
*description*, *command* and *icon*, using comma as a field separator.  
Empty lines and lines beginning with '#' are ignored. When the user  
selects an item (by left-clicking or pressing enter), the `command` of  
their selection is executed as a new process.

For example:

    printf "Terminal,xterm\nWeb Browser,firefox" | jgmenu

The following mark-up is supported in the *command* field:

  - ^tag() - define a submenu

  - ^checkout() - check-out a submenu

  - ^sub() - draw a "submenu" arrow.

Icons will be displayed if the third field is populated; for example:

    Terminal,xterm,utilities-terminal
    Firefox,firefox,firefox

Some icons themes are slow to load on start-up. In order to improve  
start-up times:

  - create icon-cache by running `jgmenu_run cache`

# OPTIONS

\--no-spawn  
:   redirect command to stdout rather than executing it

\--checkout=<*tag*>  
:   checkout submenu <*tag*> on startup

\--config-file=<*file*>  
:   read config file. If not specified, the default file  
       ~/.config/jgmenu/jgmenurc will be read.

\--icon-size=<*size*>  
:   specify icon size (22 by default)  
       If set to 0, icons will not be loaded.

\--at-pointer  
:   launch menu at mouse pointer

# CONFIGURATION SETTINGS
The default configuration file location is ~/.config/jgmenu/jgmenurc

# SEE ALSO

`JGMENU_RUN` (1)  
`JGMENUTUTORIAL` (7)
`JGMENU-CONFIG` (1)  


The jgmenu source code and documentation can be downloaded from  
<http://github.com/johanmalm/jgmenu/>
