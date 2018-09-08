% JGMENUTUTORIAL(7)  
% Johan Malm  
% 23 Aug, 2018  

# NAME

The jgmenu tutorial

# INTRODUCTION

This tutorial aims to explain the usage of jgmenu through a set of  
lessons.

# TABLE OF CONTENTS

Lesson 1  - Get started  
Lesson 2  - `jgmenu_run`  
Lesson 3  - Scripting with jgmenu  
Lesson 4  - Descriptions  
Lesson 5  - Icons  
Lesson 6  - Submenus  
Lesson 7  - XDG Application Menus  
Lesson 8  - Disable directory structure  
Lesson 9  - Apprend/Prepend and Separators  
Lesson 10 - CSV generators  
Lesson 11 - Search  

# LESSONS

Lesson 1 - Get started
----------------------

After installing jgmenu, you can get going quickly by running:  

    jgmenu

That's it!  

There are three points worth noting about what you have just done:  

  - You should see a "Linux/BSD system" menu showing installed  
    applications. We call this menu "pmenu" (explained in lesson 7).  

  - If you use tint2, jgmenu should have based its appearance and  
    alignment on tint2rc.  

  - You have just started a long-running application. If you click  
    outside the menu, press escape or select a menu item (using  
    mouse or keyboard), the menu will no longer be visible but is  
    still running. It can be awoken (made visible) by executing  
    `jgmenu_run`.

If you wish to override any default settings, you can create a  
config file by running:  

    jgmenu init

Edit this config file (~/.config/jgmenu/jgmenurc) to suit your  
system. Read the man page jgmenu(1) for further information.  

If you have a config file at ~/.config/jgmenu/jgmenurc and want to  
ignore it for the purposes of running one of the lessons, just use  
the command line argument "--config-file=" without specifying a file.  

Lesson 2 - `jgmenu_run`
---------------------

You can also start jgmenu with  

    jgmenu_run

The advantage of this wrapper is that it will either awake jgmenu or  
start a new instance depending on if it is already running or not.  

This makes it suitable for using with panels and keyboard shortcuts.  

See https://github.com/johanmalm/jgmenu/wiki for futher info on  
panel and window manager integration.  

Lesson 3 - Scripting with jgmenu
--------------------------------

From this point onwards, it is assumed that you understand basic  
shell usage including re-direction (e.g. \<, >) and piping (e.g. |).

The syntax below (here-document) is used to denote the creation of a  
text file from whatever is between the EOFs. You can of course use  
your favourite text editor instead.

    cat >file <<EOF
    foo
    bar
    EOF

There are many ways to run jgmenu. In lesson 1, you saw jgmenu as a  
long-running application. As we go through the next few lessons we  
will run jgmenu as a short-lived applications. This means that it  
starts from scratch every time it is called.

So let's get back to basics. Try the following:

    echo >foo.txt <<EOF
    xterm
    firefox
    EOF

If you have not got used to the here-document syntax yet, it just  
means that you put the words "xterm" and "firefox" in a text file  
(which you can of course do using a text editor). Then do:  

    cat foo.txt | jgmenu --simple --icon-size=0

or  

    jgmenu --vsimple --csv-file="foo.txt"

The option `--simple` make jgmenu short-lived, disables all syncing  
with tint2 and reads menu items from _stdin_.  

The option `--icon-size=0`, disables icons (i.e. it does not just  
display them at zero size, it actually avoids loading them)  

The command line argument `--vsimple` is the same as `--simple`, but also  
disables icons and ignores jgmenurc (if it exists).  

If you want a menu to be launched by a single script, you could  
construct it like this:  

    cat <<EOF >menu.sh
    #!/bin/sh
    (
    printf "foo\n"
    printf "bar\n"
    ) | jgmenu --vsimple
    EOF
    chmod +x menu.sh
    ./menu.sh

Lesson 4 - Descriptions
-----------------------

As you saw in the previous example, each line fed to *stdin* becomes  
a menu item. Any line containing two fields separated by a comma  
is parsed as *description*,*command*. Consider the following  
CSV menu data:

    Terminal,xterm
    File Manager,pcmanfm

This lets you give a more meaningful description to each menu item.

Lesson 5 - Icons
----------------

To display icons, you need to populate the third field.  

