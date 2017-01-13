% JGMENU-PMENU(1)  
% Johan Malm  
% 13 January, 2017

# NAME

jgmenu-pmenu.sh - generates a menu based on .desktop and .directory  
                  files found on the system.  

# SYNOPSIS

jgmenu_run pmenu \[<*options*>]

# DESCRIPTION

Generates a menu based on .desktop and .directory files found on the  
system. For further info, see:  
  - JGMENUTUTORIAL(7) lesson 2  
  - `jgmenu_run parse-pmenu --help`  

The root menu is appended and/or prepended by the contents of the  
following files if they exist:  
  - $HOME/.config/jgmenu/append.csv  
  - $HOME/.config/jgmenu/prepend.csv  

# OPTIONS

All options are passed onto jgmenu

# EXAMPLES

`$ jgmenu_run pmenu`
