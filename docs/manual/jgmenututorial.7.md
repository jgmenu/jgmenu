% JGMENUTUTORIAL(7)
% Johan Malm
% 16 October, 2019

# NAME

jgmenututorial - A step-by-step tutorial to jgmenu

# INTRODUCTION

This tutorial aims to explain the usage of jgmenu through a set of lessons.

# TABLE OF CONTENTS

- [Lesson 1 - Get started](#lesson1)  
- [Lesson 2 - Architecture](#lesson2)  
- [Lesson 3 - Scripting with jgmenu](#lesson3)  
- [Lesson 4 - Descriptions](#lesson4)  
- [Lesson 5 - Icons](#lesson5)  
- [Lesson 6 - Submenus](#lesson6)  
- [Lesson 7 - XDG Application Menus](#lesson7)  
- [Lesson 8 - Disable directory structure](#lesson8)  
- [Lesson 9 - Apprend/Prepend and Separators](#lesson9)  
- [Lesson 10 - CSV generators](#lesson10)  
- [Lesson 11 - Search](#lesson11)  

# Lesson 1 - Get started {#lesson1}

After installing jgmenu, you can get going quickly by running

    jgmenu_run

You should see a Linux/BSD system menu showing installed applications. We call
this menu "pmenu" (see [lesson 7](#lesson7) for further details).

Create a config file (`~/.config/jgmenu/jgmenurc`) by running

    jgmenu_run init

Full details of config options are covered in [jgmenu(1)](jgmenu.1.html)

You can also try some templates using the interactive mode

    jgmenu_run init -i

There are a small number of configuration options which may need manual
intervention in order for jgmenu to display correctly on your system.

`position_mode`

:   There are several methods for positioning the menu. Try `fixed`, `ipc`,
    `center` and `pointer` to see what works best on your system.
    See jgmenu(1) for full details.

`menu_margin_x` and `menu_margin_y`

:   If your are using `position_mode=fixed`, you may need to set these two
    variables. Depending on what window manager and panel you use, jgmenu may
    be able to automatically find a suitable vertical and horizontal position,
    so try without setting these variables first.

`menu_halign` and `menu_valign`

:   Again, depending on your system, you may need to manually specify
    horizontal and vertical alignment of the menu, but try without first.

# Lesson 2 - Architecture {#lesson2}

As you get started with jgmenu, there are two concepts worth mentioning early,
the modular architecture or jgmenu and the `jgmenu_run` wrapper.

When jgmenu is started, two processes are run to produce the menu.

    ┌────────────────┐
    │ csv-generator  │
    └────────────────┘
            |
            V
    ┌────────────────┐
    │ graphical menu │
    └────────────────┘

The first process (csv-generator) produces the menu content, whereas the
second generates a graphical menu (what you see). This modular approach
provides a lot of flexibility in how jgmenu is used.

`jgmenu_run` is a wrapper script which will either show jgmenu if it is already
running, or start a new instance. This makes it suitable for using with panels
and keyboard shortcuts. See `jgmenu_run(1)` for full details.

# Lesson 3 - Scripting with jgmenu {#lesson3}

From this point onwards, it is assumed that you understand basic shell usage
including re-direction (e.g. \<, >) and piping (e.g. |).

The syntax below (here-document) is used to denote the creation of a text file
from whatever is between the EOFs. You can of course use your favourite text
editor instead.

    cat >file <<EOF
    foo
    bar
    EOF

There are many ways to run jgmenu. In lesson 1, you saw jgmenu as a
long-running application. As we go through the next few lessons we will run
jgmenu as a short-lived applications. This means that it starts from scratch
every time it is called.

So let's get back to basics. Try the following:

    echo >foo.txt <<EOF
    xterm
    firefox
    EOF

If you have not got used to the here-document syntax yet, it just means that
you put the words "xterm" and "firefox" in a text file (which you can of course
do using a text editor). Then either of the following

    cat foo.txt | jgmenu --simple --icon-size=0

    jgmenu --vsimple --csv-file="foo.txt"

The option `--simple` make jgmenu short-lived, disables all syncing with tint2
and reads menu items from `stdin`.

The option `--icon-size=0`, disables icons (i.e. it does not just display them
at zero size, it actually does not load them)

The command line argument `--vsimple` is the same as `--simple`, but also
disables icons and ignores jgmenurc (if it exists).

If you want a menu to be launched by a single script, you could construct it
like this:

    cat <<EOF >menu.sh
    #!/bin/sh
    (
    printf "foo\\n"
    printf "bar\\n"
    ) | jgmenu --vsimple
    EOF
    chmod +x menu.sh
    ./menu.sh

# Lesson 4 - Descriptions {#lesson4}

As you saw in the previous example, each line fed to `stdin` becomes a menu
item. Any line containing two fields separated by a comma is parsed as
`description`,`command`. Consider the following CSV menu data:

    Terminal,xterm
    File Manager,pcmanfm

This lets you give a more meaningful description to each menu item.

# Lesson 5 - Icons {#lesson5}

To display icons, you need to populate the third field. By default, jgmenu will
obtain the icon theme from xsettings (if it is running) or tint2rc (if it
exists). When running with the --simple argument, make sure that `icon_theme`
is set to something sensible in your $HOME/.config/jgmenu/jgmenurc.  Consider
the following CSV menu data:

    Browser,firefox,firefox
    File manager,pcmanfm,system-file-manager
    Terminal,xterm,utilities-terminal
    Lock,i3lock -c 000000,system-lock-screen
    Exit to prompt,openbox --exit,system-log-out
    Reboot,systemctl -i reboot,system-reboot
    Poweroff,systemctl -i poweroff,system-shutdown

In the third field you can also specify the full path if you wish.

# Lesson 6 - Submenus {#lesson6}

So far we have looked at producing a single "root" menu only.  jgmenu
understands a small amount of markup and enables submenus by ^tag() and
^checkout(). Try this:

    Terminal,xterm
    File Manager,pcmanfm
    Settings,^checkout(settings)

    ^tag(settings)
    Set Background Image,nitrogen

In pseudo-code, build your CSV file as follows:

    # the root-menu
    item0.0
    item0.1
    sub1,^checkout(1)
    sub2,^checkout(2)

    # the first sub-menu
    ^tag(1)
    item1.0
    item1.1

    # the second sub-menu
    ^tag(2)
    item2.0
    item2.1

`^root()` can be used instead of `^checkout()` in order to open the submenu in
the parent window.

# Lesson 7 - XDG Application Menus {#lesson7}

freedesktop.org have developed a menu standard which is adhered to by the big
Desktop Environments. We will refer to this type of menu as XDG. jgmenu can run
two types of XDG(ish) menus: pmenu and lx.

To understand the subtleties between them, you need a basic appreciataion of
the XDG menu-spec and desktop-entry-spec. See:
http://standards.freedesktop.org/ for further information.

To keep things simple, when discussing XDG paths, only one location will be
referred to rather than XDG variables and every possible location. So for
example, if "/usr/share" is quoted, it may refer to "/usr/local/share",
"$HOME/.local/share", etc on your system.

In brief, there are three types of files which define the Linux/BSD system
menu:

`.menu`

:   These files are generally located in `/etc/xdg/menus`. They are XML files
    describing such things as the menu categories and directory structure.

`.directory`

:   These are typically located in `/usr/share/desktop-directories` and
    describe the menu directories

`.desktop`

:   On many systems, these will be found at`/usr/share/applications`. Each
    application has a .desktop file associated with it. These files contain
    most of the information needed to build a menu (e.g. `Name`,
    `Exec command`, `Icon` and `Category`)

`pmenu` is written in python by @o9000. It uses .directory and .desktop files
to build a menu, but ignores any .menu files.  Instead of the structure
specified in the .menu file, it simply maps each ".desktop" application onto
one of the ".directory" categories.  If a matching ".directory" category does
not exist, it tries to cross-reference "additional categories" to "related
categories" in accordance with the XDG menu-spec.  This is a generic approach
which avoids Desktop Environment specific rules defined in the .menu file. It
ensures that all .desktop files are included in the menu.

`lx` uses LXDE's libmenu-cache to generate an XDG compliant menu including
separators and internationalization. It requires a recent version of
libmenu-cache, so may not be included in your build.

Set `csv_cmd` in jgmenurc to specify which of these csv-commands you wish to
run.

## Comparison of application menu modules

This table summarise the key features of each module:

    ╔═══════════════════════╤═════════════════╤═════════════════════╗
    ║                       │ pmenu           │ lx                  ║
    ║ ──────────────────────│─────────────────│─────────────────────║
    ║ speed (my machine)    │ 400 ms          │ 99 ms               ║
    ║ language              │ python          │ C                   ║
    ║ dependencies          │ python3         │ glib, libmenu-cache ║
    ║ XDG compliance        │ not intended    │ yes                 ║
    ║ localisation support  │ yes             │ yes                 ║
    ║ ──────────────────────│─────────────────│─────────────────────║
    ║ {ap,pre}pend support  │ yes             │ yes                 ║
    ║ 'no-dirs' support     │ yes             │ yes                 ║
    ║ single window support │ yes             │ no                  ║
    ║ formatting            │ no              │ yes                 ║
    ║ generic name support  │ no              │ yes                 ║
    ╚═══════════════════════╧═════════════════╧═════════════════════╝

# Lesson 8 - Disable directory structure {#lesson8}

Many modern menus and launchers, ignore the XDG directory strcture.

With jgmenu, an XDG menu without any directories can be created in a
number of ways:

The config options `csv_no_dirs = 1`

The CSV generators pmenu and lx understand the environment variable
`JGMENU_NO_DIRS`. Set this variable (e.g. `JGMENU_NO_DIRS=1` to open
a menu without a directory structure.

# Lesson 9 - Apprend/Prepend and Separators {#lesson9}

When running pmenu or lx, you can add menu items to the top and
bottom of the root menu by editing append.csv and/or prepend.csv in
~/.config/jgmenu. For example, try the following:

prepend.csv

    Browser,firefox,firefox
    File manager,pcmanfm,system-file-manager
    Terminal,xterm,utilities-terminal
    ^sep()

append.csv

    ^sep()
    Suspend,systemctl -i suspend,system-log-out
    Reboot,systemctl -i reboot,system-reboot
    Poweroff,systemctl -i poweroff,system-shutdown

In these example we have used the markup ^sep(), which inserts a
horizontal separator line. Similarly, ^sep(foo) inserts a text
separator displaying "foo"

# Lesson 10 - CSV generators {#lesson10}

In lesson 7, we introduced pmenu and lx. These commands are
referred to as "CSV generators" and are invoked as follows:

    jgmenu_run <command>

This is the full list of built-in "CSV generators":

  - pmenu
  - lx
  - ob

They are documented by a man page or a simple --help message.

    man jgmenu-<command>
    jgmenu_run <command> --help

Here follow some examples of how they can be used.

Specify CSV generator in the config file by setting `csv_cmd` in
`~/.config/jgmenu/jgmenurc`

    csv_cmd = jgmenu_run pmenu

Specify CSV generator on the command line

    jgmenu --csv-cmd="jgmenu_run pmenu"

Pipe the CSV output to jgmenu (using `--simple` to read from `stdin`)

    jgmenu_run pmenu | jgmenu --simple

Create a pipemenu using ^pipe() markup. Consider this example

    Terminal,xterm
    File Manager,pcmanfm
    ^pipe(jgmenu_run pmenu)

# Lesson 11 - Search {#lesson11}

jgmenu has search support, which can be invoked by just typing when the menu
is open.

A search box can be inserted using widgets.