By default, jgmenu will obtain the icon theme from xsettings (if  
it is running) or tint2rc (if it exists). When running with the  
--simple argument, make sure that *icon_theme* is set to something  
sensible in your $HOME/.config/jgmenu/jgmenurc.  Consider the  
following CSV menu data:  

    Browser,firefox,firefox
    File manager,pcmanfm,system-file-manager
    Terminal,xterm,utilities-terminal
    Lock,i3lock -c 000000,system-lock-screen
    Exit to prompt,openbox --exit,system-log-out
    Reboot,systemctl -i reboot,system-reboot
    Poweroff,systemctl -i poweroff,system-shutdown

In the third field you can also specify the full path if you wish  
e.g. "/usr/share/icons/Faenza/places/22/folder.png"

Lesson 6 - Submenus
-------------------

So far we have looked at producing a single "root" menu only.  
jgmenu understands a small amount of markup and enables submenus  
by ^tag() and ^checkout(). Try this:  

    Terminal,xterm
    File Manager,pcmanfm
    Settings,^checkout(settings)
    
    ^tag(settings)
    Set Background Image,nitrogen

In pseudo-code, build your CSV file as follows:  

    # the root-menu
    sub1,^checkout(sub1)
    sub2,^checkout(sub2)
    
    # the first sub-menu
    ^tag(sub1)
    item1.1
    item1.2
    
    # the second sub-menu
    ^tag(sub2)
    item2.1
    item2.2

^root() can be used instead of ^checkout() in order to open the  
submenu in the parent window.  

Lesson 7 - XDG Application Menus
--------------------------------

freedesktop.org have developed a menu standard which is adhered to  
by the big Desktop Environments. We will refer to this type of menu  
as XDG. jgmenu can run three types of XDG(ish) menus: pmenu, lx and  
xdg.

To understand the subtleties between them, you need a basic  
appreciataion of the XDG menu-spec and desktop-entry-spec. See:  
http://standards.freedesktop.org/ for further information.  

To keep things simple, when discussing XDG paths, only one location  
will be referred to rather than XDG variables and every possible  
location. So for example, if "/usr/share" is quoted, it may refer to  
"/usr/local/share", "$HOME/.local/share", etc on your system.

In brief, there are three types of files which define the Linux/BSD  
system menu:

  - .menu (/etc/xdg/menus)  
    These are XML files describing such things as the menu categories  
    and directory structure.

  - .directory (/usr/share/desktop-directories)  
    These describe the menu directories

  - .desktop (/usr/share/applications)  
    Each application has a .desktop file associated with it. These  
    files contain most of the information needed to build a menu  
    (e.g. "Name", "Exec command", "Icon", "Category")

`pmenu` is written in python by @o9000. It uses .directory and  
.desktop files to build a menu, but ignores any .menu files.  
Instead of the structure specified in the .menu file, it simply maps  
each ".desktop" application onto one of the ".directory" categories.  
If a matching ".directory" category does not exist, it tries to  
cross-reference "additional categories" to "related categories" in  
accordance with the XDG menu-spec.  
This is a generic approach which avoids Desktop Environment specific  
rules defined in the .menu file. It ensures that all .desktop files  
are included in the menu.

`lx` uses LXDE's libmenu-cache to generate an XDG compliant menu  
including separators and internationalization. It requires a recent  
version of libmenu-cache, so may not be included in your build.  

`xdg` uses libxml2 to parse the .menu file, but is otherwise written  
from scratch. It is not as sophisticated as pmenu and lx.  

See the github jgmenu wiki for a comparison of the three.  

Set `csv_cmd` in jgmenurc to specify which of these csv-commands you  
wish to run.  

Lesson 8 - Disable directory structure
--------------------------------------

Many modern menus and launchers, ignore the XDG directory strcture.  

With jgmenu, an XDG menu without any directories can be created in a  
number of ways:  

The config options `csv_no_dirs = 1`  

The CSV generators pmenu and lx understand the environment variable  
`JGMENU_NO_DIRS`. Set this variable (e.g. `JGMENU_NO_DIRS=1` to open  
a menu without a directory structure. 

Lesson 9 - Apprend/Prepend and Separators
-----------------------------------------

When running pmenu, xdg or lx, you can add menu items to the top and  
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

Lesson 10 - CSV generators
--------------------------

In lesson 7, we introduced pmenu, xdg and lx. These commands are  
referred to as "CSV generators" and are invoked as follows:  

    jgmenu_run <command>

This is the full list of built-in "CSV generators":  

  - pmenu  
  - lx  
  - xdg  
  - ob  
  - ff-bookmarks (requires a recent version of firefox) 

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

Lesson 11 - Search
------------------

jgmenu has a *search* capability. When a menu is open, just start  
typing to invoke a filter.  

A search box can be inserted using widgets (see github wiki).  

