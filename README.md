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

Screenshots
-----------

<img src="http://i.imgur.com/ThRLqVS.png" \>

Old screenshot from around v0.1.2

<img src="http://i.imgur.com/4oprqYZ.png" \>

jgmenu v0.3.1 using Numix-Circle icon theme and
[pmenu-jgmenu.py](https://gitlab.com/o9000/pmenu/blob/master/pmenu-jgmenu.py)

Installation
------------

```bash
$ make
$ make install
```

By default, `make install` will install files to ~/bin/. Use `prefix` to 
specify a different target location. For example, to install to /usr/bin/, do:

```bash
$make prefix=/usr install
```

Dependencies
------------

jgmenu requires

  - libx11, libxinerama, cairo and pango (dependencies of gtk2/3)
  - librsvg, libxml2 (dependencies of openbox)

Getting started
---------------

`jgmenu_run` is a wrapper which searches the system for menu files and 
generates a menu. Using `jgmenu_run` is the simplest way to get started.

```bash
$ jgmenu_run
```

To change the menu appearance, create a jgmenurc file.

```bash
$ mkdir -p ~/.config/jgmenu
$ cp path_to_jgmenu_dir/jgmenu/docs/jgmenurc ~/.config/jgmenu/
```

How jgmenu works
----------------
The jgmenu binary reads menu items from stdin.

Menu items are seperated by a new-line character ('\n'). 
Empty lines and lines beginning with '#' are ignored.

The menu item "description" and "command" are seperated by a comma.

When a menu item is selected (by left clicking or pressing enter), the 
"command" is executed as a new process (i.e. not a child process to jgmenu).

The following mark-up is supported (in the command field):

  - ^tag() define a submenu
  - ^checkout() check-out a submenu
  - ^sub() draws a "submenu" arrow. This is useful when submenus are defined 
    in separate files.

### Simple Examples

#### Example 1:

```bash
echo -e "Terminal,xterm\nFile Manager,pcmanfm" | jgmenu
```

#### Example 2:

```bash
cat << EOF >> menu.txt
Terminal,xterm
File Manager,pcmanfm
Settings,^checkout(settings)

Settings,^tag(settings)
Set background image,nitrogen
EOF

jgmenu < menu.txt

# OR
cat menu.txt | jgmenu
```

#### Example 3:

```bash
cat << EOF >> menu.sh
#!/bin/bash
(
echo -e "Terminal,xterm"
echo -e "File Manager,pcmanfm"
) | jgmenu
EOF

chmod +x menu.sh
./menu.sh
```

#### More examples

For more examples, try

```bash
$ make ex
```

### jgmenu command line options

```bash
Usage: jgmenu [OPTIONS]
    --no-spawn            redirect command to stdout rather than executing it
    --checkout=<tag>      checkout submenu <tag> on startup
    --config-file=<file>  read config file
                          (~/.config/jgmenu/jgmenurc) by default
    --icon-size=<size>    specify icon size (0 by default)
```

Icons
-----

Icons can be displayed by taking the following steps:

  - Add the command line argument `--icon-size=<size>` to jgmenu or  
    set `icon_size=<size>` in jgmenurc
  - Set `icon_theme=<theme>` in jgmenurc  
    The theme is set to "Adwaita" by default
  - Add a third field to the stdin data  
    For example:

```bash
Terminal,xterm,utilities-terminal
Firefox,firefox,firefox
```

### Icon cache

Some icons themes are slow to load on start-up.

An order to improve start-up times:

  - create icon-cache (see example below)
  - set `icon_theme=jgmenu` in your jgmenurc

```bash
$ jgmenu-cache --menu-file=<menu-file> --theme=<theme> --icon-size=<size>
```

For example

```bash
$ jgmenu-cache --menu-file=~/.config/jgmenu/default.csv \
               --theme=Numix-Circle \
               --icon-size=22
```

The icon cache is merely a set of symlinks stored locally under
`~/.local/share/icons/jgmenu`, but it speeds up the jgmenu significantly by
avoiding costly icon lookup-time.



