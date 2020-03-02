% JGMENU-APPS(1)
% Johan Malm
% 2 March, 2020

# NAME

jgmenu-apps - generate jgmenu flavoured CSV menu data

# SYNOPSIS

`jgmenu_run apps` \[\--help] \[\--no-append] \[\--no-prepend]  

# DESCRIPTION

`jgmenu_run apps` generates jgmenu flavoured CSV menu data for system
applications using built-in schema data or a specified schema file to
map categories to directories, rather than system .directory files.

A schema is searched for in the following locations and order:

- `$XDG_CONFIG_HOME/jgmenu/schema`  
- `$HOME/.config/jgmenu/schema`  
- `$XDG_CONFIG_DIRS/jgmenu/schema`  
- `/etc/xdg/jgmenu/schema`  

The root menu is appended and/or prepended by the contents of the files
`$HOME/.config/jgmenu/{append,prepend}.csv` if they exist.

Applications which do not belong in any of the categories in the schema
file, are placed in 'Other' (which is the one with no Categories= field)

Example schema file:

```
Name=Accessories
Name[sv]=Tillbehör
Icon=applications-accessories
Categories=Accessibility;Core;Utility;

Name=Development
Name[sv]=Utveckling
Icon=applications-development
Categories=Development;

Name=Education
Name[sv]=Utbildning
Icon=applications-science
Categories=Education;

Name=Games
Name[sv]=Spel
Icon=applications-games
Categories=Game;

Name=Graphics
Name[sv]=Grafik
Icon=applications-graphics
Categories=Graphics;

Name=Multimedia
Name[sv]=Multimedia
Icon=applications-multimedia
Categories=Audio;Video;AudioVideo;

Name=Internet
Name[sv]=Internet
Icon=applications-internet
Categories=Network;

Name=Office
Name[sv]=Kontorsprogram
Icon=applications-office
Categories=Office;

Name=Other
Name[sv]=Övrigt
Icon=applications-other

Name=Settings
Name[sv]=Inställningar
Icon=preferences-desktop
Categories=Settings;Screensaver;

Name=System
Name[sv]=System
Icon=applications-system
Categories=Emulator;System;
```


# OPTIONS

`--help`
:   Show help message and exit

`--no-prepend`
:   Do not output ~/.config/jgmenu/prepend.csv before root menu

`--no-append`
:   Do not output ~/.config/jgmenu/append.csv after root menu

# ENVIRONMENT VARIABLES

`JGMENU_SINGLE_WINDOW`

:   If set, `^checkout()` items will be replaced by `^root()`

`JGMENU_NO_DIRS`

:   If set, applications will be listed without a directory structure

`JGMENU_NO_PEND`

:   Do not read append.csv or prepend.csv

`JGMENU_NAME_FORMAT`

:   See `csv_name_format` in jgmenu(1)

`JGMENU_NO_DUPLICATES`

:   See `csv_no_duplicates` in jgmenu(1)

`JGMENU_I18N`

:   See `csv_i18n` in jgmenu(1) and jgmenu-i18n(1)

