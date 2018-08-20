% JGMENU-PMENU(1)  
% Johan Malm  
% 20 Aug, 2018  

# NAME

jgmenu-pmenu.py - generate jgmenu flavoured CSV menu data based on  
                  .desktop and .directory files found on the system  

# SYNOPSIS

`jgmenu_run pmenu` \[-h | --help] \[--append-file <*FILE*>]  
                 \[--prepend-file <*FILE*>] \[--locale <*LOCALE*>]  

# DESCRIPTION

Generates jgmenu flavoured CSV menu data based on .desktop and  
.directory files found on the system, but ignores any .menu files.  
Instead of the structure specified in the .menu file, it simply maps  
each ".desktop" application onto one of the ".directory" categories.  
If a matching ".directory" category does not exist, it tries to  
cross-reference "additional categories" to "related categories" in  
accordance with the XDG menu-spec. This is a generic approach which  
avoids Desktop Environment specific rules defined in the .menu file.  
It ensures that all .desktop files are included in the menu.  

The root menu is appended/prepended by the contents of the following  
files if they exist:

    ~/.config/jgmenu/append.csv  
    ~/.config/jgmenu/prepend.csv  

# OPTIONS

\-h, --help
:   show this help message and exit

\--append-file FILE
:   Path to menu file to append to the root menu  

\--prepend-file FILE
:   Path to menu file to prepend to the root menu  

\--locale LOCALE
:   Use a custom locale (e.g. `en_US.UTF-8`)  
       Available locales can be shown by running 'locale -a'.  

# ENVIRONMENT VARIABLES  

JGMENU_SINGLE_WINDOW
:   If set, ^checkout() items will be replaced by ^root()  

JGMENU_NO_DIRS
:   If set, applications will be listed without a directory  
       structure  


# EXAMPLES

This program is designed to be run from jgmenu by settings  
`csv_cmd = pmenu` in the configuration file (jgmenurc).  

It can also be run directly from the command line like this:  

    jgmenu_run pmenu | jgmenu --simple

    jgmenu --csv-cmd="jgmenu_run pmenu"
