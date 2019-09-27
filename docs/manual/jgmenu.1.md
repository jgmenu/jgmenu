% JGMENU(1)
% Johan Malm
% 27 September, 2019

# NAME

jgmenu - A simple X11 menu

# SYNOPSIS

jgmenu \[\--no-spawn] \[\--checkout=<*tag*>] \[\--config-file=<*file*>]  
       \[\--icon-size=<*size*>] \[\--at-pointer] \[\--hide-on-startup]  
       \[\--simple] \[\--vsimple] \[\--csv-file=<*file*>]  
       \[\--csv-cmd=<*command*>] \[\--die-when-loaded]  
       \[\--center]

jgmenu init \[\--help | <*options*>]

Use these commands to get started

- `jgmenu_run` to launch menu  
- `jgmenu init` to create config file ~/.config/jgmenu/jgmenurc  
- `jgmenu init -i` to enter interactive setup  
- `man jgmenututorial` to read step-by-step guide  

# DESCRIPTION

`jgmenu` is a simple menu for Linux/BSD. It reads CSV menu data from a file and
generates a graphical menu on an X11 window.

## Fields

Each line of CSV menu data is parsed into the following fields using comma as a
field separator:

  (1) description  
  (2) command  
  (3) icon  
  (4) working directory  
  (5) metadata  

For example:

    printf "Terminal,xterm\\nWeb Browser,firefox" | jgmenu --vsimple

## Special Characters at beginning of line

`#`
:   Ignore line

`. `
:   A line beginning with a dot followed by a space, will source the file
    specified by the string following the dot

`@`
:   Treat as widget


## Special Characters in fields

`,`
:   As commas are used as field separators, individual fields can only contain
    commas if they are triple quoted. For example:

    foo,"""^pipe(find . -printf '%f,display %p,%p\\n')"""

`< > &`
:   The description field is parsed as pango markup, so `<`, `>`, and `&`
    need to be escaped as `&lt;`, `&gt;`, and `&amp;` respectively.

## Markup

The syntax ^foo(bar) is used to carry out action `foo` with argument `bar`. We
sometimes refer to `bar` as the `inner value`.

The following markup is supported in the *description* field

`sep()`
:   define a separator (with text if argument provided, otherwise without)

The following markup is supported in the *command* field

`^tag()`
:   define a submenu (can be in the *description* field if no other field is
    defined on that line)

`^checkout()`
:   open a submenu in a new window

`^root()`
:   open a submenu in the root window, replacing the current menu

`^sub()`
:   draw a "submenu" arrow

`^back()`
:   check-out parent menu

`^term()`
:   run program in terminal

`^pipe()`
:   execute sub-process and checkout a menu based on its stdout.

`^filter()`
:   invoke search

## Icons

Icons will be displayed if the third field is populated, for example:

    Terminal,xterm,utilities-terminal
    Firefox,firefox,firefox

# OPTIONS

\--no-spawn
:   Redirect command to stdout rather than execute it.

\--checkout=<*tag*>
:   Checkout submenu <*tag*> on startup.

\--config-file=<*file*>
:   Read config file.

\--icon-size=<*size*>
:   Specify icon size (22 by default). If set to 0, icons will not be loaded.

\--at-pointer
:   Launch menu at mouse pointer.

\--hide-on-startup
:   Start menu is hidden state.

\--simple
:   Ignore tint2 settings; Run in 'short-lived' mode (i.e. exit after mouse
    click or enter/escape); read menu items from _stdin_.

\--vsimple
:   Same as --simple, but also disables icons and ignores jgmenurc.

\--csv-file=<*file*>
:   Specify menu file (in jgmenu flavoured CSV format). If file cannot be
    opened, input is reverted to *stdin*.

\--csv-cmd=<*command*>
:   Specify command to produce menu data, for example `jgmenu_run pmenu`

\--die-when-loaded
:   Open menu and then exit(0). Useful for debugging and testing.

\--center
:   Center align menu horizontally and vertically. Also set `tint2_look=0` to
    disable alignment to tint2 panel

# USER INTERFACE

