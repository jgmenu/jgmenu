#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# License: GPLv2
#
# Copyright (C) 2016 Ovidiu M <mrovi9000@gmail.com>
#

import os
import sys

# Reference: http://standards.freedesktop.org/desktop-entry-spec/desktop-entry-spec-1.1.html
# Loads a Desktop Entry file into a dictionary key -> value
# A special key "_path" stores the path of the file
def readDesktopEntry(path):
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
  return entry


# Reference: http://refspecs.linuxfoundation.org/FHS_3.0/fhs/index.html
# Reference: http://standards.freedesktop.org/basedir-spec/basedir-spec-0.8.html
def getSettingLocations():
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
def loadCategories():
  categories = {}
  for d in getSettingLocations():
    d = d + "/desktop-directories/"
    for (dirpath, dirnames, filenames) in os.walk(d):
      for filename in filenames:
        entry = readDesktopEntry(os.path.join(dirpath, filename))
        if "Name" in entry and "Type" in entry and entry["Type"] == "Directory":
          categories[entry["Name"]] = entry
  if "Other" not in categories:
    categories["Other"] = {"Name": "Other", "Icon": "applications-other", "_path": "auto-generated"}
  return categories


# Returns a list of alternate, more generic categories for a given category name
# Reference: http://standards.freedesktop.org/menu-spec/latest/apas02.html
def normalizeCategory(c):
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
def suggestCategories(appName):
  appName = appName.lower()
  if "network" in appName or "google" in appName or "cisco" in appName or "mail" in appName or "youtube" in appName:
    return normalizeCategory("Network")
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
# The categories are in the same form as returned by loadCategories()
def loadApplications():
  categories = loadCategories()
  menu = {}
  for d in getSettingLocations():
    d = d + "/applications/"
    for (dirpath, dirnames, filenames) in os.walk(d):
      for filename in filenames:
        entry = readDesktopEntry(os.path.join(dirpath, filename))
        cmd = get_cmd(entry)
        if "Type" in entry and entry["Type"] == "Application" and cmd:
          entry["cmd"] = cmd
          if "Name" not in entry:
            entry["Name"] = filename.replace(".desktop", "")
            entry["Name"] = entry["Name"][:1].upper() + entry["Name"][1:]
          appCategories = []
          if "Categories" in entry:
            appCategories = [s.strip() for s in entry["Categories"].split(";") if s.strip()]
          appCategories.append("")
          added = False
          for c in appCategories:
            options = []
            if c:
              if c not in categories:
                options = normalizeCategory(c)
              else:
                options = [c]
            else:
              options = suggestCategories(entry["Name"])
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


# Creates and shows the menu
def createMenu():
  print("jgmenu,^tag(pmenu)")
  tree, categories = loadApplications()
  for c in sorted(tree):
    category = categories[c]
    icon = category["Icon"] if "Icon" in category else "folder"
    print(category["Name"] + ",^checkout(" + category["Name"] + ")," + icon)
  for c in sorted(tree):
    category = categories[c]
    print("")
    print("submenu,^tag(" + category["Name"] + ")")
    print("go back,^checkout(pmenu),folder")
    for app in tree[c]:
      icon = app["Icon"] if "Icon" in app else "application-x-executable"
      print(app["Name"] + "," + app["cmd"] + "," + icon)


if __name__ == '__main__':
  createMenu()
