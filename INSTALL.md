Install
-------

### Install from repos

| OS               | command                   | repo                 |
| :---             | :---                      | :---                 |
| Arch Linux       | pacman -S jgmenu          | [community](https://www.archlinux.org/packages/community/x86_64/jgmenu/) |
| ArchLabs Linux   | pacman -S jgmenu          | [github](https://github.com/ARCHLabs/archlabs_repo/tree/master/x86_64) |
| BunsenLabs Linux | apt-get install jgmenu    | [backports](http://eu.pkg.bunsenlabs.org/debian/pool/main/j/jgmenu/) |
| NixOS            |                           | [package](https://github.com/NixOS/nixpkgs/tree/master/pkgs/applications/misc/jgmenu) |
| Slackware        |                           | [slackbuilds](https://slackbuilds.org/repository/14.2/desktop/jgmenu/) |
| Void Linux       | xbps-install -S jgmenu    | [package](https://github.com/voidlinux/void-packages/blob/master/srcpkgs/jgmenu/template) |

### Build and install from scratch

```bash
git clone https://github.com/johanmalm/jgmenu.git
cd jgmenu
./scripts/install-debian-dependencies.sh  # or equivalent
echo "NO_LX=1" >> config.mk               # unless you have libmenu-cache v1.1+
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


  - A *menu* package  is required for "pmenu", "xdg" and "lx".
    Examples of *menu* packages include: lxmenu-data, gnome-menus
    and libgarcon-common (xfce))

  - python3 is required by "pmenu"

  - A Composite Manager such as `compton` is required to enable transparency.
    Most Desktop Environments already have one installed.

  - To build the man pages, you need to have `pandoc` installed. However, as
    many users do not have this package, the man pages are commited in the git
    repo (i.e. you only need pandoc if you want to contribute to or change the
    man pages.)

