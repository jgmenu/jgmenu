jgmenu
======

Introduction
------------

Jgmenu is written with the following aims:

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

<img src="http://i.imgur.com/4oprqYZ.png" \>

jgmenu v0.3.1 using Numix-Circle icon theme and
[pmenu-jgmenu.py](https://gitlab.com/o9000/pmenu/blob/master/pmenu-jgmenu.py)

Installation
------------

```bash
make
make install
```

By default, `make install` will install files to `~/bin/`. You will need  
`~/bin` in your `$PATH` to run it from here.

Use `prefix` to specify a different target location. For example, to  
install to /usr/bin/, do:

```bash
sudo make prefix=/usr install
```

Dependencies
------------

jgmenu requires libx11, libxinerama, cairo, pango and librsvg.

jgmenu-parse-xdg requires libxml2

If you're running openbox and tint2, you'll probably alread have these
on your system.

Getting started
---------------

To get started, use the `jgmenu_run` wrapper. Try:

```bash
jgmenu_run pmenu
```

Some icons themes are slow to load on start-up. Icon load-times can be  
significantly improved by running:

```bash
jgmenu_run cache
```

To change the menu appearance, create a jgmenurc file.

```bash
mkdir -p ~/.config/jgmenu
cp path_to_jgmenu_dir/jgmenu/docs/jgmenurc ~/.config/jgmenu/
```

How jgmenu works
----------------

The jgmenu binary reads menu-items from *stdin* and draws a menu. 

A number of helpers (shell and C) exist to simplify the user interface and  
extend the feature-set.

For details, see the man pages:

  - [jgmenu_run](docs/manual/jgmenu_run.1.md)
  - [jgmenu](docs/manual/jgmenu.1.md)

### High-level commands

  - `jgmenu_run` [pmenu](docs/manual/jgmenu-pmenu.1.md)
  - `jgmenu_run` [xdg](docs/manual/jgmenu-xdg.1.md)
  - `jgmenu_run` [csv](docs/manual/jgmenu-csv.1.md)
  - `jgmenu_run` [cache](docs/manual/jgmenu-cache.1.md)

### Low-level commands

These are designed to be used by the high-level commands.  
Use the `--help` option for further details on how these work.

  - `jgmenu_run` config
  - `jgmenu_run` icon-find
  - `jgmenu_run` parse-xdg
  - `jgmenu_run` parse-pmenu
  - `jgmenu_run` xsettings
