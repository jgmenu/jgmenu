% JGMENUTUTORIAL(7)  
% Johan Malm  
% 1 January, 2016

# NAME

The Jgmenu tutorial

# INTRODUCTION

This tutorial aims to explain the usage of Jgmenu through a set of  
lessons.

To keep things simple, when discussing XDG paths, only one location  
will be referred to rather than XDG variables and every possible  
location. So for example, if "/usr/share" is quoted, it may refer to  
"/usr/local/share", "$HOME/.local/share", etc on your system.

It is assumed that you understand basic shell usage such as  
re-direction (e.g. \<, >) and piping (e.g. |).

The syntax below is used to denote the creation of a text file from  
whatever is between the EOFs. You can use an editor instead.

    cat >file <<EOF
    foo
    bar
    EOF

If you have a config file at ~/.config/jgmenu/jgmenurc and want to  
ignore it for the purposes of running one of the lessons, just use  
"--config-file=" without specifying a file.

# LESSONS

Lesson 1
--------

After installing jgmenu, you can get going quickly by taking the  
following steps:

Create a config file by running:

    jgmenu_run init

Edit the config file (~/.config/jgmenu/jgmenurc) to suit your  
system. You will most likely need to review the icon-theme, alignment  
and margins.

Create icon cache using the command:

    jgmenu_run cache

There are many ways to run the menu, but the following is a simple way  
to get started. It will display a Linux/BSD system menu:

    jgmenu_run pmenu

Lesson 2
--------

freedesktop.org have developed a menu standard which is adhered to  
by the big Desktop Environments. In this tutorial we will refer to  
this type of menu as XDG.

There are at least two ways to run XDG(ish) menus:

  - `jgmenu_run pmenu`  
  - `jgmenu_run xdg`  

To understand the subtleties between these, you need a basic  
appreciataion of the XDG menu-spec and desktop-entry-spec. See:  
http://standards.freedesktop.org/ for further information.  

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

`jgmenu_run pmenu` is written in python by o9000. It uses .directory  
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

Let us put XDG system menus to one side and get back to basics.  
The next few lessons will explain how you can build your own menu  
from scratch. Try the following:

    echo >foo.txt <<EOF
    xterm
    firefox
    EOF

If you have not got used to the syntax yet, it just means that you  
put the words "xterm" and "firefox" in a text file using a text  
editor. Then do:

    cat foo.txt | jgmenu --icon-size=0

The option --icon-size=0, disables icons (i.e. it does not just  
display them at zero size, it actually avoid loading them)

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
    ) | jgmenu
    EOF
    
    chmod +x menu.sh
    ./menu.sh

This lets you give a more meaningful description to each menu item.

Lesson 5
--------

To display icons, you need to populate the third field.  
Also make sure that *icon_size* and *icon_theme* are set to something  
sensible in your $HOME/.config/jgmenu/jgmenurc.

    (
    Browser,firefox,firefox
    File manager,pcmanfm,system-file-manager
    Terminal,xterm,utilities-terminal
    Lock,i3lock -c 000000,system-lock-screen
    Exit to prompt,openbox --exit,system-log-out
    Reboot,systemctl -i reboot,system-reboot
    Poweroff,systemctl -i poweroff,system-shutdown
    ) | jgmenu

In the third field you can also specify the full path if you wish  
e.g. "/usr/share/icons/Faenza/places/22/folder.png"

Lesson 6
--------

So far we have looked at producing a single "root" menu only.  
Jgmenu understands a small amount of markup and enables submenus  
by ^tag() and ^checkout(). Try this:  

    cat <<EOF >menu.txt
    Terminal,xterm
    File Manager,pcmanfm
    Settings,^checkout(settings)
    
    Settings,^tag(settings)
    Set Background Image,nitrogen
    EOF
    
    jgmenu <menu.txt
    
    # OR
    cat menu.txt | jgmenu

A couple of points on submenus:

  - You can press *backspace* to go back to the parent menu.  

  - You can define the root menu with a ^tag(). If you do not, it  
    can still be checked out with ^checkout(root).

Lesson 7
--------

You can create a very simple XDG menu without any directories or  
categories in the following way:  

    jgmenu_run parse-xdg --no-dirs | jgmenu --icon-size=0

"parse-xdg --no-dirs" outputs all apps with a .desktop file  
(normally in /usr/share/applications) without and categories  
or directories.

Jgmenu has a *search* capability. When a menu is open, just start  
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
    ) | jgmenu

If you have dmenu installed, the following should be the same:

```
dmenu_path | jgmenu
```

