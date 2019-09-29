Install
=======

Install from repos
------------------

[Repology](https://repology.org/metapackage/jgmenu/versions)

| Repo                                                                                              | command                 | Version                                                                                      |
| :------------------------------------------------------------------------------------------------ | :---------------------- | :------------------------------------------------------------------------------------------- |
| [Arch](https://www.archlinux.org/packages/community/x86_64/jgmenu/)                               | pacman -S jgmenu        | ![](https://repology.org/badge/version-only-for-repo/arch/jgmenu.svg)                        |
| [BunsenLabs](http://eu.pkg.bunsenlabs.org/debian/pool/main/j/jgmenu/)                             | apt-get install jgmenu  | ![](https://repology.org/badge/version-only-for-repo/bunsenlabs_jessie_backports/jgmenu.svg) |
| [Mageia Cauldron](http://madb.mageia.org/package/show/application/0/release/cauldron/name/jgmenu) | urpmi jgmenu            | ![](https://repology.org/badge/version-only-for-repo/mageia_cauldron/jgmenu.svg)             |
| [Manjaro]()                                                                                       | pacman -S jgmenu        | ![](https://repology.org/badge/version-only-for-repo/manjaro_stable/jgmenu.svg)              |
| [OpenSUSE](https://build.opensuse.org/package/show/openSUSE:Factory/jgmenu)                       |                         | ![](https://repology.org/badge/version-only-for-repo/opensuse_tumbleweed/jgmenu.svg)         |
| [Slackware](https://slackbuilds.org/repository/14.2/desktop/jgmenu/)                              | slackpkg install jgmenu | ![](https://repology.org/badge/version-only-for-repo/slackbuilds/jgmenu.svg)                 |
| [Void](https://github.com/void-linux/void-packages/tree/master/srcpkgs/jgmenu)                    | xbps-install -S jgmenu  | ![](https://repology.org/badge/version-only-for-repo/void_x86_64/jgmenu.svg)                 |

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
./scripts/install-debian-dependencies.sh  # or equivalent
make
make prefix=$HOME install
```

Make sure you have `$HOME/bin` in your `$PATH`.

If you have another version of jgmenu installed on your system, please run `hash -r` and then `type jgmenu` to make sure the correct one will be launched

The Makefile contains an uninstall target. As I am not brave enough to write 'sudo rm -rf...', it only works when prefix=$HOME. It is advisable to use a package manager for installations outside of the $HOME directory.

To uninstall jgmenu, run

```bash
make prefix=$HOME uninstall
```

For subsequent updates, do:

```bash
make prefix=$HOME uninstall
make clean
git pull
make
make prefix=$HOME install
```

System wide installation
------------------------

By default, `make install` installs jgmenu to /usr/local/{bin,lib/jgmenu,share/man}

Use `prefix` to specify a different target location. For example:

```bash
sudo make prefix=/usr install
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

## Core Programs

| program | dependencies                             |
| :---    | :---                                     |
| jgmenu  | libx11, libxrandr, cairo, pango, librsvg |
| ob      | libxml2                                  |
| lx      | glib-2.0, libmenu-cache (>=1.1.0)        |

-   A *menu* package  is required for "lx". Recommended *menu* packages include: lxmenu-data and gnome-menus. Xfce's libgarcon-common does not yet work with lx.
-   "pmenu" uses any installed *menu* packages if they exist, but also works without these by showing applications in the menu's root directory.
-   python3 is required by "pmenu"
-   A Composite Manager such as `compton` is required to enable transparency. Most Desktop Environments already have one installed.
-   To build the man pages, you need to have `pandoc` installed. However, as many users do not have this package, the man pages are commited in the git repo (i.e. you only need pandoc if you want to contribute to or change the man pages.)

## contrib/ packages

| program    | dependencies         |
| :---       | :---                 |
| xfce-panel | libgtk3, xfce4-panel |

The `-dev` packages are required to build the xfce-panel plugin library.

For those who are not going to build, the dependencies are only required if
you are actually going to use it with the xfce panel (in which case you'll have
it installed anyway).

Build Options
-------------

In addition to `prefix`, there are a number of build variables which can be defined. These are described in the Makefile. Create a config.mk to override build settings without making your tree dirty or having to re-type them every time.
