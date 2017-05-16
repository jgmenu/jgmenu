% JGMENUTUTORIAL(7)  
% Johan Malm  
% 1 May, 2017  

# NAME

The jgmenu tutorial

# INTRODUCTION

This tutorial aims to explain the usage of jgmenu through a set of  
lessons.

# LESSONS

Lesson 1
--------

After installing jgmenu, you can get going quickly by running:  

    jgmenu_run

That's it!  

There are three points worth noting about what you have just done:  

  - You should see a "Linux/BSD system" menu with categories such as  
    "Graphics" and "Office". We call this menu "pmenu" (explained  
    below).

  - If you use tint2, jgmenu should have copied its appearance,  
    position and alignment.  

  - You have just started a long-running application. If you click  
    outside the menu, press escape or select a menu item (using  
    mouse or keyboard), the menu will no longer be visible but is  
    still running. It can be awoken (made visible) by executing  
    `jgmenu_run` again.

If you do not use tint2 or if you wish to override some of its  
settings, you can create a config file by running:  

    jgmenu_run init

Edit this config file (~/.config/jgmenu/jgmenurc) to suit your  
system. Read JGMENU-CONFIG(1) for further information.  

If you have a config file at ~/.config/jgmenu/jgmenurc and want to  
ignore it for the purposes of running one of the lessons, just use  
the command line argument "--config-file=" without specifying a file.  

If you use tint2 and want a traditional "start menu", you can do one  
of the following using tint2conf or by editing tint2rc directly:

  - Add jgmenu.desktop to your launcher  

  - Create a button setting `button_lclick_command = jgmenu_run`  

Lesson 2
--------

From this point onwards, it is assumed that you understand basic  
shell usage including re-direction (e.g. \<, >) and piping (e.g. |).

The syntax below is used to denote the creation of a text file from  
whatever is between the EOFs. You can of course use your favourite  
text editor instead.

    cat >file <<EOF
    foo
    bar
    EOF

freedesktop.org have developed a menu standard which is adhered to  
by the big Desktop Environments. In this tutorial we will refer to  
this type of menu as XDG.

There are at least two ways to run XDG(ish) menus:

  - `jgmenu_run pmenu`  
  - `jgmenu_run xdg`  

To understand the subtleties between these, you need a basic  
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

`jgmenu_run pmenu` is written in python by @o9000. It uses .directory  
and .desktop files to build a menu, but ignores any .menu files.  
Instead of the structure specified in the .menu file, it simply maps  
each ".desktop" application onto one of the ".directory" categories.  
If a matching ".directory" category does not exist, it tries to  
cross-reference "additional categories" to "related categories" in  
accordance with the XDG menu-spec.  
This is a generic approach which avoids Desktop Environment specific  
rules defined in the .menu file. It ensures that all .desktop files  
are included in the menu.

`jgmenu_run xdg` is written in C by myself. It uses libxml2 to parse  
the .menu file, but is otherwise written from scratch. It adheres  
to the basics of XDG's menu-spec but has a long way to go to achieve  
full compliance.

Lesson 3
---------

There are many ways to run jgmenu. In lesson 1, you saw jgmenu as a  
long-running application. As we go through the next few lessons we  
will run jgmenu as a short-lived applications. This means that it  
starts from scratch every time it is called.

Let us put XDG system menus and `jgmenu_run` to one side and get  
back to basics. Try the following:

    echo >foo.txt <<EOF
    xterm
    firefox
    EOF

If you have not got used to the syntax yet, it just means that you  
put the words "xterm" and "firefox" in a text file using a text  
editor. Then do:

    cat foo.txt | jgmenu --simple --icon-size=0

The option --simple make jgmenu short-lived and disables all syncing  
with tint2

The option --icon-size=0, disables icons (i.e. it does not just  
display them at zero size, it actually avoids loading them)

If you have dmenu installed, you will get a very similar result  
with:

    cat foo.txt | dmenu

Lesson 4
---------

