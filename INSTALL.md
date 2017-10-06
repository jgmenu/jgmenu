Install
-------

### Install from repos

Arch Linux: [AUR package](https://aur.archlinux.org/packages/jgmenu/)  

Void Linux: [Official package](https://github.com/voidlinux/void-packages/blob/master/srcpkgs/jgmenu/template)  

### Build and install from scratch

On Debian based systems such as Bunsenlabs and Ubuntu, do:

```bash
mkdir ~/src && cd ~/src
git clone https://github.com/johanmalm/jgmenu.git
cd jgmenu
./scripts/install-debian-dependencies.sh
make
make install
```

For subsequent updates, do:

```bash
git pull
make clean
make
make install
```

A user has written some alternative installation notes
[here](https://forums.bunsenlabs.org/viewtopic.php?id=3100)  

### Furter details on build and installation

By default, `make install` will install jgmenu in your $HOME-directory, thus  
avoiding the need for root-privilegies. You will need `$HOME/bin` in your  
`$PATH` to run from there.

The build process installs files under:  

  - bin/
  - lib/jgmenu/
  - share/man/

Use `prefix` to specify a different target location. For example,  
if you wish to keep your $HOME top-level directory clean, you could do:  

```bash
make prefix=$HOME/.local install
```

Or, to do a system-wide installation use '/usr' or '/usr/local'.  
For example: 

```bash
sudo make prefix=/usr install
```

In addition to `prefix`, there are a number of build variables which can be  
defined. These are described in the Makefile. Create a config.mk to override  
build settings without making your tree dirty or having to re-type them every  
time. 

There is no "uninstall" target in the Makefile. To tidy up, delete the  
following:

  - $prefix/bin/jgmenu*  
  - $prefix/lib/jgmenu/  
  - $prefix/share/man/man1/jgmenu*  
  - $prefix/share/man/man7/jgmenu*  

Although it is recommended to do a `make install`, it is possible to run  
`./jgmenu_run` from the source directory by setting `JGMENU_EXEC_PATH`.

Dependencies
------------

### Build dependencies

  - libx11
  - libxinerama
  - cairo
  - pango
  - librsvg
  - libxml2 (for xdg and ob modules)
  - glib (for lx module)
  - libmenu-cache (for lx module)


### Run-time dependencies:

  - A *menu* package  is required for "`jgmenu_run pmenu`",  
    "`jgmenu_run xdg`" and "`jgmenu_run lx`"  
    Examples of *menu* packages include: lxmenu-data, gnome-menus  
    and libgarcon-common (xfce))  
  - python3 is required by "`jgmenu_run pmenu`"  

To enable menu transparency, you need to have a Composite Manager such as  
`compton`. Most Desktop Environments already have one installed.

### Development dependencies

To build the man pages, you need to have `pandoc` installed. However, as many  
users do not have this package, the man pages are commited in the git repo.  
(i.e. you only need pandoc if you want to contribute to or change the man  
pages.)

