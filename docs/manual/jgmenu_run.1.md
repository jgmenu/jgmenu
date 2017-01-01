% JGMENU_RUN(1)  
% Johan Malm  
% 31 September, 2016

# NAME

`jgmenu_run` - A simple X11 menu

# SYNOPSIS

`jgmenu_run` <*command*> \[<*args*>]

# DESCRIPTION

Jgmenu is a small menu application designed for Linux/BSD. It  
consists of the jgmenu binary and a number of helper modules  
written in C, python and shell. The `jgmenu_run` command is an  
abstraction layer which hides the plumbing of components and  
creates a simpler user interface.

# COMMANDS

Some commands have their own man-pages. These can be opened by  
`man jgmenu-<command>`. If a specific man-page does not exist,  
use the `--help` option for usage details.

Valid `commands` include:

### High-level commands

pmenu  
        Menu based on .directory and .desktop files

xdg  
        Menu based on .menu, .directory and .desktop files  
        See JGMENUTUTORIAL (7) (lesson 2) for further details on the  
        differences between "pmenu" and "xdg"

csv  
        Menu based on .csv file

cache  
        Create icon cache

init  
        Create or amend configuration file  

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

Before first use:

```
jgmenu_run init
jgmenu_run cache
```

To run the menu:

```
jgmenu_run pmenu
```

# SEE ALSO

`JGMENU` (1)  
`JGMENUTUTORIAL` (7)  
`JGMENU-CACHE` (1)  
`JGMENU-CONFIG` (1)  



