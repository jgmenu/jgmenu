% JGMENU-CSV(1)  
% Johan Malm  
% 28 September, 2016

# NAME

jgmenu-csv.sh - generates a menu based on CSV file

# SYNOPSIS

jgmenu_run csv \[<*options*>] \[<*file*>]

# DESCRIPTION

`jgmenu_run csv` generates a menu based on a CSV file.

By default, `~/.config/jgmenu/default.csv` will be used.

See jgmenu(1) for further details for the CSV file syntax and  
config file settings.

# OPTIONS

\--add-pmenu  
:   enables ^checkout(pmenu)

\--add-xdg
:   enables ^checkout(Applications)

# EXAMPLES

`jgmenu_run csv`

`jgmenu_run csv --add-pmenu ./docs/default.csv`
