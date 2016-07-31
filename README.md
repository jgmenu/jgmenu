jgmenu
======

Introduction
------------
jgmenu is written with the following aims:
  - Be a stand-alone, simple and contemporary-looking menu
  - Be hackable with a clean, small code base
  - Minimise features and bloat
  - Work with linux, openbox and tint2
  - Have co-ordinate and alignment parameters to avoid xdotool hacks, etc.
  - Contain similar settings to tint2 (e.g. padding, transparency)
  - Have few dependencies (perferably just Xlib, cairo and Xinerama)
  - Be free from toolkits such as GTK and Qt.
  - Read the menu items from stdin in a similar way to dmenu and dzen2, but
    with seperate fields for the name and command
    (a bit like dzen2 -m with ^ca parameter).
    E.g. echo -e "terminal,xterm\nfile manager,pcmanfm" | jgmenu

<img src="http://i.imgur.com/ThRLqVS.png" \>

Old screenshot from around v0.1.2

<img src="http://i.imgur.com/4oprqYZ.png" \>

jgmenu v0.3.1 using Numix-Circle icon theme and pmenu-jgmenu.py
(https://gitlab.com/o9000/pmenu/blob/master/pmenu-jgmenu.py)

Installation
------------
```
$ make
$ make install
$ jgmenu_run
```

Basic Usage
-----------
The program reads the menu items from stdin.  Menu items are seperated by a
new-line character ('\n').  Empty lines and lines beginning with '#' are
ignored.

The menu item "description" and "command" are seperated by a comma.

Submenus can be defined and checked out using ^tag() and ^checkout()
markup in the "command" field.

The ^sub() markup draws an arrow to denote "submenu". This is useful for
moving to a submenu in a separate menu file/pipe.

Simple Examples
---------------
Example 1:
```
echo -e "terminal,xterm\nfilemanager,pcmanfm" | jgmenu
```

Example 2:
```
cat << EOF >> menu.txt
terminal,xterm
file manager,pcmanfm
settings,^checkout(settings)
Settings,^tag(settings)
background,nitrogen
EOF

jgmenu < menu.txt
cat menu.txt | jgmenu
```

Example 3:
```
cat << EOF >> menu.sh
#!/bin/bash
(
echo -e "terminal,xterm"
echo -e "file manager,pcmanfm"
) | jgmenu
EOF

chmod +x menu.sh
./menu.sh
```

Icons
-----
Icons can be displayed by taking the following steps
  - add the command line argument --icon-size=X or set icon_size=X in jgmenurc
  - add a third field to the stdin data
    For example
	Terminal,xterm,utilities-terminal
	Firefox,firefox,firefox


