% JGMENU(1)
% Johan Malm
% 4 January, 2019

# NAME

jgmenu - A simple X11 menu

# SYNOPSIS

jgmenu \[\--no-spawn] \[\--checkout=<*tag*>] \[\--config-file=<*file*>]  
       \[\--icon-size=<*size*>] \[\--at-pointer] \[\--hide-on-startup]  
       \[\--simple] \[\--vsimple] \[\--csv-file=<*file*>]  
       \[\--csv-cmd=<*command*>] \[\--die-when-loaded]  
       \[\--center]

Use these commands to get started

`jgmenu_run`

:   Launch menu

`jgmenu_run init`

:   Create config file ~/.config/jgmenu/jgmenurc

`jgmenu_run init -i`

:   Interactive setup

`man jgmenututorial`

:   Step-by-step tutorial

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
refer to `bar` as the `inner value`.

The following markup is supported in the `description` field

`sep()`

:   Define a separator. If an inner value is provided, the separator will
    appear as a title. If no inner value is provided, the separator will simply
    be a horizontal line

The following markup is supported in the `command` field

`^tag()`

:   Define the beginning of a new menu structure node.

    The lines following `^tag(whatever)` will not show in the top level menu,
    but can be opened by using `^checkout()` or `^root()`.


        item0.0
        item0.1
        submenu1,^checkout(1)
        submenu2,^root(2)

        ^tag(1)
        item1.0
        item1.1

        ^tag(2)
        item2.0
        item2.1

`^checkout()`

:   Open the tag specified by the inner value as a submenu in a new window

`^root()`

:   Open the tag specified by the inner value in the root window, replacing the
    current menu

`^sub()`

:   Draw a submenu arrow. This can be useful for creating submenus with
    `stay_alive=0`. For example:

        item0.0
        item0.1
        submenu1,^sub(echo "item1.0" | jgmenu --simple)

`^back()`

:   Check-out parent menu

`^term()`

:   Run program in terminal

`^pipe()`

:   Execute sub-process and checkout a menu based on its stdout.

`^filter()`

:   Invoke search

## Icons

Icons will be displayed if the third field is populated, for example:

    Terminal,xterm,utilities-terminal
    Firefox,firefox,firefox

# OPTIONS

`--no-spawn`

:   Redirect command to stdout rather than execute it.

`--checkout=<tag>`

:   Checkout submenu <tag> on startup.

`--config-file=<file>`

:   Read config file.

`--icon-size=<size>`

:   Specify icon size (22 by default). If set to 0, icons will not be loaded.

`--at-pointer`

:   Launch menu at mouse pointer.

`--hide-on-startup`

:   Start menu is hidden state.

`--simple`

:   Ignore tint2 settings; Run in short-lived mode (i.e. exit after mouse
    click or enter/escape); read menu items from `stdin`.

`--vsimple`

:   Same as `--simple`, but also disables icons and ignores jgmenurc.

`--csv-file=<file>`

:   Specify menu file (in jgmenu flavoured CSV format). If file cannot be
    opened, input is reverted to `stdin`.

`--csv-cmd=<command>`

:   Specify command to produce menu data, for example `jgmenu_run pmenu`

`--die-when-loaded`

:   Open menu and then exit(0). Useful for debugging and testing.

`--center`

:   Center align menu horizontally and vertically.

# USER INTERFACE

`Up`, `Down`

:   Select previous/next item

`Left`. `Right`

:   Move to parent/sub menu

`PgUp`, `PgDn`

:   Scroll up/down

`Home`, `End`

:   Select first/last item

`Enter`

:   Select an item or open a submenu

`F5`

:   Restart

`F8`

:   Print node tree to stderr

`F9`

:   exit(1)

`F10`

:   exit(0)

`Backspace`

:   Return to parent menu

Type any string to invoke a search. Words separated by space will be searched
for using `OR` logic (i.e. the match of either word is sufficient to display an
item).

# WIDGETS {#widgets}

