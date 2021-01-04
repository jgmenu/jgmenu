Install
=======

- [Install from repos](#install-from-repos)
- [Build and install in $HOME directory](#build-and-install-in-home-directory)
- [System wide installation](#system-wide-installation)
- [Build a Debian package](#build-a-debian-package)
- [Dependencies](#dependencies)
- [Build Options](#build-options)

Install from repos
------------------

[Repology](https://repology.org/metapackage/jgmenu/versions)

[![Packaging status](https://repology.org/badge/vertical-allrepos/jgmenu.svg)](https://repology.org/project/jgmenu/versions)

Build and install in $HOME directory
------------------------------------

This is for users who:

-   wish to use/test a feature/fix which is not yet in the latest version in their repo.
-   wish to experiement with the code or contribute code
-   use a distro without jgmenu in their repo and run a single-user system
-   do not like to or are not permitted to use sudo

```bash
git clone https://github.com/johanmalm/jgmenu.git
cd jgmenu
./configure --prefix=$HOME --with-lx --with-pmenu
make
make install
```

Make sure you have `$HOME/bin` in your `$PATH`.

If you have another version of jgmenu installed on your system, please run `hash -r` and then `type jgmenu` to make sure the correct one will be launched

The Makefile contains an uninstall target. As I am not brave enough to write 'sudo rm -rf...', it only works when prefix=$HOME. It is advisable to use a package manager for installations outside of the $HOME directory.

To uninstall jgmenu, run

```bash
make uninstall
```

For subsequent updates, do:

```bash
make uninstall
make clean
git pull
make
make install
```

System wide installation
------------------------

By default, `make install` installs jgmenu to /usr/local/{bin,lib/jgmenu,share/man}

Use `./configure --prefix` to specify a different target location. For example:

```bash
./configure --prefix=/usr
sudo make install
```

Build a Debian package
----------------------

A debian package can be built as follows:

```
dpkg-buildpackage -tc -b -us -uc
```

Then install the .deb package with

```
sudo dpkg -i ../jgmenu_<whatever>.deb
```

Dependencies
------------

## src/ programs

- jgmenu

  * libx11, libxrandr, cairo, pango, librsvg, glib-2.0

- apps

  * Nothing - not even a menu package :)

- ob

  * libxml2

## contrib/ programs

- lx

  * glib-2.0, libmenu-cache (>=1.1.0)
  * A `menu package` such as lxmenu-data or gnome-menus. Xfce's libgarcon-common does not yet work with lx.

- pmenu

  * python3
  * A `menu package` is optional. If none is installed, all applications will be shown in the menu's root directory.

- xfce-panel

  * xfce4-panel

## Other

- A Composite Manager such as `compton` is required to enable transparency. Most Desktop Environments already have one installed.

- To build the man pages, you need to have `pandoc` installed. However, as many users do not have this package, the man pages are commited in the git repo (i.e. you only need pandoc if you want to contribute to or change the man pages.)

Build Options
-------------

In addition to `prefix`, there are a number of build variables which can be defined. Run `./configure --help` for further details.
