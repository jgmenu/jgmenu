% JGMENU-OB(1)
% Johan Malm
% 3 December, 2019

# NAME

jgmenu-ob - convert openbox menu data to jgmenu flavoured CSV

# SYNOPSIS

`jgmenu_run ob` \[\--tag=<*tag*>] \[\--cmd=<*cmd*> | <*file*>]

# DESCRIPTION

`jgmenu_run ob` coverts openbox XML menu data to jgmenu flavoured CSV. If no
`<file>` or `--cmd=<cmd>` is specified, `~/.config/openbox/menu.xml` or
`/etc/xdg/openbox/menu.xml` will be used if they exist, giving higher
precedence to the former.

If environment variable `JGMENU_I18N` or config variable `csv_i18n` are set, a
translation file will be searched for in the specified file or directory. See
`jgmenu_run i18n --help` for further details.

# OPTIONS

`<file>`

:   Specify openbox XML file.

`--cmd=<cmd>`

:   Specify command to produce openbox XML menu data.

`--tag=<tag>`

:   Specify value of root `^tag()`. This can be useful for pipemenus to avoid
    clashing tag-names

