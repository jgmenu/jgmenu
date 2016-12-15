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

The original "aims" above hold true today, with the exception of a few  
added dependencies:  

  - pango (for font rendering)  
  - librsvg (for SVG icons)  

Some of the `jgmenu_run` commands have further dependencies such as libxml2.  
The build process currently requires these, but it would be easy to #ifdef  
them out if anyone wants that.

It's also worth noting that it has been compiled and runs fine on various  
Linux distributions as well as OpenBSD. It also seems to run fine on other  
Window Managers and across Desktop Environments.

At the heart of Jgmenu is the jgmenu binary which reads menu-items from *stdin*  
and draws a menu. A number of helpers (shell, python and C) exist to simplify  
the user interface and extend the feature-set.

Screenshots
-----------

[![foo](http://i.imgur.com/4oprqYZt.png)](http://i.imgur.com/4oprqYZ.png)
[![foo](http://i.imgur.com/QvBqI2Lt.png)](http://i.imgur.com/QvBqI2L.png)  

From the left:  

1. jgmenu and tint2 using Numix-Circle icon theme
2. jgmenu and tint2 using Papirus icon theme

Dependencies
------------

Jgmenu has the following build requirements:

  - libx11
  - libxinerama
  - cairo
  - pango
  - librsvg
  - libxml2

`jgmenu_run pmenu` and `jgmenu_run xdg` also require a menu package to work  
correctly. For example:

  - gnome-menus
  - lxmenu-data
  - etc

`jgmenu_run pmenu` requires python3 to run.

To build the man pages, you need to have `pandoc` installed. However,  
as many users do not have this package, the man pages are commited  
in the git repo.

To enable menu transparency, you need to have a Composite Manager such as  
`compton`.

For Arch Linux users, there is an AUR package named "jgmenu" which  
installs the correct dependencies.

On Debian based systems such as Bunsenlabs and Ubuntu, do:

```bash
sudo apt-get install libx11-dev libxinerama-dev libcairo2-dev \
libpango1.0-dev librsvg2-dev libxml2-dev
```

Build and Install
-----------------

```bash
make
make install
```

By default, `make install` will install Jgmenu in your $HOME-  
directory, thus avoiding the need for root-privilegies.  
You will need `$HOME/bin` in your `$PATH` to run from here.

The default build process creates `$HOME/{bin,share/man}`

Use `prefix` to specify a different target location. For example,  
if you wish to keep your $HOME top-level directory clean, do:

```bash
make prefix=$HOME/.local install
```

Or, to do a system-wide installation use '/usr' or '/usr/local'.  
For example: 

```bash
sudo make prefix=/usr install
```

In addition to `prefix`, there are a number of build variables which  
can be defined. These are described in the Makefile. Create a  
config.mk to override build settings without making your tree dirty  
or having to re-type them every time. 

There is no "uninstall" in the Makefile. To tidy up, delete the  
following:

  - $prefix/bin/jgmenu*
  - $prefix/share/man/man1/jgmenu*


Getting started
---------------

Create a configuration file using the following command:

```bash
jgmenu_run init
```

Edit ~/.config/jgmenu/jgmenurc to suit your setup. Typically you  
will need to review the following as a minimum:  

  - icon theme
  - alignment
  - margins

Read `man jgmenu-config` for further information.

Although not necessary, it is recommended to create icon cache to  
improve performance:

```bash
jgmenu_run cache
```

There are many ways to run Jgmenu. To get started, try:

```bash
jgmenu_run pmenu
```

For details, see the man pages:

  - [jgmenututorial](docs/manual/jgmenututorial.7.md)
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

Desktop File
------------

`make install` creates a desktop-file in ~/.local/share/applications.  
This file can be used in panels such as tint2, plank and unity.

The build-script for the desktop file reads confik.mk and respects the  
following options:

  - JGMENU_DESKTOP_EXEC   
  - JGMENU_DESKTOP_ICON  
  - JGMENU_UNITY  

For example:

```bash
JGMENU_DESKTOP_EXEC="jgmenu_run csv \"--add-pmenu\""
JGMENU_DESKTOP_ICON="distributor-logo-archlinux"
```

If you are using Ubuntu's Unity, prepend with `env JGMENU_UNITY=1`.  
For example:

```bash
JGMENU_DESKTOP_EXEC="env JGMENU_UNITY=1 jgmenu_run csv"
```

To add the desktop file to tint2, add the line below to the launcher  
section or just drag-and-drop in tint2conf:

```bash
launcher_item_app = jgmenu.desktop
```

If you are using plank, `JGMENU_DESKTOP_ICON` needs to contain the full  
path to the icon.
