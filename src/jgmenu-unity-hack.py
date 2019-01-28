#!/usr/bin/env python3

"""
This is a quick hack to give the Unity launcher an invisible and short-lived
"override_redirect=False" window. This is necessary in order to
  - stop the launcher-button flashing for several seconds;
  - enable the launcher-button to be clicked again.

If jgmenu's "override_redirect" window attribute is set to "True" in
x11-ui.c, a new icon will appear in the launcher which we don't want.

If anyone needs this for anything other than Ubuntu's Unity, I'll consider
re-writing it in C to avoid the gtk-3.0 dependency.
"""

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk
from gi.repository import GLib

def q():
    Gtk.main_quit()
    return False

def timer(widget, data=None):
    GLib.timeout_add(250, q)

window = Gtk.Window()
window.set_opacity(0)
window.set_default_size(0,0)
window.connect("show", timer)
window.show_all()
Gtk.main()
