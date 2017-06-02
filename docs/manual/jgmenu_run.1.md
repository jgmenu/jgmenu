% JGMENU_RUN(1)  
% Johan Malm  
% 1 Jun, 2017

# NAME

`jgmenu_run` - A simple X11 menu

# SYNOPSIS

`jgmenu_run`  
`jgmenu_run` <*command*> \[<*args*>]

# DESCRIPTION

jgmenu is a small menu application designed for Linux/BSD. It  
consists of the jgmenu binary and a number of helper modules  
written in C, python and shell. The `jgmenu_run` command is an  
abstraction layer which hides the plumbing of components and  
creates a simpler user interface.

If no 'command' or argument is specified, `jgmenu_run` either awakes  
jgmenu or (if not already running) starts jgmenu using the command  
specified by `csv_cmd` in jgmenurc (pmenu by default).

# COMMANDS

Some commands have their own man-pages. These can be opened by  
`man jgmenu-<command>`. If such a man-page does not exist, use  
the `--help` option for usage details.

Valid `commands` include:

### High-level commands

start  
        Start the menu is hidden mode

restart  
        Re-start the menu in order to read the config file or load  
        newly installed apps.

init  
        Create or amend configuration file  

pmenu  
        Menu based on .directory and .desktop files  

xdg  
        Menu based on .menu, .directory and .desktop files  
        See JGMENUTUTORIAL(7) (lesson 2) for further details on the  
        differences between "pmenu" and "xdg"

lx  
        XDG compliant menu based using libmenu-cache to parse  
        .menu, .directory and .desktop files

csv  
        Menu based on .csv file

ob  
        Menu based on openbox menu-file

### Low-level commands

These are designed to be used by the high-level commands, although  
the user can invoke them directly if they wish.

config  
        Get or set variables in the configuration file  

icon-find  
        Find an icon based on name, size and theme

parse-xdg  
        As `xdg`, but outputs jgmenu-csv to stdout

parse-pmenu  
        As `pmenu`, but outputs jgmenu-csv to stdout

xsettings  
        Simple xsettings client

# EXAMPLES

To run the menu:

```
jgmenu_run
```

# SEE ALSO

`JGMENU(1)`  
`JGMENUTUTORIAL(7)`  
`JGMENU-CONFIG(1)`  
