% JGMENU_RUN(1)  
% Johan Malm  
% 19 September, 2016

# NAME

jgmenu_run - top level wrapper for jgmenu

# SYNOPSIS

jgmenu_run <*command*> \[<*args*>]

# DESCRIPTION

jgmenu_run makes jgmenu easier to use.

# COMMANDS

Valid `commands` include:

pmenu

    Menu based on .directory and .desktop files

xdg

    Menu based on .menu file

csv

    Menu based on .csv file

cache

    Create icon cache

simple

    Simple fall-back menu

# EXAMPLES

    jgmenu_run xdg

# SEE ALSO

`jgmenu` (1)
`jgmenu-cache` (1)