As you saw in the previous example, each line fed to *stdin* becomes  
a menu item. Any line containing two fields separated by a comma  
is parsed as *description*,*command*. Consider the following:  

    cat <<EOF >menu.sh
    #!/bin/sh
    (
    printf "Terminal,xterm\n"
    printf "File Manager,pcmanfm\n"
    ) | jgmenu --vsimple
    EOF
    
    chmod +x menu.sh
    ./menu.sh

This lets you give a more meaningful description to each menu item.

The command line argument --vsimple is the same as --simple, but also  
disables icons and ignores jgmenurc (if it exists).

Lesson 5
--------

To display icons, you need to populate the third field.  

By default, jgmenu will obtain the icon theme from xsettings (if  
it is running) or tint2rc (if it exists). When running with the  
--simple argument, make sure that *icon_theme* is set to something  
sensible in your $HOME/.config/jgmenu/jgmenurc.

    (
    Browser,firefox,firefox
    File manager,pcmanfm,system-file-manager
    Terminal,xterm,utilities-terminal
    Lock,i3lock -c 000000,system-lock-screen
    Exit to prompt,openbox --exit,system-log-out
    Reboot,systemctl -i reboot,system-reboot
    Poweroff,systemctl -i poweroff,system-shutdown
    ) | jgmenu --simple

In the third field you can also specify the full path if you wish  
e.g. "/usr/share/icons/Faenza/places/22/folder.png"

Lesson 6
--------

So far we have looked at producing a single "root" menu only.  
jgmenu understands a small amount of markup and enables submenus  
by ^tag() and ^checkout(). Try this:  

    cat <<EOF >menu.txt
    Terminal,xterm
    File Manager,pcmanfm
    Settings,^checkout(settings)
    
    Settings,^tag(settings)
    Set Background Image,nitrogen
    EOF
    
    jgmenu --vsimple <menu.txt
    
    # OR
    cat menu.txt | jgmenu --vsimple

A couple of points on submenus:

  - You can press *backspace* to go back to the parent menu.  

  - You can define the root menu with a ^tag(). If you do not, it  
    can still be checked out with ^back().

Lesson 7
--------

You can create a very simple XDG menu without any directories or  
categories in the following way:  

    jgmenu_run parse-xdg --no-dirs | jgmenu --vsimple

"parse-xdg --no-dirs" outputs all apps with a .desktop file  
(normally in /usr/share/applications) without and categories  
or directories.

jgmenu has a *search* capability. When a menu is open, just start  
typing to invoke a filter.

Carrying on the comparison with dmenu, the equivalent can be achieved  
by:

    jgmenu_run parse-xdg --no-dirs | awk -F, '{ print $2}' | dmenu

Lesson 8
--------

This one is just for a bit of fun:

    IFS=:
    (
    for d in $PATH
    do
            cd $d
            find . -maxdepth 1 -type f -executable | sed "s|^\./||"
    done
    ) | jgmenu --vsimple

If you have dmenu installed, the following should be the same:

```
dmenu_path | jgmenu
```

Lesson 9
--------

Let's go back to pmenu.

If you create the files append.csv and/or prepend.csv in  
$HOME/.config/jgmenu, these will be added to your root menu.  

For example, you could do:  

    cat >$HOME/.config/jgmenu/prepend.csv <<EOF
    Browser,firefox,firefox
    File manager,pcmanfm,system-file-manager
    Terminal,xterm,utilities-terminal
    ^sep()
    EOF
    
    cat >$HOME/.config/jgmenu/append.csv <<EOF
    ^sep()
    Exit to prompt,openbox --exit,system-log-out
    Suspend,systemctl -i suspend,system-log-out
    Reboot,systemctl -i reboot,system-reboot
    Poweroff,systemctl -i poweroff,system-shutdown
    EOF
    
    jgmenu_run pmenu

^sep() inserts a horizontal separator line

Lesson 10
---------

If you run on a slow computer, you could speed up the start-up time  
by "caching" the menu data. For example:

    jgmenu_run parse-pmenu >foo.csv
    jgmenu <foo.csv

Or if you do

    jgmenu_run parse-pmenu >$HOME/.config/jgmenu/default.csv

you could simply invoke this menu by:

    jgmenu_run csv

