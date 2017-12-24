% JGMENU_RUN(1)  
% Johan Malm  
% 14 December, 2017

# NAME

`jgmenu_run` - A wrapper for jgmenu

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

Valid `commands` include any programe in $libexecdir/jgmenu (which  
is a directory containing executable programs and shell fragments  
designed to be run by jgmenu rather than directly by users).  

# EXAMPLES

To run the menu:

```
jgmenu_run
```

# SEE ALSO

`JGMENU(1)`  
`JGMENUTUTORIAL(7)`  