Lines beginning with '@' in jgmenu flavoured CSV files are parsed in
accordance with the following syntax:

    @type,action,x,y,w,h,r,halign,valign,fgcol,bgcol,content

`type`

:   The widget type, which can be one of the following:

    `rect`

    :   Rectangle with a 1px thick border drawn using `fgcol`

    `search`

    :   Search box showing the current filter (what the user has typed)
        or the specifed `text` if no filter has been invoked. 

    `icon`

    :   Icon

`action`

:   The action to take when selected. This can either be a shell command or
    a menu action such `^root()`.

`x`, `y`

:   Horizontal and vertical margin of widget

`w`, `h`

:   Width and height of widget

`r`

:   Corner radius

`fgcol`, `bgcol`

:   Foreground and background colours using syntax `rrggbb #aa`
    `fgcol` accepts `auto` to use the jgmenurc's `color_norm_fg`

`content`

:   `icon_path` for `icon` widgets
    `text` for all other widget types

`halign`, `valign`

:    Horizontal and vertical alignment of widget.
     This has not yet been implemented, but defaults to `top` and `left`

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
    careful!). If left blank, jgmenu will read from `stdin`. Examples:

        csv_cmd = lx
        csv_cmd = jgmenu_run lx --no-dirs
        csv_cmd = cat ~/mymenu.csv

`tint2_look` = __boolean__ (default 0)

:   Read tint2rc and parse config options for colours, dimensions and
    alignment.

`position_mode` = (fixed | ipc | pointer | center) (default fixed)

