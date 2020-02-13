% JGMENU-I18N(1)
% Johan Malm
% 15 February, 2020

# NAME

jgmenu-i18n - support translation of jgmenu flavoured CSV menu data

# SYNOPSIS

jgmenu_run i18n <*translation-file*>

jgmenu_run i18n \--init \[<*translation-file*>]

# DESCRIPTION

`jgmenu_run i18n` reads jgmenu flavoured CSV menu data from stdin and either
translates it, or creates a po style translation file

The <*translation-file*> argument can be a file or directory. If it is the
latter, translation files will be searched for in this directory based on the
environment variable $LANG, which will be assumed to be in the format
`ll_CC.UTF8` where `ll` is an ISO 639 two-letter language code and `CC` is an
ISO 3166 two-letter country code.  Files named `ll_CC` and `ll` will be used
in said order.

# OPTIONS

`--init`

:   Create a template po style translation file with blank msgid entries

# EXAMPLES

## Create translation file template for manual translation

Modules jgmenu-apps(1) and jgmenu-ob(1) have built-in i18n support. In order
to use this features, a translation file is required. Here follow examples of
how a translation file can be created:

Example 1.

    jgmenu_run ob | jgmenu_run i18n --init >sv

Example 2.

    jgmenu_run i18n --init <prepend.csv >sv

The result of the above commands would be a skeleton translation file (`sv` in
this case) containing entries like this:

    msgid "Web Browser"
    msgstr ""

Manually translate the empty msgstr fields.

## Use translation file

If using modules jgmenu-apps(1) or jgmenu-ob(1), just set `csv_i18n` in
jgmenurc to directly point to a translation-file or a directory containing
translation files.

In special circumstances, you may wish to include i18n in command plumbing.
Here follow an example of how this could be constructed:

    <csv-generating-command> \
    	| jgmenu_run i18n <translation-file> \
        | jgmenu --vsimple

