% JGMENU(1)  
% Johan Malm  
% 11 September, 2017

# NAME

jgmenu - A simple X11 menu

# SYNOPSIS

jgmenu \[\--no-spawn] \[\--checkout=<*tag*>] \[\--config-file=<*file*>]  
       \[\--icon-size=<*size*>] \[\--at-pointer] \[\--hide-on-startup]  
       \[\--simple] \[\--vsimple] \[\--csv-file<*file*>]  
       \[\--csv-cmd=<*command*>]  

# DESCRIPTION

jgmenu is a small menu application for Linux/BSD. It is intended to  
be used with openbox and tint2, but is not dependent on these. It has  
a number of helper-scripts and wrappers. See *JGMENU_RUN*(1) for  
further details.  

jgmenu reads a list of new-line ('\\n') separated items from standard  
input (stdin) and creates a menu. Each line is parsed into  
*description*, *command* and *icon*, using comma as a field separator.  
Empty lines and lines beginning with '#' are ignored. When the user  
selects an item (by left-clicking or pressing enter), the `command` of  
their selection is executed as a new process.

For example:

    printf "Terminal,xterm\nWeb Browser,firefox" | jgmenu --simple  

If the user wishes to use a comma in a field, triple quotes can be  
used around the whole field in the format aaa,"""bbb"""  

For example:

    foo,"""^pipe(find /usr/share/pixmaps -printf '%f,display %p,%p\n')"""

The following mark-up is supported in the *description* field:

  - ^sep() - define a separator (with or without text)

The following mark-up is supported in the *command* field:

  - ^tag() - define a submenu

  - ^checkout() - check-out a submenu

  - ^sub() - draw a "submenu" arrow

  - ^back() - check-out parent menu

  - ^term() - run program in terminal

  - ^pipe() - execute sub-process and checkout a menu based on its  
  stdout.

Icons will be displayed if the third field is populated; for example:

    Terminal,xterm,utilities-terminal
    Firefox,firefox,firefox

# OPTIONS  

\--no-spawn
:   redirect command to stdout rather than executing it  

\--checkout=<*tag*>
:   checkout submenu <*tag*> on startup  

\--config-file=<*file*>
:   read config file

\--icon-size=<*size*>
:   specify icon size (22 by default)  
       If set to 0, icons will not be loaded.  

\--at-pointer
:   launch menu at mouse pointer  

\--hide-on-startup
:   start menu is hidden state  

\--simple
:   ignore tint2 settings and run in 'short-lived' mode (i.e. exit  
       after mouse click or enter/escape)  

\--vsimple
:   same as --simple, but also disables icons and ignores jgmenurc

\--csv-file=<*file*>
:   specify menu file (in jgmenu flavoured CSV format)  
       If file cannot be opened, input if reverted to *stdin*  

\--csv-cmd=<*command*>
:   specify command to produce menu data  
       E.g. `jgmenu_run parse-pmenu`  

# USER INTERFACE
The user interface is generally pretty intuitive. Here follow mouse  
and keyboard events which are not so obvious:  

  - Right-click - return to parent menu  
  - Backspace - return to parent menu  
  - F10 - quit even if in `stay_alive` mode  

# CONFIGURATION SETTINGS

If no file is specified using the --config-file= option, the XDG Base  
Directory Specification is adhered to. I.e:  

  - Global config in `${XDG_CONFIG_DIRS:-/etc/xdg}`  
  - User config override in `${XDG_CONFIG_HOME:-$HOME/.config}`  

For most users ~/.config/jgmenu/jgmenurc is appropriate.  

# SEE ALSO

`JGMENU_RUN(1)`  
`JGMENUTUTORIAL(7)`  
`JGMENU-CONFIG(1)`  


The jgmenu source code and documentation can be downloaded from  
<https://github.com/johanmalm/jgmenu/>