:   Define menu positioning mode.

    `fixed`

    :   Align to `margin_{x,y}` and respect `_NET_WORKAREA`.

    `ipc`

    :   Use IPC to read environment variables set by panel.
        See [Inter-Process Communication](#ipc) for further info.

    `pointer`

    :   Launch at pointer whilst respecting both `_NET_WORKAREA` and
        `edge_snap_x`.

    `center`

    :   Launch at center of screen and ignore `_NET_WORKAREA`.
        Take precedence over `menu_{v,h}align`.

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

:   Time (in milliseconds) from hovering over an item until a submenu is
    opened.

`hide_back_items` = __boolean__ (default 1)

:   If enabled, all ^back() items will be suppressed. As a general rule, it
    should be set to 1 for a multi-window menu, and 0 when in single-window
    mode.

`columns` = __integer__ (default 1)

:   Number of columns in which to show menu items

`tabs` = __integer__ (default 120)

:   Specify the position is pixels of the first tab

`menu_margin_x` = __integer__ (default 0)

:   Distance between the menu (=X11 window) and the edge of the screen. See
    note on `_NET_WORKAREA` under `menu_{v,h}align` variables.

`menu_margin_y` = __integer__ (default 0)

:   Vertical equilvalent of `menu_margin_x`

`menu_width` = __integer__ (default 200)

:   Minimum menu width of the menu. The menu width will adjust to the
    longest item in the current (sub)menu. If a filter is applied
    (e.g. by the user typing) the menu width will not adjust.

`menu_height_min` = __integer__ (default 0)

:   Set the minimum height of the root menu. If `menu_height_min` and
    `menu_height_max` these are set to the same value, the menu height will be
    fixed at that value. If set to zero, they will be ignored.

`menu_height_max` = __integer__ (default 0)

:   Minimum height of the root menu. See `menu_height_min`

`menu_height_mode` = (static | dynamic) (default static)

:   `static`

    :   Height of the initial root menu will be used for any subsequent
        `^root()` action

    `dynamic`

    :   Root menu height will be re-calculated every time a new tag is opened
        using `^root()`.

`menu_padding_top` = __integer__ (default 5)

:   Distance between top border and item/widget

`menu_padding_right` = __integer__ (default 5)

:   Distance between right border and item/widget

`menu_padding_bottom` = __integer__ (default 5)

:   Distance between bottom border and item/widget

`menu_padding_left` = __integer__ (default 5)

:   Distance between left border and item/widget

`menu_radius` = __integer__ (default 1)

:   Radius of rounded corners of menu

`menu_border` = __integer__ (default 0)

:   Thickness of menu border

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

:   Same as `menu_padding_top` but applies to submenu windows only. It
    understands the keyword `auto` which means that the smallest of the four
    `menu_padding_*` variables will be used.

`sub_padding_right` = __integer__ (default auto)

:   See `sub_padding_top`

`sub_padding_bottom` = __integer__ (default auto)

:   See `sub_padding_top`

`sub_padding_left` = __integer__ (default auto)

:   See `sub_padding_top`

`sub_hover_action` = __integer__ (default 1)

:   Open submenu when hovering over item (only works in multi-window mode).

`item_margin_x` = __integer__ (default 3)

:   Horizontal distance between items and the edge of the menu.

`item_margin_y` = __integer__ (default 3)

:   Vertical distance between items and the edge of the menu.

`item_height` = __integer__ (default 25)

:   Height of menu items.

`item_padding_x` = __integer__ (default 4)

:   Horizontal distance between item edge and its content (e.g. text or icon)

`item_radius` = __integer__ (default 1)

:   Radius of rounded corners of items

`item_border` = __integer__ (default 0)

:   Thickness of item border

`item_halign` = (left | right) (default left)

:   Horizontal alignment of menu items. If set to `right`, the option
    `arrow_string` should be changed too.

`sep_height` = __integer__ (default 5)

:   Height of separator without text (defined by ^sep()). Separators with text
    use `item_height`

`sep_halign` = (left | center | right) (default left)

:   Horizontal alignment of separator text

`sep_markup` = __string__ (unset by default)

:   If specified, `<span $sep_markup>foo</span>` will be passed to pango for
    ^sep(foo).

    See the following link for pango <span> attributes:
    [https://developer.gnome.org/pango/stable/pango-Markup.html](https://developer.gnome.org/pango/stable/pango-Markup.html)

    Keywords include (but are not limited to):

    - font
    - size (x-small, small, medium, large, x-large)
    - style (normal, oblique, italic)
    - weight (ultralight, light, normal, bold, ultrabold, heavy
    - foreground (using format #rrggbb or a colour name)
    - underline (none, single, double)

    Example:

        sep_markup = font="Sans Italic 12" foreground="blue"

`font` = __string__ (unset by default)

:   Font description for menu items. `font` accepts a string such as
    `Cantarell 10` or `UbuntuCondensed 11`. The font description without a
    specified size unit is interpreted as `points`. If `px` is added, it will
    be read as pixels. Using "points" enables consistency with other
    applications.

`font_fallback` = __string__ (default xtg)

:   Same as `icon_theme_fallback`, except that the xsettings variable
    `Gtk/FontName` is read.

`icon_size` = __integer__ (default 22)

:   Size of icons in pixels. If set to 0, icons will be disabled.

`icon_text_spacing` = __integer__ (default 10)

:   Distance between icon and text within a menu item

`icon_theme` = __string__ (unset by default)

:   Name of icon theme. E.g. `Adwaita`, `breeze`, `Paper`, `Papirus` and
    `Numix`. See `ls /usr/share/icons/` (or similar) for available icon themes
    on your system.

`icon_theme_fallback` = __string__ (default xtg)

:   Fallback sources of the icon theme in order of precedence, where the
    left-most letter designates the source with highest precedence. The
    following characters are acceptable: `x=xsettings Net/IconThemeName`;
    `t=tint2`; `g=gtk3.0`. `icon_theme` takes priority if set.
    In order to increase consistency with tint2, xsettings variables will only
    be read if the tint2rc variable `launcher_icon_theme_override` is `0`.

`arrow_string` = __string__ (default ▸)

:   String to be used to indicate that an item will open submenu.
    See jgmenuunicode(7) for examples

`arrow_width` = __integer__ (default 15)

:   Width allowed for `arrow_string`. Set to 0 to hide arrow.

`color_menu_bg` = __color__ (default #000000 100)

:   Background colour of menu window

`color_menu_border` = __color__ (default #eeeeee 8)

:   Border colour of menu window

`color_norm_bg` = __color__ (default #000000 0)

:   Background colour of menu items, except the one currently selected.

`color_norm_fg` = __color__ (default #eeeeee 100)

:   Font (foreground) colour of menu items, except the one currently selected.

`color_sel_bg` = __color__ (default #ffffff 20)

:   Background color of the currently selected menu item.

`color_sel_fg` = __color__ (default #eeeeee 100)

:   Font (foreground) color of the currently selected menu item.

`color_sel_border` = __color__ (default #eeeeee 8)

:   Border color of the currently selected menu item.

`color_sep_fg` = __color__ (default #ffffff 20)

:   Font (foreground) colour of seperators without text

`color_title_fg` = __color__ (default #eeeeee 50)

:   Font (foreground) colour of separators with text. The font colour can be
    overriden by `sep_markup`

`color_title_bg` = __color__ (default #000000 0)

:   Background colour of separators with text.

`color_title_border` = __color__ (default #000000 0)

:   Border colour of separators with text.

`color_scroll_ind` = __color__ (default #eeeeee 40)

:   Colour of scroll indicator lines (which show if there are menu items above
    or below those which are visible).

## CSV generator variables

The following variables begin with `csv_` which denotes that they set
environment variables which are used by the CSV generators.

`csv_name_format` = __string__ (default `%n (%g)`)

:   Defines the format of the `name` field for CSV generators. Supported by
    apps and lx. It understands the following two fields:

    `%n`

    :   Application name

    `%g`

    :   Application generic name. If a `generic name` does not exist or is
        the same as the `name`, `%n` will be used without any formatting.

`csv_single_window` = __boolean__ (default 0)

:   If set, csv-generators will output ^root() instead of ^checkout().
    This results in a single window menu, where submenus appear in the same
    window. This is supported by apps and pmenu.

`csv_no_dirs` = __boolean__ (default 0)

:   If set, csv-generators will output applications without any director
    structure. This is supported by apps, pmenu and lx.

`csv_i18n` = __string__ (no default)

:   If set, the ob module will look for a translation file in the
    specified file or directory. See `jgmenu_run i18n --help` and
    `jgmenu-ob(1)` for further details.

# Inter-Process Communication (IPC) {#ipc}

IPC can be used to align jgmenu to a panel launcher in real-time. This is
currently supported by tint2 and xfce-panel. It works as follows:

`jgmenu_run` reads the environment variables listed below and passes them via a
unix socket to the long-running instance of jgmenu.

If `position_mode=ipc`, jgmenu aligns to these variables every times it is
launched.

The following four environment variables define the extremities of the panel:
`TINT2_BUTTON_PANEL_X1`, `TINT2_BUTTON_PANEL_X2`, `TINT2_BUTTON_PANEL_Y1`,
`TINT2_BUTTON_PANEL_Y2`.

```
(X1,Y1)
╔══════════════════════╗
║ panel                ║
╚══════════════════════╝
                 (X2,Y2)
```

The following environment variables define the position of the launcher. These
are interpreted differently depending on panel alignment.

In the case of a horizontal panel:

- `TINT2_BUTTON_ALIGNED_X1` and `TINT2_BUTTON_ALIGNED_X2` define the launcher
  button's horizontal extremities to align to.

- `TINT2_BUTTON_ALIGNED_Y1` and `TINT2_BUTTON_ALIGNED_Y2` define the edge of
  the panel to align to. These shall be the same.

In the case or a vertical panel, the same rules apply with X and Y reversed.

If the above variables are not set, `menu_margin_x` and `menu_margin_y` are
used.

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

- `jgmenu_run(1)`
- `jgmenututorial(7)`
- `jgmenuunicode(7)`

The jgmenu source code and documentation can be downloaded from
[https://github.com/johanmalm/jgmenu/](https://github.com/johanmalm/jgmenu/)

