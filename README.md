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
$ make
$ make install
```

By default, `make install` will install files to ~/bin/. Use `prefix` to 
specify a different target location. For example, to install to /usr/bin/,
do:

```bash
$make prefix=/usr install
```

Dependencies
------------

jgmenu requires libx11, libxinerama, cairo, pango and librsvg.

jgmenu-parse-xdg requires libxml2

Getting started
---------------

To get started, use the `jgmenu_run` wrapper. Try:

```bash
$ jgmenu_run xdg
```

Some icons themes are slow to load on start-up. Icon load-times can be
significantly improved by running:

```bash
$ jgmenu_run cache
```

FIXME: `jgmenu_run cache` is not quite finished yet - will implement
over the next couple of days.

To change the menu appearance, create a jgmenurc file.

```bash
$ mkdir -p ~/.config/jgmenu
$ cp path_to_jgmenu_dir/jgmenu/docs/jgmenurc ~/.config/jgmenu/
```

How jgmenu works
----------------

The jgmenu binary reads menu-items from *stdin* and draws a menu. 

A number of wrappers (shell and C) exist to extend the feature-set.

### `jgmenu`

  - [jgmenu.1](docs/manual/jgmenu.1.md)

### `jgmenu_run` - top-level wrapper

  - [jgmenu_run.1](docs/manual/jgmenu_run.1.md)

### high-level helpers

  - [jgmenu-cache.1](docs/manual/jgmenu-cache.1.md)

