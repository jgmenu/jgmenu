#!/usr/bin/env python3

# Copyright (C) @Misko_2083 2019

import gi
gi.require_version("Gtk", "3.0")
from gi.repository import Gtk, Gdk

gset = Gtk.Settings.get_default ()
themename = gset.get_property ("gtk-theme-name")

# it's a boolean
prefdark = gset.get_property ("gtk-application-prefer-dark-theme")

cprov = Gtk.CssProvider.get_named (themename)
print (cprov.to_string())


