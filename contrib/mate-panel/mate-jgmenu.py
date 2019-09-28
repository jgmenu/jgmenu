#!/usr/bin/env python3

import gi
gi.require_version("Gtk", "3.0")
gi.require_version("Gdk", "3.0")
gi.require_version('MatePanelApplet', '4.0')
from gi.repository import Gtk, Gdk
from gi.repository import MatePanelApplet
from gi.repository.GdkPixbuf import Pixbuf
import os
import sys
import shlex
import subprocess
import re
import setproctitle

# Rename the process
setproctitle.setproctitle('mate-jgmenu')

HOME=os.environ['HOME']
CONFIG_PATH=HOME + '/.config/jgmenu/jgmenurc'


class JgMenu(MatePanelApplet.Applet):
    def __init__(self, applet):
        self.applet_fill(applet)


    def execute(self, command):
        """function to exec"""
        subprocess.Popen(shlex.split(command), 
                         stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    def applet_fill(self, applet):

         # you can use this path with gio/gsettings
        settings_path = applet.get_preferences_path()

        applet.set_border_width(0)
        self.button = Gtk.Button(border_width=0)
        try:
           pixbuf = Gtk.IconTheme.get_default().load_icon("distributor-logo", applet.get_size(), 0)
           button_icon = Gtk.Image.new_from_pixbuf(pixbuf)
        except:
           button_icon = Gtk.Label("jgmenu")
        self.button.add(button_icon)
        # Disable label to show icon
        self.button.set_label("JG menu")
        self.button.connect("clicked", self.on_toggle, self.button, applet)
        self.applet = applet
        self.applet.add(self.button)
        toplevel = self.button.get_toplevel()

        self.applet.show_all()


    def replace(self, file, pattern, subst):
        file_handle = open(file, 'r')
        file_string = file_handle.read()
        file_handle.close()
        file_string = (re.sub(pattern, subst, file_string))
        file_handle = open(file, 'w')
        file_handle.write(file_string)
        file_handle.close()

    def parse_config(self, filename):
        COMMENT_CHAR = '#'
        OPTION_CHAR =  '='
        options = {}
        f = open(filename)
        for line in f:
            if COMMENT_CHAR in line:
                line, comment = line.split(COMMENT_CHAR, 1)
            if OPTION_CHAR in line:
                option, value = line.split(OPTION_CHAR, 1)
                option = option.strip()
                value = value.strip()
                options[option] = value
        f.close()
        return options

    def on_toggle(self, button, applet, data=None):
        rect = button.get_allocation()
        main_window = self.button.get_toplevel()
        [val, win_x, win_y] = main_window.get_window().get_origin()
        orientation = self.applet.get_orient()
        # This needs fixing
        if orientation == 0:
           # BOTTOM
           cal_x = win_x 
           cal_y = win_y
        elif orientation == 1:
           # TOP
           cal_x = win_x + rect.x
           cal_y = win_y + rect.y + rect.height
           print(cal_y)
        elif orientation == 2:
           # RIGHT
           cal_x = win_x
           cal_y = win_y
        elif orientation == 3:
           # LEFT
           cal_x = win_x + rect.x
           cal_y = win_y + rect.y
        else:
           cal_x = win_x
           cal_y = win_y
        options = self.parse_config(CONFIG_PATH)
        [x, y] = self.apply_screen_coord_correction(cal_x, cal_y, options, button)
        #set coordinates
        pattern_x = re.compile("menu_margin_x\s+\=\s+(\d+)")
        self.replace(CONFIG_PATH,pattern_x,'menu_margin_x = ' + str(x))
        pattern_y = re.compile("menu_margin_y\s+\=\s+(\d+)")
        self.replace(CONFIG_PATH,pattern_y,'menu_margin_y = ' + str(y))
        self.execute('jgmenu')

    def apply_screen_coord_correction(self, x, y, options, relative_widget):
        corrected_y = y
        corrected_x = x

        screen_w = Gdk.Screen.width()
        screen_h = Gdk.Screen.height()
        delta_x = screen_w - (x + int(options["menu_width"]))
        delta_y = screen_h - (y + int(options["menu_height_max"]))
        corrected_y = delta_y
        print(delta_x,delta_y)
        if delta_x < 0:
            corrected_x += delta_x
        if corrected_x < 0:
            corrected_x = 0
        if delta_y < 0:
            corrected_y = y - int(options["menu_height_max"]) - relative_widget.get_allocation().height
        if corrected_y < 0:
            corrected_y = 0
        print(corrected_x,corrected_y)
        return [corrected_x, corrected_y]


def applet_factory(applet, iid, data):
    if iid != "JgMenuApplet":
       return False
    JgMenu(applet)
    return True

if __name__ == "__main__":
    MatePanelApplet.Applet.factory_main("JgMenuAppletFactory", True,
                                    MatePanelApplet.Applet.__gtype__,
                                    applet_factory, None)