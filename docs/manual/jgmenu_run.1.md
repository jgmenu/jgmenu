% `JGMENU_RUN(1)`
% Johan Malm
% 29 September, 2019

# NAME

`jgmenu_run` - A wrapper for jgmenu

# SYNOPSIS

`jgmenu_run`  
`jgmenu_run` <command> \[<*args*>]  
`jgmenu_run` <options>  

# DESCRIPTION

Use without `command` or `argument` to launch menu. If an instance of jgmenu
is already running, this will be activated, otherwise a new instances will be
started.

Use with `command` to call any programe in $libexecdir/jgmenu which containis
executable programs designed to be run by jgmenu rather than directly by
users.

# OPTIONS

`--help`

:   Print help message

`--exec-path`

:   Print path to $libexecdir/jgmenu which is where jgmenu commands are
    installed.


# EXAMPLES {#examples}

Launch menu

    jgmenu_run

Run the following to see all `jgmenu_run` commands:

    ls $(jgmenu_run --exec-path)

# SEE ALSO

- `jgmenu(1)`
- `jgmenututorial(7)`