- Up/Down - select previous/next item  
- Left/Right - move to parent/sub menu  
- PgUp/PgDn - scroll up/down by one menu's worth of items  
- Home/End - select first/last item  
- Enter - select an item or open a submenu  
- F5 - restart  
- F8 - print node tree to stderr  
- F9 - exit(1)  
- F10 - exit(0)  
- Backspace - return to parent menu  

Type any string to invoke a search. Words separated by space will be searched
for using OR logic (i.e. the match of either word is sufficient to display an
item).

# CONFIGURATION FILE

If no file is specified using the --config-file= option, the XDG Base Directory
Specification is adhered to. I.e:

- Global config in `${XDG_CONFIG_DIRS:-/etc/xdg}`  
- User config override in `${XDG_CONFIG_HOME:-$HOME/.config}`  

For most users ~/.config/jgmenu/jgmenurc is appropriate.

Global config variables are set in the following order (i.e. bottom
of list has higher precedence):

- built-in defaults (config.c)  
- tint2rc config file (can be specified by `TINT2_CONFIG` environment variable  
- jgmenurc config file (can be specified by --config-file=)  
- command line arguments  

## Syntax

Lines beginning with `#` are ignored.

All other lines are recognised as setting variables in the format

    key = value

White spaces are mostly ignored.

### Values

Unless otherwise specified, values as treated as simple strings.

Here follow some specific types:

`boolean`
:   When a variable takes a boolean value, only 0 and 1 are accepted. 0 means
    false; 1 means true.

`integer`
:   When a variable takes an integer value, only numerical values are accepted.
    The only valid characters are digits (0-9) and minus-sign. All integer
    variables relating to geometry and position are interpreted as pixel values
    unless otherwise specified.

`color`
:   When a variable takes a color value, only the syntax `#rrggbb aaa` is
    recognised, where `rr`, `gg` and `bb` represent hexadecimal values (00-ff)
    for the colours red, green and blue respectively; and `aaa` stands for the
    alpha channel value expressed as a percentage (0-100) (i.e. 100 means no
    transparency and 0 means fully transparent.) For example `#ff0000 100`
    represents red with no transparency, whereas `#000088 50` means dark blue
    with 50% transparency.

`pathname`
:   When a variable takes a pathname value, it is evaluated as a string. If the
    first character is tilde (~), it will be replaced by the the environment
    variable $HOME just as a shell would expand it.

## Variables

`verbosity` = __integer__ (default 0)
:   General verbosity: (0) warnings only; (1) basic info; (2) more info;
    (3) max info

    Additional specific topics: (4) IPC

    Note: Some IPC messages need environment variable `JGMENU_VERBOSE=4` too

`stay_alive` = __boolean__ (default 1)
:   If set to 1, the menu will "hide" rather than "exit" when the following
    events occur: clicking on menu item; clicking outside the menu; pressing
    escape. When in the hidden mode, a USR1 signal will "un-hide" the menu.

`hide_on_startup` = __boolean__ (default 0)
:   If set to 1, jgmenu start in "hidden" mode. This is useful for starting
    jgmenu during the boot process and then sending a `killall -SIGUSR1 jgmenu`
    to show the menu.

`csv_cmd` = __string__ (default `pmenu`)
:   Defines the command to produce the jgmenu flavoured CSV for `jgmenu`.
    Accpetable keyword include pmenu, lx, apps and ob. If a value is given
    other than these keywords, it will be executed in a shell (so be
    careful!). If left blank, jgmenu will read from __stdin__. Examples:

    csv_cmd = lx  
    csv_cmd = jgmenu_run lx --no-dirs  
    csv_cmd = cat ~/mymenu.csv  

`tint2_look` = __boolean__ (default 0)
:   Read tint2rc and parse config options for colours, dimensions
    and alignment.

`position_mode` = (fixed | ipc | pointer | center) (default fixed)
:   Define menu positioning mode. `fixed` aligns to `margin_{x,y}` and respect
    `_NET_WORKAREA`. `ipc` use IPC to read environment variables set by panel.
    `pointer` launches at pointer and respects both `_NET_WORKAREA` and
    `edge_snap_x`. `center` launch at center of screen and ignore
    `_NET_WORKAREA` (takes precedence over `menu_{v,h}align`).

`edge_snap_x` = __integer__ (default 30)
:   Specify the distance (in pixles) from the left hand edge, within which the
    menu will snap to the edge. Note that this only applies in `at_pointer`
    mode.

`terminal_exec` = __string__ (default x-terminal-emulator)
:   Define terminal to use for commands with ^term() markup

`terminal_args` = __string__ (default -e)
:   The values of these two variables are used to build a string to launch
    programs requiring a terminal to run. With the default values, the string
    would become: `x-terminal-emulator -e 'some_command with arguments'`.
    `terminal_args` must finish with `-e` or equivalent, where `-e` refers to
    the meaning of `-e` in `xterm -e`.

`monitor` = __integer__ (default 0)
:   Specify a particular monitor as an index starting from 1. If 0, the menu
    will be launched on the monitor where the mouse is.

`hover_delay` = __integer__ (default 100)
:   The amount of time (in milliseconds) from hovering over an item until a
    submenu is opened.

`hide_back_items` = __boolean__ (default 1)
:   If enabled, all ^back() items will be suppressed. As a general rule, it
    should be set to 1 for a multi-window menu, and 0 when in single-window
    mode.

`columns` = __integer__ (default 1)
:   Specify the number of columns in which to show menu items

`tabs` = __integer__ (default 120)
:   Specify the position is pixels of the first tab

`menu_margin_x` = __integer__ (default 0)
:   Define the distance between the menu (=X11 window) and the edge of the
    screen. See note on `_NET_WORKAREA` under `menu_{v,h}align` variables.

`menu_margin_y` = __integer__ (default 0)
:   Vertical equilvalent of `menu_margin_x`

`menu_width` = __integer__ (default 200)
:   Set the *minimum* menu width. The menu width will adjust to the
    longest item in the current (sub)menu. If a filter is applied
    (e.g. by the user typing) the menu width will NOT adjust.

`menu_height_min` = __integer__ (default 0)
:   Set the minimum height of the root menu. If `menu_height_min` and
    `menu_height_max` these are set to the same value, the menu height will be
    fixed at that value. If set to zero, they will be ignored.

`menu_height_max` = __integer__ (default 0)
:   Set the minimum height of the root menu. See `menu_height_min`

`menu_height_mode` = (static | dynamic) (default static)
:   `static` means that the height of the initial root menu will be used for
    any subsequent ^root() action, whereas `dynamic`means that the root menu
    height will be re-calculated every time the root menu is redefined using
    ^root().

`menu_padding_top` = __integer__ (default 5)
:   Distance between top border and item/widget

`menu_padding_right` = __integer__ (default 5)
:   Distance between right border and item/widget

`menu_padding_bottom` = __integer__ (default 5)
:   Distance between bottom border and item/widget

`menu_padding_left` = __integer__ (default 5)
:   Distance between left border and item/widget

`menu_radius` = __integer__ (default 1)
:   "radius" refers to the size of rounded corners

`menu_border` = __integer__ (default 0)
:   "border" refers to the border-thickness

`menu_halign` = (left | right | center) (default left)
:   Horizontal alignment of menu. If not set, jgmenu will try to guess the
    alignment reading `_NET_WORKAREA`, which is a freedesktop EWMH root window
    property. Not all Window Managers and Panels respect `_NET_WORKAREA`. The
    following do: openbox, xfwm4, tint2 and polybar. The following do NOT:
    awesome, i3, bspwm and plank

`menu_valign` = (top | bottom | center) (default bottom)
:   Vertical alignment of menu. See `menu_halign`.

`sub_spacing` = __integer__ (default 1)
:   Horizontal space between windows. A negative value results in each submenu
    window overlapping its parent window.

`sub_padding_top` = __integer__ (default auto)
`sub_padding_right` = __integer__ (default auto)
`sub_padding_bottom` = __integer__ (default auto)
`sub_padding_left` = __integer__ (default auto)
:   The same as `menu_padding_*` but applies to submenu windows only. It
    understands the keyword 'auto'. If set to 'auto', the smallest of the four
    `menu_padding_*` will be used.

`sub_hover_action` = __integer__ (default 1)
: Open submenu when hovering over item (only works in multi-window mode).

`item_margin_x` = __integer__ (default 3)
`item_margin_y` = __integer__ (default 3)
`item_height` = __integer__ (default 25)
`item_padding_x` = __integer__ (default 4)
`item_radius` = __integer__ (default 1)
`item_border` = __integer__ (default 0)
:   See equivalent `menu_` variable definitions.

`item_halign` = (left | right) (default left)
:   Horizontal alignment of actual menu items. Items are left-aligned by
    default. If set to right, the option `arrow_string` should be changed too.

`sep_height` = __integer__ (default 5)
:   Height of separator without text (defined by ^sep()). Separators with text
    use `item_height`

`sep_halign` = (left | center | right) (default left)
:   Horizontal alignment of separator text

`sep_markup` = __string__ (unset by default)
:   If specified, `<span $sep_markup>foo</span>` will be passed to pango for
    ^sep(foo). See the following link for pango <span> attributes:
    https://developer.gnome.org/pango/stable/PangoMarkupFormat.html

    Keywords include (but are not limited to):
        font  
        size (x-small, small, medium, large, x-large)  
        style (normal, oblique, italic)  
        weight (ultralight, light, normal, bold, ultrabold, heavy  
        foreground (using format #rrggbb or a colour name)  
        underline (none, single, double)  

    Example:  
        `sep_markup = font="Sans Italic 12" foreground="blue"`  

`font` = __string__ (unset by default)  

    *font* accepts a string such as *Cantarell 10* or  
    *UbuntuCondensed 11*. The font description without a specified  
    size unit is interpreted as "points". If "px" is added, it will  
    be read as pixels. Using "points" enables consistency with other  
    applications.

`font_fallback` = __string__ (default xtg)  

    The same as 'icon_theme_fallback' (see below), except that  
    the xsettings variable 'Gtk/FontName' is read.  

`icon_size` = __integer__ (default 22)  

    If icon_size is set to 0, icons will not be searched for and  
    loaded.

`icon_text_spacing` = __integer__ (default 10)  

    Distance between icon and text.  

`icon_theme` = __string__ (unset by default)  

    Specify icon theme.  

`icon_theme_fallback` = __string__ (default xtg)  

    Specifies the fallback sources of the icon theme in order of  
    precedence, where the left-most letter designates the source  
    with the highest precedence. The following are acceptable  
    characters:  

    x = xsettings 'Net/IconThemeName'  
    t = tint2 config file  
    g = gtk3.0 config file  

    'icon_theme' takes priority if set.  

    In order to increase consistency with tint2, xsettings  
    variables will only be read if the tint2rc variable  
    launcher_icon_theme_override is zero.  

`arrow_string` = __string__ (default ▸)  

    The "arrow" indicates that a menu item points a submenu.  
    jgmenuunicode(7) contains arrow suggestions  

`arrow_width` = __integer__ (default 15)  

    Width of area allocated for arrow. Set to 0 to hide arrow.  

`color_menu_bg` = __color__ (default #000000 100)  

    Background colour of menu window  

`color_menu_border` = __color__ (default #eeeeee 8)  

    Border colour of menu window  

`color_norm_bg` = __color__ (default #000000 0)  
`color_norm_fg` = __color__ (default #eeeeee 100)  

    Background and foreground (=font) colors of all menu items,  
    except the one currently selected.  

`color_sel_bg` = __color__ (default #ffffff 20)  
`color_sel_fg` = __color__ (default #eeeeee 100)  
`color_sel_border` = __color__ (default #eeeeee 8)  

    Background, foreground (=font) and border colors of the currently  
    selected menu item.  

`color_sep_fg` = __color__ (default #ffffff 20)  

    Colour of seperators without text  

`color_title_fg` = __color__ (default #eeeeee 50)  
`color_title_bg` = __color__ (default #000000 0)  
`color_title_border` = __color__ (default #000000 0)  

    Foreground (=font), background and border colours of separators  
    with text (also known as 'title')  
    The font colour can be overriden by 'sep_markup'  

`color_scroll_ind` = __color__ (default #eeeeee 40)  

    Colour of scroll indicator lines (which show if there are menu  
    items above or below those which are visible).  

## CSV generator variables

The following variables begin with `csv_` which denotes that they set  
environment variables which are used by the CSV generators.  

`csv_name_format` = __string__ (default `%n (%g)`)  

    Defines the format of the *name* field for CSV generators  
    (currently only applicable to lx). It understands the following  
    two fields:  
        %n - application name  
        %g - application generic name  
    If a *generic name* does not exist or is the same as the *name*,  
    %n will be used without any formatting.  

`csv_single_window` = __boolean__ (default 0)  

    If set, ^root() will be used instead of ^checkout().  
    This results in a single window menu, where submenus appear in  
    the same window.  
    This is currently only supported by pmenu.  

`csv_no_dirs` = __boolean__ (default 0)  

    If set, applications will be listed without any directory  
    structure. This is currently only supported by pmenu and lx.  

`csv_i18n` = __string__ (no default)  

    If set, the ob module will look for a translation file in the  
    specified file or directory. See `jgmenu_run i18n --help` for  
    further details.  

# Inter-process Communication (IPC)

`jgmenu_run` reads the environment variables listed below and passes
them via a unix socket to the long-running instance of jgmenu. On
'wake up' (=show menu), jgmenu aligns the menu to these variables.

`TINT2_BUTTON_ALIGNED_X1`  
`TINT2_BUTTON_ALIGNED_X2`  
`TINT2_BUTTON_ALIGNED_Y1`  
`TINT2_BUTTON_ALIGNED_Y2`  
`TINT2_BUTTON_PANEL_X1`  
`TINT2_BUTTON_PANEL_X2`  
`TINT2_BUTTON_PANEL_Y1`  
`TINT2_BUTTON_PANEL_Y2`  

If thse variables are not set, jgmenurc config variables `margin_{x,y}`
are reverted to.

# DIAGRAMS {#diagrams}

## General Notes

`margin`
:   Refers to space outside an object

`padding`
:   Refers to space inside an object (between border and content)

## Vertical Menu

```
╔════════════════════════╗
║            1           ║
╟────────────────────────╢
║            2           ║
╟────────────────────────╢
║          item          ║
╟────────────────────────╢
║            2           ║
╟────────────────────────╢
║          item          ║
╟────────────────────────╢
║            2           ║
╟────────────────────────╢
║            3           ║
╚════════════════════════╝

1. menu_padding_top
2. item_margin_y
3. menu_padding_bottom
```

## Horizontal Menu

```
╔═╤═╤════════════════╤═╤═╗
║ │ │                │ │ ║
║ │ ├────────────────┤ │ ║
║ │ │ @    web      >│ │ ║
║ │ ├────────────────┤ │ ║
║2│1│                │1│3║
║ │ ├───┬─┬────────┬─┤ │ ║
║ │ │ 4 │5│        │6│ │ ║
║ │ ├───┴─┴────────┴─┤ │ ║
║ │ │                │ │ ║
║ │ │                │ │ ║
╚═╧═╧════════════════╧═╧═╝

1. item_margin_x
2. padding_left
3. padding_right
4. icon_size
5. icon_to_text_spacing
6. arrow_width
```

## External to menu

```
    screen
    ╔════════════════════════╗
    ║    2                   ║
    ║ ┌──────┐               ║
    ║ │ root │ ┌──────┐      ║
    ║1│ menu │ │ sub  │      ║
    ║ │      │3│ menu │      ║
    ║ └──────┘ │      │      ║
    ║          └──────┘      ║
    ║                        ║
    ║                        ║
    ║                        ║
    ╚════════════════════════╝

1. menu_margin_x
2. menu_margin_y
3. sub_spacing
```

# SEE ALSO

`jgmenu_run(1)`  
`jgmenututorial(7)`  

The jgmenu source code and documentation can be downloaded from  
<https://github.com/johanmalm/jgmenu/>

