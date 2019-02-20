#!/usr/bin/env python3
#
# Copyright (C) Johan Malm 2018
#

""" create/modify/query jgmenu config files """

import argparse
import sys
import os

DEFAULT_CONFIG_FILE = "~/.config/jgmenu/jgmenurc"

def jgmenurc():
    """ return dict with config file keys and values """
    keys_and_values = {
        "stay_alive": "1",
        "hide_on_startup": "0",
        "csv_cmd": "pmenu",
        "tint2_look": "1",
        "at_pointer": "0",
        "edge_snap_x": "30",
        "terminal_exec": "x-terminal-emulator",
        "terminal_args": "-e",
        "monitor": "0",
        "hover_delay": "100",
        "hide_back_items": "1",
        "columns": "1",
        "tabs": "120",
        "menu_margin_x": "0",
        "menu_margin_y": "0",
        "menu_width": "200",
        "menu_height_min": "0",
        "menu_height_max": "0",
        "menu_height_mode": "static",
        "menu_padding_top": "5",
        "menu_padding_right": "5",
        "menu_padding_bottom": "5",
        "menu_padding_left": "5",
        "menu_radius": "1",
        "menu_border": "0",
        "menu_halign": "left",
        "menu_valign": "bottom",
        "sub_spacing": "1",
        "sub_padding_top": "auto",
        "sub_padding_right": "auto",
        "sub_padding_bottom": "auto",
        "sub_padding_left": "auto",
        "sub_hover_action": "1",
        "item_margin_x": "3",
        "item_margin_y": "3",
        "item_height": "25",
        "item_padding_x": "4",
        "item_radius": "1",
        "item_border": "0",
        "item_halign": "left",
        "sep_height": "5",
        "sep_halign": "left",
        "sep_markup": "",
        "font": "",
        "font_fallback": "xtg",
        "icon_size": "22",
        "icon_text_spacing": "10",
        "icon_theme": "",
        "icon_theme_fallback": "xtg",
        "arrow_string": "▸",
        "arrow_width": "15",
        "color_menu_bg": "#000000 85",
        "color_menu_border": "#eeeeee 8",
        "color_norm_bg": "#000000 00",
        "color_norm_fg": "#eeeeee 100",
        "color_sel_bg": "#ffffff 20",
        "color_sel_fg": "#eeeeee 100",
        "color_sel_border": "#eeeeee 8",
        "color_sep_fg": "#ffffff 20",
        "color_scroll_ind": "#eeeeee 40",
        "csv_name_format": "%n (%g)",
        "csv_single_window": "0",
        "csv_no_dirs": "0",
        "csv_i18n": ""
        }
    return keys_and_values

def die(msg):
    """ exit with message """
    sys.stderr.write("fatal: {}\n".format(msg))
    sys.exit(1)

def get_key(line):
    """ return key from "key = value" string """
    s = ''
    if '=' in line:
        s = line.split('=', 1)[0].strip()
        if s[0] == '#':
            s = s[1:]
    return s

def get_keys(lines):
    """ extract keys from config file """
    keys = []
    for line in lines:
        s = get_key(line)
        if s and s != '':
            keys.append(s)
    return keys

def get_missing_keys(conf_lines):
    """ compare config file with built-in dict and return missing keys """
    conf_keys = get_keys(conf_lines)
    return [item for item in jgmenurc() if item not in set(conf_keys)]

def resolve(filename):
    """ expand ~ and $foo """
    if filename is None or filename == '':
        die("filename must be specified")
    filename = os.path.expanduser(filename)
    filename = os.path.expandvars(filename)
    return filename

def amend(conf_filename, isdryrun):
    """ amend config-file with missing items """
    conf_filename = resolve(conf_filename)
    if not os.path.exists(conf_filename):
        die("file '{}' does not exist".format(conf_filename))
    with open(conf_filename, "r+") as f:
        conf_lines = f.read().split("\n")

    missing_keys = get_missing_keys(conf_lines)
    if not missing_keys:
        print("info: there are no missing items in {}".format(conf_filename))
        return

    if isdryrun:
        print("info: missing keys {}".format(missing_keys))
        return

    print("info: add keys {}".format(missing_keys))
    for key in missing_keys:
        conf_lines.append("#{} = {}".format(key, jgmenurc()[key]))

    # write config file
    with open(conf_filename, "w") as f:
        for line in conf_lines:
            f.write("{}\n".format(line))

def create(conf_filename):
    """ create new config file """
    conf_filename = resolve(conf_filename)
    if os.path.exists(conf_filename):
        die("file '{}' already exists".format(conf_filename))
    l = []
    d = jgmenurc()
    for key in d:
        l.append("#{} = {}".format(key, d[key]))
    with open(conf_filename, "w") as f:
        for line in l:
            f.write("{}\n".format(line))

def main():
    """ script main program """
    parser = argparse.ArgumentParser(prog="jgmenu_run config")
    subparsers = parser.add_subparsers(dest="command", help="commands")
    subparsers.required = True
    create_parser = subparsers.add_parser("create", help="create config-file")
    create_parser.add_argument("--file", default=DEFAULT_CONFIG_FILE,
                               help="specify config file", metavar="FILE")
    amend_parser = subparsers.add_parser("amend", help="amend config-file \
                                         with missing items")
    amend_parser.add_argument("--file", metavar="FILE", help="specify config \
                              file", default=DEFAULT_CONFIG_FILE)
    amend_parser.add_argument("--dryrun", action="store_true",
                              help="list missing items, but do not amend file")
    args = parser.parse_args()
    if args.command == "create":
        create(args.file)
    elif args.command == "amend":
        amend(args.file, args.dryrun)
    else:
        parser.print_help(sys.stderr)

if __name__ == '__main__':
    main()
