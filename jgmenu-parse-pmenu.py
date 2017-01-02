#!/usr/bin/env python3
#
# License: GPLv2
#
# Copyright (C) 2016 Ovidiu M <mrovi9000@gmail.com>
# Modified by Johan Malm <jgm323@gmail.com>
#

import argparse
import locale
import os
import sys

# Computes a list of the current locale names, in most specific to least specific order.
# The empty string is always the last element.
def get_current_locale_names():
  # E.g. "en_US"
  lang = locale.getlocale()[0]
  # ["en", "US"]
  lang_parts = lang.split("_")
  locale_names = [""]
  for i in range(len(lang_parts)):
    locale_names.append("_".join(lang_parts[:i+1]))
  # ["en_US", "en", ""]
  locale_names = list(reversed(locale_names))
  return locale_names

def internationalized(entry):
  # Now entry is a dict of tag -> value, with tag like "Name", "Name[en]", "Name[en_US]"
  # We modify it so that the value of "Name" is replaced with the value of the most specific tag
  # that matches the current locale.
  # We create a two-level tree:
  # tag => name, suffix => value
  # Where name is the tag name (e.g. "Name") and suffix is the tag's locale (e.g. "en_US" or "").
  tree = {}
  for tag in entry:
    if "[" in tag:
      name, suffix = tag.replace("]", "").split("[", 1)
    else:
      name = tag
      suffix = ""
    if name not in tree:
      tree[name] = {}
    tree[name][suffix] = entry[tag]
  # Collapse into a tag => value dict, using the most specific matching suffix.
  locale_names = get_current_locale_names()
  entry = {}
  for name in tree:
    for suffix in locale_names:
      if suffix in tree[name]:
        entry[name] = tree[name][suffix]
        break
  return entry

# Reference: http://standards.freedesktop.org/desktop-entry-spec/desktop-entry-spec-1.1.html
# Loads a Desktop Entry file into a dictionary key -> value
# A special key "_path" stores the path of the file
def read_desktop_entry(path):
  entry = {}
  with open(path, "r") as f:
    lines = f.read().split("\n")
    inside = False
    for line in lines:
      if line.startswith("["):
        inside = line == "[Desktop Entry]"
        continue
      if inside:
        if "=" in line:
          k, v = line.split("=", 1)
          entry[k] = v
  entry["_path"] = path
  return internationalized(entry)

# Reference: http://refspecs.linuxfoundation.org/FHS_3.0/fhs/index.html
# Reference: http://standards.freedesktop.org/basedir-spec/basedir-spec-0.8.html
def get_setting_locations():
  locations = ["/usr/share", "/usr/local/share", os.path.expanduser("~/.local/share")]
  if "XDG_DATA_DIRS" in os.environ:
    dirs = os.environ["XDG_DATA_DIRS"]
    if dirs:
      dirs = dirs.split(":")
      for d in dirs:
        while d.endswith("/"):
          d = d[:-1]
        if d not in locations:
          locations.append(d)
  return locations


# Loads all Desktop Entry files with type "Directory" into a dictionary name -> entry
# Reference: http://standards.freedesktop.org/desktop-entry-spec/desktop-entry-spec-1.1.html
def load_categories():
  categories = {}
  for d in get_setting_locations():
    d = d + "/desktop-directories/"
    for (dirpath, dirnames, filenames) in os.walk(d):
      for filename in filenames:
        entry = read_desktop_entry(os.path.join(dirpath, filename))
        if "Name" in entry and "Type" in entry and entry["Type"] == "Directory":
          categories[entry["Name"]] = entry
  if "Other" not in categories:
    categories["Other"] = {"Name": "Other", "Icon": "applications-other", "_path": "auto-generated"}
  return categories


# Returns a list of alternate, more generic categories for a given category name
# Reference: http://standards.freedesktop.org/menu-spec/latest/apas02.html
def normalize_category(c):
  if c in ["Network", "Email", "Dialup", "InstantMessaging", "Chat", "IRCClient", "Feed", "FileTransfer", "HamRadio",
           "News", "P2P", "RemoteAccess", "Telephony", "TelephonyTools", "VideoConference", "WebBrowser"]:
    return [c, "Internet"]
  if c in ["Game", "ActionGame", "AdventureGame", "ArcadeGame", "BoardGame", "BlocksGame", "CardGame", "KidsGame",
           "LogicGame", "RolePlaying", "Shooter", "Simulation", "SportsGame", "StrategyGame", "Sports", "Amusement"]:
    return [c, "Games"]
  if c in ["Utility", "TextTool", "TextTools", "Archiving", "Compression", "FileTool", "FileTools", "Calculator",
           "Clock", "TextEditor"]:
    return [c, "Accessories", "Utilities"]
  if c in ["Building", "Debugger", "IDE", "GUIDesigner", "Profiling", "RevisionControl", "Translation",
           "WebDevelopment", "ParallelComputing", "Database", "ArtificialIntelligence"]:
    return [c, "Development"]
  if c in ["Calendar", "ContactManagement", "Dictionary", "Chart", "Finance", "FlowChart", "PDA", "ProjectManagement",
           "Presentation", "Spreadsheet", "WordProcessor", "Publishing", "Viewer"]:
    return [c, "Office"]
  if c in ["2DGraphics", "VectorGraphics", "RasterGraphics", "3DGraphics", "Scanning", "OCR", "Photography",
           "ImageProcessing"]:
    return [c, "Graphics"]
  if c in ["AudioVideo", "Player", "Audio", "Video", "Midi", "Mixer", "Sequencer", "Tuner", "TV", "AudioVideoEditing",
           "Recorder", "DiscBurning", "Adult"]:
    return [c, "Multimedia", "Sound & Video"]
  if c in ["Construction", "Astronomy", "Biology", "Chemistry", "ComputerScience", "DataVisualization", "Economy",
           "Electricity", "Geography", "Geology", "Geoscience", "Maps", "Math", "NumericalAnalysis", "MedicalSoftware",
           "Physics", "Robotics", "Electronics", "Engineering"]:
    return [c, "Science"]
  if c in ["Literature", "Art", "Languages", "History", "Humanities", "Spirituality"]:
    return [c, "Education", "Science"]
  if c in ["Emulator", "FileManager", "TerminalEmulator", "Filesystem", "Monitor"]:
    return [c, "System"]
  if c in ["Settings", "Security", "Accessibility"]:
    return [c, "Preferences"]
  return [c]


