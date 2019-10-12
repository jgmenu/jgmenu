<h3 align="center"><img src="https://i.imgur.com/l8uaBVi.png" alt="jgmenu" height="64px"><br />jgmenu </h3>
<p align="center">A simple X11 menu</p>

<p align="center"> <img
src="https://img.shields.io/github/license/johanmalm/jgmenu.svg" /> <a
href="https://www.codacy.com/app/johanmalm/jgmenu?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=johanmalm/jgmenu&amp;utm_campaign=Badge_Grade"><img src="https://api.codacy.com/project/badge/Grade/a154619f17924fcd8ec2be8f338da063" /></a> <br /> <a
href="https://repology.org/metapackage/jgmenu/versions"><img src="https://repology.org/badge/tiny-repos/jgmenu.svg" /></a> <a
href="https://repology.org/metapackage/jgmenu/versions"><img src="https://repology.org/badge/latest-versions/jgmenu.svg"></a> </p>

<h3 align="center">[<a
href="INSTALL.md">Install</a>] [<a
href="NEWS.md">Release&nbsp;Notes</a>] [<a
href="https://jgmenu.github.io/">Website</a>] [<a
href="https://jgmenu.github.io/screenshots.html">Screenshots</a>]</h3>

<img src="https://i.imgur.com/O3E84L3.png" alt="jgmenu" align="right" height="610px" width="512px">

A simple, independent and contemporary-looking X11 menu, designed for scripting, ricing and tweaking.

It is hackable and has a simple code base. It does not depend on any toolkits such as GTK and Qt, but uses cairo and pango to render the menu.

It can optionally use some appearance settings from XSettings, tint2 and GTK.

It can display the following types of menu (or any combination of):

-   bespoke menu using a jgmenu flavoured CSV format
-   application menu (XDG compatible) with localisation support
-   pipe menus
-   openbox XML menu including openbox pipe-menus

It has UTF-8 search support.

Authors
-------

Authors who have contributed more than a few of lines of code to at least two files in the top-level directory include:

[@johanmalm](https://github.com/johanmalm)
-   main author

[@o9000](https://github.com/o9000)
-   wrote the pmenu module
-   produced the xsettings client and xpm loader
-   provided much advice on early design choices
-   provided a lot of technical guidance on icon related code, inter-process communication (IPC), the main loop select() and self-pipe constructs, and more (see git log for full details)

There many who have contributed in other ways including, but not limited to:

[@Vladimir-csp](https://github.com/Vladimir-csp)
-   has contributed a great deal with bug reports and general helpful suggestions (including the `lx` module)
-   advice on XDG compliance

[@johnraff](https://github.com/johnraff)
-   lots of support with the openbox (ob) module
-   packaging

[@Misko-2083](https://github.com/Misko-2083)
-   various contrib/ packages

Members of the BunsenLabs and ArchLabs communities have also helped with testing, documentation, ideas and inspiration to keep this project going.

Versioning
----------

We use [semver 2.0.0](http://www.semver.org), except that at patch-level zero, the ".0" is omitted. For example, 1.0.0 becomes 1.0.

