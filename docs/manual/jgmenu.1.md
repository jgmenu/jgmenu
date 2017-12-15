% JGMENU(1)  
% Johan Malm  
% 14 December, 2017

# NAME

jgmenu - A simple X11 menu

# SYNOPSIS

jgmenu \[\--no-spawn] \[\--checkout=<*tag*>] \[\--config-file=<*file*>]  
       \[\--icon-size=<*size*>] \[\--at-pointer] \[\--hide-on-startup]  
       \[\--simple] \[\--vsimple] \[\--csv-file<*file*>]  
       \[\--csv-cmd=<*command*>]  

# DESCRIPTION

jgmenu is a small menu application for Linux/BSD. It is intended to  
be used with openbox and tint2, but is not dependent on these.

jgmenu reads a list of new-line ('\\n') separated items from a file  
and creates a menu. Each line is parsed into *description*, *command*  
and *icon*, using comma as a field separator. Empty lines and lines  
beginning with '#' are ignored. When the user selects an item (by  
left-clicking or pressing enter), the `command` of their selection  
is executed as a new process.  

For example:

    printf "Terminal,xterm\nWeb Browser,firefox" | jgmenu --simple  

If the user wishes to use a comma in a field, triple quotes can be  
used around the whole field in the format aaa,"""bbb"""  

For example:

    foo,"""^pipe(find /usr/share/pixmaps -printf '%f,display %p,%p\n')"""

The following mark-up is supported in the *description* field:

  - ^sep() - define a separator (with or without text)

The following mark-up is supported in the *command* field:

  - ^tag() - define a submenu

  - ^checkout() - check-out a submenu

  - ^sub() - draw a "submenu" arrow

  - ^back() - check-out parent menu

  - ^term() - run program in terminal

  - ^pipe() - execute sub-process and checkout a menu based on its  
  stdout.

Icons will be displayed if the third field is populated; for example:

    Terminal,xterm,utilities-terminal
    Firefox,firefox,firefox

# OPTIONS  

\--no-spawn
:   redirect command to stdout rather than executing it  

\--checkout=<*tag*>
:   checkout submenu <*tag*> on startup  

\--config-file=<*file*>
:   read config file

\--icon-size=<*size*>
:   specify icon size (22 by default)  
       If set to 0, icons will not be loaded.  

\--at-pointer
:   launch menu at mouse pointer  

\--hide-on-startup
:   start menu is hidden state  

\--simple
:    - ignore tint2 settings  
        - run in 'short-lived' mode (i.e. exit after mouse click or  
          enter/escape)  
        - read menu items from _stdin_

\--vsimple
:   same as --simple, but also disables icons and ignores jgmenurc

\--csv-file=<*file*>
:   specify menu file (in jgmenu flavoured CSV format)  
       If file cannot be opened, input if reverted to *stdin*  

\--csv-cmd=<*command*>
:   specify command to produce menu data  
       E.g. `jgmenu_run pmenu`  

# USER INTERFACE
The user interface is generally pretty intuitive. Here follow mouse  
and keyboard events which are not so obvious:  

  - F10 - force quit  
  - Right-click - return to parent menu (in single window mode)  
  - Backspace - return to parent menu (in single window mode)  

# CONFIGURATION FILE

If no file is specified using the --config-file= option, the XDG Base  
Directory Specification is adhered to. I.e:  

  - Global config in `${XDG_CONFIG_DIRS:-/etc/xdg}`  
  - User config override in `${XDG_CONFIG_HOME:-$HOME/.config}`  

For most users ~/.config/jgmenu/jgmenurc is appropriate.  

