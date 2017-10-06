jgmenu
======

Introduction
------------

jgmenu is a simple X11 menu intended to be used with tint2 and openbox.

  - To install, see [INSTALL.md](INSTALL.mc)  

  - To run, see [JGMENUTUTORIAL(7)](docs/manual/jgmenututorial.7.md)  

  - See [WIKI](https://github.com/johanmalm/jgmenu/wiki) for screenshots,  
    notes on integration with window managers and panels, timeline, etc.

  - [TODO](TODO)  

  - [Example config file](docs/jgmenurc)  

### Key Features

  - It is a stand-alone, simple and contemporary-looking menu.  
    Although it was originally written to be used with openbox and tint2,  
    it is not in any way dependent on these and runs well with other panels  
    and window managers.  

  - It is hackable with a clean, small code base.  

  - It has a config file to set alignment, margins, padding, transparency, etc.  

  - It can synchronise with tint2's settings.  

  - It does not depend on any toolkits such as GTK and Qt.  

  - It uses cairo and pango to render the menu directly onto an X11 window.  

  - It reads menu items from stdin in a similar way to dmenu and dzen2, but  
    parses comma seperated fields for the name, command and icon.  

It has been compiled and run on OpenBSD, FreeBSD and various Linux  
distributions including Bunsenlabs, Arch, Ubuntu, Alpine, Void and Mint.

See also
--------

### Man pages

  - [JGMENUTUTORIAL(7)](docs/manual/jgmenututorial.7.md)
  - [JGMENU_RUN(1)](docs/manual/jgmenu_run.1.md)
  - [JGMENU(1)](docs/manual/jgmenu.1.md)
  - [JGMENU-CONFIG(1)](docs/manual/jgmenu-config.1.md)
  - [JGMENU-PMENU(1)](docs/manual/jgmenu-pmenu.1.md)
  - [JGMENU-XDG(1)](docs/manual/jgmenu-xdg.1.md)

