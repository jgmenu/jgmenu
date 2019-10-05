#!/usr/bin/env python3

# Copyright (C) @Misko_2083 2019

import gi
gi.require_version("Gtk", "3.0")
gi.require_version("Gdk", "3.0")
gi.require_version('MatePanelApplet', '4.0')
from gi.repository import Gtk, Gdk
from gi.repository import MatePanelApplet
from gi.repository.GdkPixbuf import Pixbuf
import os
import shlex
import subprocess
import setproctitle

# Rename the process
setproctitle.setproctitle('mate-jgmenu')

class JgMenu(MatePanelApplet.Applet):
    def __init__(self, applet):
        self.applet_fill(applet)

    def execute(self, command):
        subprocess.Popen(shlex.split(command),
                         stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    def applet_fill(self, applet):
         # you can use this path with gio/gsettings
        settings_path = applet.get_preferences_path()
        applet.set_border_width(0)
        self.button = Gtk.Button(border_width=0)
        try:
           pixbuf = Gtk.IconTheme.get_default().load_icon("start-here", applet.get_size(), 0)
           button_icon = Gtk.Image.new_from_pixbuf(pixbuf)
        except:
           button_icon = Gtk.Label("jgmenu")
        self.button.add(button_icon)
        # Disable label to show icon
#        self.button.set_label("jgmenu")
        self.button.connect("clicked", self.on_toggle, self.button, applet)
        self.applet = applet
        self.applet.add(self.button)
        toplevel = self.button.get_toplevel()
        self.applet.show_all()

    def on_toggle(self, button, applet, data=None):
        # make sure we're in ipc mode - won't touch jgmenurc if already in ipc
        self.execute("jgmenu_run config -s {} -k position_mode -v ipc".format(os.environ["HOME"] + '/.config/jgmenu/jgmenurc'))

        rect = button.get_allocation()
        main_window = self.button.get_toplevel()
        [val, button_x1, button_y1] = main_window.get_window().get_origin()
        button_x2 = button_x1 + rect.width
        button_y2 = button_y1 + rect.height
        button_y1=0
        orientation = self.applet.get_orient()

        # Cheeky hack to watch a few variables
        f = open("/home/johan/mate-temp", 'w')
        f.write("x={} ".format(button_x2))
        f.write("y={} ".format(button_y2))
        f.write("\n")
        f.close()

        os.environ["TINT2_BUTTON_PANEL_X1"] = "0"
        os.environ["TINT2_BUTTON_PANEL_Y1"] = "0"
        os.environ["TINT2_BUTTON_PANEL_X2"] = "1024"
        os.environ["TINT2_BUTTON_PANEL_Y2"] = "{}".format(button_y2)
        os.environ["TINT2_BUTTON_ALIGNED_X1"] = "{}".format(button_x1)
        os.environ["TINT2_BUTTON_ALIGNED_Y1"] = "{}".format(button_y2)
        os.environ["TINT2_BUTTON_ALIGNED_X2"] = "{}".format(button_x2)
        os.environ["TINT2_BUTTON_ALIGNED_Y2"] = "{}".format(button_y2)
        self.execute("jgmenu_run")

def applet_factory(applet, iid, data):
    if iid != "JgMenuApplet":
        return False
    JgMenu(applet)
    return True

if __name__ == "__main__":
    MatePanelApplet.Applet.factory_main("JgMenuAppletFactory", True,
                                    MatePanelApplet.Applet.__gtype__,
                                    applet_factory, None)