Global config variables are set in the following order (i.e. bottom  
of list has higher precedence):  

  - built-in defaults (config.c)  
  - tint2rc config file (can be specified by `TINT2_CONFIG`  
    environment variable  
  - jgmenurc config file (can be specified by --config-file=)  
  - command line arguments  

## Syntax

Lines beginning with # are ignored.

All other lines are recognised as setting variables in the format  
*key* = *value*

White spaces are mostly ignored.

### Values

Unless otherwise specified, values as treated as simple strings.

Here follow some specific types:

boolean  
    When a variable takes a boolean value, only 0 and 1 are accepted.  
    0 means false; 1 means true.  

integer  
    When a variable takes an integer value, only numerical values are  
    accepted. The only valid characters are digits (0-9) and  
    minus-sign.  

    All integer variables relating to geometry and position are  
    interpreted as pixel values unless otherwise specified.  

color  
    When a variable takes a color value, only the syntax described  
    below is recognised:  

    #rrggbb aaa  

    where rr, gg and bb represent hexadecimal values (00-ff) for  
    the colours red, green and blue respectively; and aaa stands for  
    the alpha channel value expressed as a percentage (0-100).  
    (i.e. 100 means no transparency and 0 means fully transparent.)  

    For example #ff0000 100 represents red with no transparency,  
    whereas #000088 50 means dark blue with 50% transparency.  

pathname  
    When a variable takes a pathname value, it is evaluated as a  
    string. If the first character is tilde (~), it will be  
    replaced by the the environment variable $HOME just as a shell  
    would expand it.


## Variables

stay_alive = __boolean__ (default 1)  

    If set to 1, the menu will "hide" rather than "exit" when the  
    following events occur:  
      - clicking on menu item  
      - clicking outside the menu  
      - pressing escape  
    When in the hidden mode, a USR1 signal will "un-hide" the menu.  

hide_on_startup = __boolean__ (default 0)  

    If set to 1, jgmenu start in "hidden" mode. This is useful for  
    starting jgmenu during the boot process and then sending a   
    `killall -SIGUSR1 jgmenu` to show the menu.  

csv_cmd = __string__ (default `pmenu`)  

    Defines the command to produce the jgmenu flavoured CSV for  
    `jgmenu`. Accpetable keyword include pmenu, xdg, lx and ob.  
    If a value is given other than these keywords, it will be  
    executed in a shell (so be careful!). If left blank, jgmenu  
    will read from __stdin__. Examples:  

    csv_cmd = xdg
    csv_cmd = jgmenu_run xdg --no-dirs  
    csv_cmd = cat ~/mymenu.csv  

tint2_look = __boolean__ (default 1)  

    Reads tint2rc and parses config options for colours, dimensions  
    and alignment. Also reads tint2 button environment variables.  
    These give more accurate alignment along the length of the panel  
    than what parsing the tint2 config file can achieve.  

at_pointer = __boolean__ (default 0)  

    If enabled, the menu is launched at the pointer position,  
    ignoring `menu_margin_?` and `menu_?align` values.  

multi_window = __boolean__ (default 1)  

    If enabled, submenus open in new windows.  

terminal_exec = __string__ (default x-terminal-emulator)  
terminal_args = __string__ (default -e)

    The values of these two variables are used to build a string to  
    launch programs requiring a terminal to run.  
    With the default values, the string would become:  

    x-terminal-emulator -e 'some_command with arguments'  

    terminal_args must finish with '-e' or equivalent (where '-e'  
    refers to the meaning of '-e' in 'xterm -e'.  

monitor = __integer__ (default 0)  

    Specify a particular monitor as an index starting from 1.  
    If 0, the menu will be launched on the monitor where the mouse  
    is.  

menu_margin_x = __integer__ (default 2)  
menu_margin_y = __integer__ (default 32)  
menu_width = __integer__ (default 200)  
menu_padding_top = __integer__ (default 5)  
menu_padding_right = __integer__ (default 5)  
menu_padding_bottom = __integer__ (default 5)  
menu_padding_left = __integer__ (default 5)  
menu_radius = __integer__ (default 1)  
menu_border = __integer__ (default 0)  

    "margin" refers to space outside an object  
    "padding" refers to space inside an object (between border and  
    content)  
    "radius" refers to the size of rounded corners  
    "border" refers to the border-thickness  

    The `menu_margin_*` variables refer to the distance between the  
    menu (=X11 window) and the edge of the screen.  

menu_halign = (left | right) (default left)  
menu_valign = (top | bottom) (default bottom)  

    Horizontal and vertical alignment respectively.  

sub_spacing = __integer__ (default 1)

    Horizontal space between windows. A negative value results in  
    each submenu window overlapping its parent window.

sub_padding_top = __integer__ (default -1)  
sub_padding_right = __integer__ (default -1)  
sub_padding_bottom = __integer__ (default -1)  
sub_padding_left = __integer__ (default -1)  

    The same as `menu_padding_*` but applies to submenu windows  
    only. If set to a negative value, the `menu_padding_*`  
    equivalent will be used.  

sub_hover_action = __integer__ (default 1)

    Open submenu when hovering over item (only works in multi-window  
    mode).  

item_margin_x = __integer__ (default 3)  
item_margin_y = __integer__ (default 3)  
item_height = __integer__ (default 25)  
item_padding_x = __integer__ (default 4)  
item_radius = __integer__ (default 1)  
item_border = __integer__ (default 0)  

    See equivalent `menu_` variable definitions.  

item_halign = (left | right) (default left)  

    Horizontal alignment of actual menu items. Items are left-aligned  
    by default. If set to right, the option `arrow_string` should be  
    changed too.  

sep_height = __integer__ (default 5)  

    height of separator (defined by ^sep())  

font = __string__ (unset by default)  

    "font" accepts a string such as "Cantarell 10"  
    The font description without a specified size unit is  
    interpreted as "points". If "px" is added, it will be read as  
    pixels. Using "points" enables consistency with other  
    applications.

font_fallback = __string__ (default xtg)  

    The same as 'icon_theme_fallback' (see below)  

icon_size = __integer__ (default 22)  

    If icon_size is set to 0, icons will not be searched for and  
    loaded.

icon_text_spacing = __integer__ (default 10)  

    Distance between icon and text.  

icon_theme = __string__ (unset by default)  

    If an xsettings-daemon is running, the icon theme will be  
    obtained from that daemon. Otherwise, the variable above will be  
    read.

    The behaviour described above can be over-ruled by defining the  
    following two:

icon_theme_fallback = __string__ (default xtg)  

    Specifies the fallback sources of the icon theme in order of  
    precedence, where the left-most letter designates the source  
    with the highest precedence. The following are acceptable  
    characters:  

    x = xsettings  
    t = tint2 config file  
    g = gtk3.0 config file  

    'icon_theme' takes priority if set.  

    In order to increase consistency with tint2, xsettings  
    variables will only be read if the tint2rc variable  
    launcher_icon_theme_override is zero.  

arrow_string = __string__ (default ▸)  

    The "arrow" indicates that a menu item points a submenu.  
    Suggested styles include:  
    → ▶ ➔ ➙ ➛ ➜ ➝ ➞ ➟ ➠ ➡ ➢ ➣ ➤ ➥ ➦ ↦ ⇒ ⇝ ⇢ ⇥ ⇨ ⇾ ➭ ➮ ➯ ➱ ➲ ➺ ➼ ➽ ➾  

arrow_width = __integer__ (default 15)  

    Width of area allocated for arrow. Set to 0 to hide arrow.  

color_menu_bg = __color__ (default #000000 70)  
color_menu_fg = __color__ (default #eeeeee 20)  
color_menu_border = __color__ (default #eeeeee 8)  
color_norm_bg = __color__ (default #000000 00)  
color_norm_fg = __color__ (default #eeeeee 100)  
color_sel_bg = __color__ (default #ffffff 20)  
color_sel_fg = __color__ (default #eeeeee 100)  
color_sel_border = __color__ (default #eeeeee 8)  
color_sep_fg = __color__ (default #ffffff 20)  

# SEE ALSO

`JGMENU_RUN(1)`  
`JGMENUTUTORIAL(7)`  


The jgmenu source code and documentation can be downloaded from  
<https://github.com/johanmalm/jgmenu/>
