#!/usr/bin/env python3

# Copyright (C) @Misko_2083 2019
# Copyright (C) Johan Malm 2019

""" Parse gtk theme and set some key/value pairs in jgmenurc """

import os
import sys
import shlex
import subprocess
try:
    import gi
except ImportError:
    print("[gtktheme] fatal: require python3-gi")
    sys.exit(1)

gi.require_version("Gtk", "3.0")
from gi.repository import Gtk

def run(command):
    """ run a command """
    proc = subprocess.Popen(shlex.split(command))
    proc.wait()

def fmt(s):
    """ ensure string is at least two characters long """
    if len(s) == 0:
        return "00"
    elif len(s) == 1:
        return "0{}".format(s)
    return s

def rgb2hex(line):
    """ convert rgb values to a 6-digit hex string """
    s = line.split("rgb(")
    rgb = s[-1].replace(");", "").split(",")
    r = hex(int(rgb[0]))[2:]
    g = hex(int(rgb[1]))[2:]
    b = hex(int(rgb[2]))[2:]
    return "{}{}{}".format(fmt(r), fmt(g), fmt(b))

def setconfig(key, value):
    """ set key/value pain in ~/.config/jgmenu/jgmenurc """
    filename = os.environ["HOME"] + "/.config/jgmenu/jgmenurc"
    value = "#{} 100".format(value)
    print("[gtktheme] {} = {}".format(key, value))
    cmd = "jgmenu_run config -s {} -k {} -v '{}'".format(filename, key, value)
    run(cmd)

def process_line(line):
    """ process one line """
    if "background-color" in line:
        setconfig("color_menu_bg", rgb2hex(line))
        setconfig("color_title_bg", rgb2hex(line))
        setconfig("color_title_border", rgb2hex(line))

def cache(themename):
    """ save the theme-name to XDG_CACHE_HOME/jgmenu/.last-gtktheme or 
        ~/.cache/jgmenu/.last-gtktheme if it doesn't exist """
    print("themename={}".format(themename))
    home = os.environ["HOME"]
    directory = os.getenv("XDG_CACHE_HOME", home + "/.cache") + "/jgmenu"
    if not os.path.exists(directory):
        os.mkdir(directory)
    f = open(directory + "/.last-gtktheme", "w")
    f.write(themename)
    f.close()

def main():
    """ main """
    gset = Gtk.Settings.get_default()
    themename = gset.get_property("gtk-theme-name")
    cache(themename)
    prefdark = gset.get_property("gtk-application-prefer-dark-theme")
    css = Gtk.CssProvider.get_named(themename).to_string()
#    print(css)
#    exit(1)
    lines = css.split("\n")

    # parse some @define-color lines
    for line in lines:
        if "@define-color" not in line:
            break
        if "theme_text_color" in line:
            setconfig("color_norm_fg", rgb2hex(line))
            setconfig("color_title_fg", rgb2hex(line))
        if "theme_selected_bg_color" in line:
            setconfig("color_sel_bg", rgb2hex(line))
        if "theme_selected_fg_color" in line:
            setconfig("color_sel_fg", rgb2hex(line))

    # parse the menu { } section
    inside = False
    for line in lines:
        if "menu {" in line:
            inside = True
            continue
        if inside:
            if "{" in line:
                inside = False
                break
            process_line(line)

if __name__ == '__main__':
    main()

