<h3 align="center"><img src="https://i.imgur.com/l8uaBVi.png" alt="jgmenu" height="64px"><br />jgmenu </h3>
<p align="center">A simple X11 menu</p>

<p align="center"> <img
src="https://img.shields.io/github/license/johanmalm/jgmenu.svg" /> <a
href="https://repology.org/metapackage/jgmenu/versions"><img src="https://repology.org/badge/tiny-repos/jgmenu.svg" /></a> <a
href="https://repology.org/metapackage/jgmenu/versions"><img src="https://repology.org/badge/latest-versions/jgmenu.svg"></a> </p>

<h3 align="center">[<a
href="INSTALL.md">Install</a>] [<a
href="docs/manual/jgmenututorial.7.md">Tutorial</a>] [<a
href="https://github.com/johanmalm/jgmenu/wiki">Wiki</a>] [<a
href="https://github.com/johanmalm/jgmenu/wiki/Screenshots">Screenshots</a>]</h3>

<img src="https://i.imgur.com/O3E84L3.png" alt="jgmenu" align="right" height="610px" width="512px">

jgmenu is a stand-alone, simple and contemporary-looking menu application for Linux and BSD.

Although it was originally written to be used with openbox and tint2, it is not in any way dependent on these and runs well with other panels and window managers.

It is hackable with a clean, small code base.

It is designed for customisation, scripting, ricing and tweaking

It can optionally use some appearance settings from XSettings, tint2 and GTK.

It does not depend on any toolkits such as GTK and Qt, but uses cairo and pango to render the menu directly onto an X11 window.

It can display the following types of menu (or any combination of):

- bespoke menu using a jgmenu flavoured CSV format

- application menu (XDG compatible) with localisation support

- pipe menus

- openbox XML menu including openbox pipe-menus

It can display SVG, PNG and XPM icons.

It has UTF-8 search support.

It has been compiled and run on OpenBSD, FreeBSD and various Linux distributions including Bunsenlabs, Arch, Ubuntu, Alpine, Void and Mint.

Authors
-------

Authors who have contributed more than a few of lines of code to at least two files in the top-level directory include:

@johanmalm
  - main author

@o9000
  - wrote the pmenu module
  - produced the xsettings client and xpm loader
  - provided much advice on early design choices
  - provided much technical guidance on icon related code, inter-process communication (IPC), the main loop select() and self-pipe constructs, and more (see git log for full details)

There many who have contributed in other ways including, but not limited to:

Vladimir-csp

  - has contributed a great deal with suggestions, bug reports and advice on XDG compliance

Various members of the BunsenLabs and ArchLabs communities have also helped with testing, documentation, ideas and inspiration to keep this project going.

Versioning
----------

We use [semver 2.0.0](http://www.semver.org), except that at patch-level zero, the ".0" is omitted. For example, 1.0.0 would become 1.0.


