% JGMENUTUTORIAL(7)
% Johan Malm
% 21 February, 2020

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
- [Lesson 8 - Config Options](#lesson8)  
- [Lesson 9 - Apprend/Prepend and Separators](#lesson9)  
- [Lesson 10 - CSV generators](#lesson10)  
- [Lesson 11 - Search](#lesson11)  

# Lesson 1 - Get started {#lesson1}

After installing jgmenu, start the menu by running the following command

    jgmenu_run

You should see a Linux/BSD system menu showing installed applications.
See [lesson 7](#lesson7) for further details.

Create a config file (`~/.config/jgmenu/jgmenurc`) by running

    jgmenu_run init

Full details of config options are covered in [jgmenu(1)](jgmenu.1.html).

By entering the interactive mode and then selecting 't', you can try some
pre-defined templates/themes.

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

The design of jgmenu is very modular, providing a lot of flexibility in how
it is used.

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
second generates the graphical menu.

[jgmenu_run(1)](jgmenu_run.1.html) is a multi-purpose wrapper script which
does the following is pseudo code:

    if (jgmenu is already running)
            show menu
    else
            start a new instance of jgmenu

This makes it suitable for using with panels and keyboard shortcuts.

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
do using a text editor). Then run either of the following

    cat foo.txt | jgmenu --simple --icon-size=0

    jgmenu --vsimple --csv-file="foo.txt"

The option `--simple` make jgmenu short-lived and reads menu items from
`stdin`.

The option `--icon-size=0`, disables icons (i.e. it does not just display them
at zero size, it simply does not load them)

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

    Browser,        firefox,               firefox
    File manager,   pcmanfm,               system-file-manager
    Terminal,       xterm,                 utilities-terminal
    Lock,           i3lock -c 000000,      system-lock-screen
    Exit to prompt, openbox --exit,        system-log-out
    Reboot,         systemctl -i reboot,   system-reboot
    Poweroff,       systemctl -i poweroff, system-shutdown

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

XDG (freedesktop.org) have defined a Linux/BSD Desktop Menu Specification
which is followed by the big Desktop Environments. See
[menu-spec](http://specifications.freedesktop.org/menu-spec/latest/)
for further details. In brief, there are three types of files which define an
XDG menu:

`.menu`

:   XML file describing menu categories and directory structure.
    Located in `/etc/xdg/menus/`, or XDG_CONFIG_{HOME,DIRS} equivalent.

`.directory`

:   Describe menu directories. Located in `/usr/share/desktop-directories/`,
    or XDG_DATA_{HOME,DIRS} equivalent.

`.desktop`

:   Describe applications and contain most of the information needed to build
    a menu (e.g. `Name`, `Exec command`, `Icon` and `Category`)
    Located in `/usr/share/applications/`, or XDG_DATA_{HOME,DIRS}
    equivalent.

Most desktop applications provided their own associated .desktop files,
whereas .menu and .directory files are supplied by menu packages, such as
libmenu-cache (LXDE) and libcargon (XFCE).

The jgmenu core module [jgmenu-apps(1)](jgmenu-apps.1.html) provides a system
menu based on .desktop files and built-in schema data or a specified schema
file, rather than system .menu and .directory files. Whilst this deviates from
XDG menu spec, it is much simpler to understand and tweak. It also avoids
reliance on menu packages.

For strict XDG compliance, the optional module
[jgmenu-lx(1)](jgmenu-lx.1.html) can be used.

See [Lesson 10](#lesson10) for generic instructions on modules.

# Lesson 8 - Config Options {#lesson8}

In lesson 1 we discussed config options `position_mode`, `menu_margin_x`,
`menu_margin_y`, `menu_halign` and `menu_valign`.

Here follow a few more options you may wish to explore. For full details, see
[jgmenu(1)](jgmenu.1.html).

Rofi style:

    csv_no_dirs=1
    csv_single_window=1
    columns=2
    menu_width=600
    menu_valign=center
    menu_halign=center

Synchronize colours, font and icons with tint2 panel

    tint2_look=1

# Lesson 9 - Apprend/Prepend and Separators {#lesson9}

When using `apps`, `pmenu` or `lx`, you can add menu items to the top and
bottom of the root menu by editing append.csv and/or prepend.csv in
~/.config/jgmenu. For example, try the following:

prepend.csv

    Browser,      firefox,               firefox
    File manager, pcmanfm,               system-file-manager
    Terminal,     xterm,                 utilities-terminal
    ^sep()

append.csv

    ^sep()
    Suspend,      systemctl -i suspend,  system-log-out
    Reboot,       systemctl -i reboot,   system-reboot
    Poweroff,     systemctl -i poweroff, system-shutdown

In these example we have used the markup ^sep(), which inserts a horizontal
separator line. Similarly, ^sep(foo) inserts a text separator displaying "foo"

# Lesson 10 - CSV generators {#lesson10}

In previous lessons, we introduced the `apps`, `lx` and `pmenu`. These modules
are referred to as "CSV generators" and are invoked as follows:

    jgmenu_run <command>

Built-in "CSV generators" include: `apps` and `ob`

Optional "CSV generators" include: `lx` and `pmenu`

They are documented by a man page or a simple --help message.

    man jgmenu-<command>
    jgmenu_run <command> --help

Here follow some examples of how they can be used.

Specify CSV generator in the config file by setting `csv_cmd` in
`~/.config/jgmenu/jgmenurc`

    csv_cmd = pmenu

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

A search box can be inserted using widgets. For example, add this to
~/.config/jgmenu/prepend.csv:

    @search,,3,3,150,20,2,left,top,auto,#000000 0,Type to Search

Make sure you adjust menu padding accordingly, for example

    menu_padding_top=24

A search can also be invoked by associating a widget with a ^filter() command.