# Returns a list of suggested categories for a given application name (useful if no category is specified or found)
def suggest_categories(app_name):
  app_name = app_name.lower()
  if "network" in app_name or "google" in app_name or "cisco" in app_name or "mail" in app_name or "youtube" in app_name:
    return normalize_category("Network")
  return ["Other"]


def get_cmd(app):
  if "Exec" in app:
    cmd = ""
    if "Path" in app and app["Path"]:
      cmd += "cd " + app["Path"] + ";"
    exe = ""
    percent = False
    for c in app["Exec"]:
      if not percent:
        if c == "%":
          percent = True
        else:
          exe += c
      else:
        if c == "%":
          exe += c
          percent = False
        else:
          pass
    cmd += exe
    return cmd
  return None


# Returns the start menu hierarchy and the main application categories
# The menu is a dictionary category-name -> list of application entries
# The categories are in the same form as returned by load_categories()
def load_applications():
  categories = load_categories()
  menu = {}
  for d in get_setting_locations():
    d = d + "/applications/"
    for (dirpath, dirnames, filenames) in os.walk(d):
      for filename in filenames:
        entry = read_desktop_entry(os.path.join(dirpath, filename))
        cmd = get_cmd(entry)
        if "Type" in entry and entry["Type"] == "Application" and cmd:
          entry["cmd"] = cmd
          if "Name" not in entry:
            entry["Name"] = filename.replace(".desktop", "")
            entry["Name"] = entry["Name"][:1].upper() + entry["Name"][1:]
          app_categories = []
          if "Categories" in entry:
            app_categories = [s.strip() for s in entry["Categories"].split(";") if s.strip()]
          app_categories.append("")
          added = False
          for c in app_categories:
            options = []
            if c:
              if c not in categories:
                options = normalize_category(c)
              else:
                options = [c]
            else:
              options = suggest_categories(entry["Name"])
            for o in options:
              if o in categories:
                if o not in menu:
                  menu[o] = []
                menu[o].append(entry)
                added = True
                break
            if added:
              break
  for c in menu:
    menu[c] = sorted(menu[c], key=lambda item: item["Name"])
  return menu, categories

def cat_file(path):
  if path and os.path.isfile(path):
    with open(path, encoding='utf-8') as data_file:
      print(data_file.read())

# Creates and shows the menu
def create_menu(arg_append_file, arg_prepend_file):
  print("jgmenu,^tag(pmenu)")
  cat_file(arg_prepend_file)
  tree, categories = load_applications()
  for c in sorted(tree):
    category = categories[c]
    icon = category["Icon"] if "Icon" in category else "folder"
    print(category["Name"] + ",^checkout(" + category["Name"] + ")," + icon)
  cat_file(arg_append_file)
  for c in sorted(tree):
    category = categories[c]
    print("")
    print("submenu,^tag(" + category["Name"] + ")")
    print("go back,^checkout(pmenu),folder")
    for app in tree[c]:
      icon = app["Icon"] if "Icon" in app else "application-x-executable"
      print(app["Name"] + "," + app["cmd"] + "," + icon)

def main():
  parser = argparse.ArgumentParser(prog="jgmenu_run parse-pmenu")
  parser.add_argument("--append-file", help="Path to menu file to append to the root menu", metavar="FILE")
  parser.add_argument("--prepend-file", help="Path to menu file to prepend to the root menu", metavar="FILE")
  parser.add_argument("--locale", help="Use a custom locale (e.g. 'en_US.UTF-8'; available locales can be shown " +
                                       "by running 'locale -a')", default="")
  args = parser.parse_args()
  try:
    locale.setlocale(locale.LC_ALL, args.locale)
  except:
    print("Warning: setting locale failed! Use an available locale as listed by 'locale -a'.", file=sys.stderr)
  create_menu(args.append_file, args.prepend_file)

if __name__ == '__main__':
  main()
