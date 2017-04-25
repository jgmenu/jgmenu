% JGMENU-CONFIG(1)  
% Johan Malm  
% 25 April, 2017

# NAME

jgmenu-config - gets and sets jgmenu options

# SYNOPSIS

jgmenu_run config \--get <*key*>  
jgmenu_run config \--set <*key*> \<*value*>

# DESCRIPTION

Config options can be queried and set using this command.

# OPTIONS

\--get <*key*>  
:   returns the value of specified key

\--set <*key*> <*value*>  
:   sets/adds config option <*key*>

# CONFIGURATION FILE

The jgmenu configuration file contains a number of variables that  
affect its behavior. It is stored at

$HOME/.config/jgmenu/jgmenurc

## Syntax

Lines beginning with # are ignored.

All other lines are recognised as setting variables in the format  
*key* = *value*

White spaces are mostly ignored.

When a variable takes a boolean value, only 0 and 1 are accepted  
for false and true respectively.

Colours are recognised in the format #rrggbb aaa; where  
  - rr=red; gg=green; bb=blue; aa=alpha  
The colors take a hexadecimal value from 00 to ff.  
Alpha takes a decimal value from 0 to 100 where:  
  - 100 means no transparency  
  - 0 means fully transparent  


All space-related variables are in pixels unless otherwise specified

## Variables

stay_alive  

    If set to 1, the menu will "hide" rather than "exit" when the  
    following events occur:  
      - clicking on menu item  
      - clicking outside the menu  
      - pressing escape  
    When in the hidden mode, a USR1 signal will "un-hide" the menu.  

hide_on_startup  

    If set to 1, jgmenu start in "hidden" mode. This is useful for  
    starting jgmenu during the boot process and then sending a   
    `killall -SIGUSR1 jgmenu` to show the menu.  

csv_cmd  

    defines the command to produce the jgmenu flavoured CSV for  
    `jgmenu_run` (when run without argument). Examples include:  

    csv_cmd = jgmenu_run parse-ob  
    csv_cmd = cat ~/mymenu.csv  

tint2_look  

    Reads tint2rc and parse config options for colours, dimensions  
    and alignment.  

tint2_button  

    Reads tint2 button environment position variables. These give  
    more accurate alignment along the length of the panel than what  
    the "tint2_look" option achieves.  

tint2_rules  

    Reads tint2rc variables in preference to jgmenurc.  
    If "tint2_rules = 0", jgmenurc can be used to overrule specific  
    tint2rc settings.  

menu_margin_x  
menu_margin_y  
menu_width  
menu_radius  
menu_border  

    "margin" refers to space outside an object  
    "padding" refers to space inside an object (between border and  
    content)  
    "radius" refers to the size of rounded corners  
    "border" refers to the border-thickness (in pixels)  

    The menu_margin_* variables refer to the distance between the  
    menu (=X11 window) and the edge of the screen.  

menu_halign  
menu_valign  

    Valid options for menu horizontal alignment are "left" and  
    "right". Vertical alignment understands "top" and "bottom".  

at_pointer  

    "at_pointer" will launch the menu at the mouse pointer position.  
    This option overrides menu_margin_? and menu_?align settings

item_margin_x  
item_margin_y  
item_height  
item_padding_x  
item_radius  
item_border  


sep_height  

    height of separator (defined by ^sep())

font  

    "font" accepts a string such as "Cantarell 10"  
    The font description without a specified size unit is  
    interpreted as "points". If "px" is added, it will be read as  
    pixels. Using "points" enables consistency with other  
    applications.

icon_size  
icon_theme  

    If icon_size is set to 0, icons will not be searched for and  
    loaded.

    If an xsettings-daemon is running, the icon theme will be  
    obtained from that daemon. Otherwise, the variable above will be  
    read.

    It is recommended to create icon cache before running jgmenu,  
    using `jgmenu_run cache`. However, please note that:  
      - If no cache exists, jgmenu will search xsettings and jgmenurc  
        in real time.  
      - If cache already exists, changing the xsettings values or the  
        jgmenurc values below will have no effect until  
        `jgmenu_run cache` is run again or the cache is deleted  
        (using `jgmenu_run cache --delete`)

    The behaviour described above can be over-ruled by defining the  
    following two:

ignore_xsettings  
ignore_icon_cache  

show_title  

arrow_string  
arrow_show  

    The "arrow" indicates that a menu item points a submenu.  
    Suggested styles include:  
    → ▶ ➔ ➙ ➛ ➜ ➝ ➞ ➟ ➠ ➡ ➢ ➣ ➤ ➥ ➦ ↦ ⇒ ⇝ ⇢ ⇥ ⇨ ⇾ ➭ ➮ ➯ ➱ ➲ ➺ ➼ ➽ ➾  

search_all_items  

    Define "search_all_items" to include all submenus when searching.  
    If set to 0, only the checked out submenu (or root-menu) will be  
    searched.  

color_menu_bg  
color_menu_fg  
color_menu_border  
color_norm_bg  
color_norm_fg  
color_sel_bg  
color_sel_fg  
color_sel_border  
color_noprog_fg  
color_title_bg  
color_sep_fg  

    Example colour string: #eeeeee 70  
