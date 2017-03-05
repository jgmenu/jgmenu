% JGMENU(1)  
% Johan Malm  
% 21 February, 2017

# NAME

jgmenu - A simple X11 menu

# SYNOPSIS

jgmenu \[\--no-spawn] \[\--checkout=<*tag*>] \[\--config-file=<*file*>]  
       \[\--icon-size=<*size*>] \[\--at-pointer] \[\--stay-alive]  
       \[\--hide-on-startup]

# DESCRIPTION

jgmenu is a small menu application for Linux/BSD, intended to be  
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

The following mark-up is supported in the *description* field:

  - ^sep() - define a separator (with or without text)

The following mark-up is supported in the *command* field:

  - ^tag() - define a submenu

  - ^checkout() - check-out a submenu

  - ^sub() - draw a "submenu" arrow.

  - ^back() - check-out parent menu

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

\--stay-alive
:   make jgmenu a long-running application. In this mode, the  
       normal "exit events" such as selecing an item or clicking  
       outside the menu, will hide the menu. A hidden menu is awoken  
       by running the `jgmenu_run` command.

\--hide-on-startup
:   start menu is hidden state

# USER INTERFACE
The user interface is generally pretty intuitive. Here follow mouse  
and keyboard events which are not so obvious:  

  - Right-click - return to parent menu  
  - Backspace - return to parent menu  
  - F10 - quit even if in `stay_alive` mode  

# CONFIGURATION SETTINGS
The default configuration file location is ~/.config/jgmenu/jgmenurc

# SEE ALSO

`JGMENU_RUN(1)`  
`JGMENUTUTORIAL(7)`  
`JGMENU-CONFIG(1)`  


The jgmenu source code and documentation can be downloaded from  
<http://github.com/johanmalm/jgmenu/>
