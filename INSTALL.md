Install
-------

### Install from repos

BunsenLabs Linux: [backports](http://eu.pkg.bunsenlabs.org/debian/pool/main/j/jgmenu/)  

Arch Linux: [AUR package](https://aur.archlinux.org/packages/jgmenu/)  

Void Linux: [Official package](https://github.com/voidlinux/void-packages/blob/master/srcpkgs/jgmenu/template)  

### Build and install from scratch

On Debian based systems:

```bash
mkdir ~/src && cd ~/src
git clone https://github.com/johanmalm/jgmenu.git
cd jgmenu
./scripts/install-debian-dependencies.sh
make
sudo make install
```

For subsequent updates, do:

```bash
git pull
make clean
make
sudo make install
```

### Furter details on build and installation

By default, `make install` installs jgmenu to  
/usr/local/{bin,lib/jgmenu,share/man}  

Use `prefix` to specify a different target location. For example: 

```bash
sudo make prefix=/usr install
```

In order to avoid the need for root-privilegies, you can install to your $HOME  
directory by `make prefix=$HOME install`. You will need `$HOME/bin` in your  
`$PATH` to run from there.  

In addition to `prefix`, there are a number of build variables which can be  
defined. These are described in the Makefile. Create a config.mk to override  
build settings without making your tree dirty or having to re-type them every  
time. 

There is no "uninstall" target in the Makefile. To tidy up, delete the
following:

```bash
$prefix/bin/jgmenu*
$prefix/lib/jgmenu/
$prefix/share/man/man1/jgmenu*
$prefix/share/man/man7/jgmenu*
```

Although it is recommended to do a `make install`, it is possible to run
`./jgmenu_run` from the source directory by setting `JGMENU_EXEC_PATH`.

Dependencies
------------

| program | dependencies                                |
| :---    | :---                                        |
| jgmenu  | libx11, libxinerama, cairo, pango, librsvg  |
| ob      | libxml2                                     |
| xdg     | libxml2                                     |
| lx      | glib, libmenu-cache                         |


  - A *menu* package  is required for "`jgmenu_run pmenu`",
    "`jgmenu_run xdg`" and "`jgmenu_run lx`"
    Examples of *menu* packages include: lxmenu-data, gnome-menus
    and libgarcon-common (xfce))

  - python3 is required by "`jgmenu_run pmenu`"

  - A Composite Manager such as `compton` is required to enable transparency.
    Most Desktop Environments already have one installed.

  - To build the man pages, you need to have `pandoc` installed. However, as
    many users do not have this package, the man pages are commited in the git
    repo (i.e. you only need pandoc if you want to contribute to or change the
    man pages.)

