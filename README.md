jgmenu
======

Introduction
------------

jgmenu is a simple X11 menu intended to be used with tint2 and openbox.

To run the menu, see lesson 0 and 1 in
[JGMENUTUTORIAL(7)](docs/manual/jgmenututorial.7.md)  

### Key Features

  - It is a stand-alone, simple and contemporary-looking menu  
  - It is hackable with a clean, small code base  
  - Although written to be used with tint2 and openbox, it runs with other  
    panels and window managers  
  - It has a config file to set alignment, margins, padding, transparency, etc  
  - It does not depend on any toolkits such as GTK and Qt   
  - It uses cairo and pango to render the menu directly onto an X11 window  
  - It reads menu items from stdin in a similar way to dmenu and dzen2, but  
    parses comma seperated fields for the name, command and icon  

It has been compiled and runs on OpenBSD and various Linux distributions  
including Bunsenlabs, Arch, Ubuntu, Alpine and Mint.

Screenshots
-----------

[![foo](http://i.imgur.com/4oprqYZt.png)](http://i.imgur.com/4oprqYZ.png)  
jgmenu+tint2+openbox with Numix-Circle icon theme

[![foo](http://i.imgur.com/QvBqI2Lt.png)](http://i.imgur.com/QvBqI2L.png)  
jgmenu+tint2+openbox with Papirus icon theme

[![foo](http://i.imgur.com/4XzVBDOt.png)](http://i.imgur.com/4XzVBDO.png)  
jgmenu on bunsenlabs (`jgmenu_run ob`)

Build and Install
-----------------

### In simple steps

For Arch Linux users, there is an AUR package named "jgmenu".

On Debian based systems such as Bunsenlabs and Ubuntu, do:

```bash
sudo apt-get install libx11-dev libxinerama-dev libcairo2-dev libpango1.0-dev librsvg2-dev libxml2-dev
mkdir ~/src && cd ~/src
git clone https://github.com/johanmalm/jgmenu.git
cd jgmenu
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
  - libxml2


### Run-time dependencies:

  - A *menu* package (for example one of lxmenu-data, libgarcon-common or  
    gnome-menus) is required for "`jgmenu_run pmenu`" and "`jgmenu_run xdg`"  

  - python3 is required by "`jgmenu_run pmenu`"

To enable menu transparency, you need to have a Composite Manager such as  
`compton`. Most Desktop Environments already have one installed.

### Development dependencies

To build the man pages, you need to have `pandoc` installed. However, as many  
users do not have this package, the man pages are commited in the git repo.  
(i.e. you only need pandoc if you want to contribute to or change the man  
pages.)

Desktop File and Panel Integration
----------------------------------

`make install` creates a desktop-file in `~/.local/share/applications` or  
`$prefix/share/applications` if $prefix is specified.

This file can be used in panels such as tint2, plank and unity.

To add this .desktop-file to tint2, add the line below to the launcher  
section in `~/.config/tint2/tint2rc`:

```bash
launcher_item_app = jgmenu.desktop
```

N.B. On some older versions of tint2, the full path to the desktop file  
is required.

If you are using plank, just drag the icon onto the panel. Note that in  
plank, `JGMENU_DESKTOP_ICON` needs to contain the full path to the icon.

The build-script for the .desktop-file reads config.mk and respects the  
following options:

  - `JGMENU_DESKTOP_EXEC`   
  - `JGMENU_DESKTOP_ICON`  
  - `JGMENU_UNITY`  

For example:

```bash
JGMENU_DESKTOP_EXEC="jgmenu_run csv \"--add-pmenu\""
JGMENU_DESKTOP_ICON="distributor-logo-archlinux"
```

If you are using Ubuntu's Unity, prepend the `Exec` command with  
`env JGMENU_UNITY=1`. This prevents the Unity Launcher from flashing  
for 5+ seconds after the menu has been opened. For example:

```bash
JGMENU_DESKTOP_EXEC="env JGMENU_UNITY=1 jgmenu_run"
```

See also
--------

### Man pages

  - [JGMENUTUTORIAL(7)](docs/manual/jgmenututorial.7.md)
  - [JGMENU_RUN(1)](docs/manual/jgmenu_run.1.md)
  - [JGMENU(1)](docs/manual/jgmenu.1.md)
  - [JGMENU-CONFIG(1)](docs/manual/jgmenu-config.1.md)
  - [JGMENU-PMENU(1)](docs/manual/jgmenu-pmenu.1.md)
  - [JGMENU-XDG(1)](docs/manual/jgmenu-xdg.1.md)
  - [JGMENU-CSV(1)](docs/manual/jgmenu-csv.1.md)
  - [JGMENU-CACHE(1)](docs/manual/jgmenu-cache.1.md)

### Other files in repo

  - [Road Map](TODO)  
  - [jgmenurc](docs/jgmenurc)  
