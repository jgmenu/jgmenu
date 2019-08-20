#!/usr/bin/env python3

# Copyright (C) @Misko_2083 2019

import gi
gi.require_version("Gtk", "3.0")
from gi.repository import Gtk, Gdk

def main():
    gset = Gtk.Settings.get_default()
    themename = gset.get_property("gtk-theme-name")
    prefdark = gset.get_property("gtk-application-prefer-dark-theme")
    css = Gtk.CssProvider.get_named(themename)
    print(css.to_string())

if __name__ == '__main__':
    main()
