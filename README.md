jgmenu
======

jgmenu is a simple X11 menu intended to be used with tint2 and openbox.

  - [Install](INSTALL.md)  

  - [Tutorial](docs/manual/jgmenututorial.7.md)  

  - [Wiki and Screenshots](https://github.com/johanmalm/jgmenu/wiki)  

Description and Key Features
----------------------------

  * jgmenu is a stand-alone, simple and contemporary-looking menu application
    for Linux and BSD.

  * Although it was originally written to be used with openbox and tint2,
    it is not in any way dependent on these and runs well with other panels
    and window managers.

  * It is hackable with a clean, small code base.

  * It can display the following types of menu (or any combination of):

      - bespoke menu using a jgmenu flavoured CSV format

      - application menu (XDG compatible) with localisation support

      - openbox XML menu including pipe-menus

  * It can display SVG, PNG and XPM icons.

  * It has UTF-8 search support.

  * It is highly customizable (e.g. colours, alignment, margins, padding,
    transparency).

  * It can synchronise with xsettings, GTK and tint2 settings.

  * It does not depend on any toolkits such as GTK and Qt, but uses cairo and
    pango to render the menu directly onto an X11 window.

  * It has been compiled and run on OpenBSD, FreeBSD and various Linux
    distributions including Bunsenlabs, Arch, Ubuntu, Alpine, Void and Mint.

Authors
-------

Authors who have contributed more than a few of lines of code to at
least two files in the top-level directory include:

@johanmalm
  - main author

@o9000
  - wrote the pmenu module
  - produced the xsettings client and xpm loader
  - provided much advice on early design choices
  - provided much technical guidance on icon related code,
    inter-process communication (IPC), the main loop select() and
    self-pipe constructs, and more (see git log for full details)

There many who have contributed in other ways including, but not
limited to:

Vladimir-csp
  - has contributed a great deal with suggestions, bug reports and
    advice on XDG compliance

Various members of the BunsenLabs and ArchLabs communities have also
helped with testing, documentation, ideas and inspiration to keep this
project going.

