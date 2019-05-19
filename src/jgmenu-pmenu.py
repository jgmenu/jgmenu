#!/usr/bin/env python3
#
# License: GPLv2
#
# Copyright (C) 2016-2017 Ovidiu M <mrovi9000@gmail.com>
# Copyright (C) 2017-2019 Johan Malm <jgm323@gmail.com>
#

import argparse
import gettext
import locale
import os
import sys

strings = {
  "Back": "Back",
  "Back[am]": "ወደ ኋላ",
  "Back[ar]": "إلى الخلف",
  "Back[ast]": "Atrás",
  "Back[be]": "Назад",
  "Back[bg]": "Назад",
  "Back[bn]": "পূর্ববর্তী",
  "Back[ca]": "Endarrere",
  "Back[cs]": "Zpět",
  "Back[da]": "Tilbage",
  "Back[de]": "Zurück",
  "Back[el]": "Πίσω",
  "Back[eo]": "Malantaŭen",
  "Back[es]": "Atrás",
  "Back[et]": "Tagasi",
  "Back[eu]": "Atzera",
  "Back[fa_IR]": "بازگشت",
  "Back[fi]": "Edellinen",
  "Back[fr]": "Précédent",
  "Back[gl]": "Recuar",
  "Back[he]": "אחורה",
  "Back[hr]": "Natrag",
  "Back[hu]": "Vissza",
  "Back[id]": "Kembali",
  "Back[is]": "Til baka",
  "Back[it]": "Indietro",
  "Back[ja]": "戻る",
  "Back[kk]": "Артқа",
  "Back[ko]": "뒤로",
  "Back[lt]": "Atgal",
  "Back[lv]": "Atpakaļ",
  "Back[ms]": "Undur",
  "Back[nb]": "Tilbake",
  "Back[nl]": "Terug",
  "Back[nn]": "Tilbake",
  "Back[oc]": "Precedent",
  "Back[pa]": "ਪਿੱਛੇ",
  "Back[pl]": "Wstecz",
  "Back[pt_BR]": "Voltar",
  "Back[pt]": "Recuar",
  "Back[ro]": "Înapoi",
  "Back[ru]": "Назад",
  "Back[sk]": "Späť",
  "Back[sq]": "Prapa",
  "Back[sr]": "Назад",
  "Back[sv]": "Bakåt",
  "Back[te]": "వెనుకకు",
  "Back[th]": "ถอยกลับ",
  "Back[tr]": "Geri",
  "Back[ug]": "ئارقىسىغا",
  "Back[uk]": "Назад",
  "Back[ur_PK]": "پیچھے",
  "Back[ur]": "پیچھے",
  "Back[vi]": "Quay lui",
  "Back[zh_CN]": "后退",
  "Back[zh_HK]": "往前",
  "Back[zh_TW]": "往前",
  "Other": "Other",
  "Other[af]": "Ander",
  "Other[ar]": "أخرى",
  "Other[as]": "অন্যান্য",
  "Other[ast]": "Otres",
  "Other[be]": "Іншыя",
  "Other[be@latin]": "Inšyja",
  "Other[bg]": "Други",
  "Other[bn]": "অন্যান্য",
  "Other[bn_IN]": "অন্যান্য",
  "Other[br]": "All",
  "Other[ca]": "Altres",
  "Other[cs]": "Ostatní",
  "Other[cy]": "Eraill",
  "Other[da]": "Andre",
  "Other[de]": "Sonstige",
  "Other[dz]": "གཞན།",
  "Other[el]": "Άλλα",
  "Other[en_CA]": "Other",
  "Other[en_GB]": "Other",
  "Other[eo]": "Alia",
  "Other[es]": "Otras",
  "Other[es_VE]": "Otras",
  "Other[et]": "Muu",
  "Other[eu]": "Bestelakoak",
  "Other[fa]": "غیره",
  "Other[fi]": "Muut",
  "Other[fr]": "Autre",
  "Other[frp]": "Autres",
  "Other[fur]": "Altri",
  "Other[ga]": "Eile",
  "Other[gl]": "Outros",
  "Other[gn]": "Amboae",
  "Other[gu]": "અન્ય",
  "Other[he]": "אחר",
  "Other[hi]": "अन्य",
  "Other[hr]": "Ostalo",
  "Other[hu]": "Egyéb",
  "Other[hy]": "Այլ",
  "Other[id]": "Lainnya",
  "Other[io]": "Altra",
  "Other[is]": "Aðrir",
  "Other[it]": "Altro",
  "Other[ja]": "その他",
  "Other[ka]": "სხვა",
  "Other[kk]": "Басқалар",
  "Other[ko]": "기타",
  "Other[ku]": "Yên din",
  "Other[ky]": "Башкалар",
  "Other[lt]": "Kitos",
  "Other[lv]": "Citas programmas",
  "Other[mai]": "आन",
  "Other[mg]": "Hafa",
  "Other[mk]": "Други",
  "Other[ml]": "മറ്റുളളവ",
  "Other[mn]": "Бусад",
  "Other[mr]": "अन्य",
  "Other[ms]": "Lain-lain",
  "Other[nb]": "Annet",
  "Other[ne]": "अन्य",
  "Other[nl]": "Overig",
  "Other[nn]": "Andre",
  "Other[oc]": "Autre",
  "Other[or]": "ଅନ୍ଯାନ୍ଯ",
  "Other[pa]": "ਹੋਰ",
  "Other[pl]": "Inne",
  "Other[ps]": "نور",
  "Other[pt]": "Outras",
  "Other[pt_BR]": "Outros",
  "Other[ro]": "Altele",
  "Other[ru]": "Прочие",
  "Other[rw]": "Ikindi",
  "Other[si]": "වෙනත්",
  "Other[sk]": "Ostatné",
  "Other[sl]": "Drugo",
  "Other[sq]": "Tjetër",
  "Other[sr]": "Остало",
  "Other[sr@latin]": "Ostalo",
  "Other[sv]": "Övriga",
  "Other[ta]": "மற்றவை",
  "Other[te]": "ఇతర",
  "Other[th]": "อื่นๆ",
  "Other[tr]": "Diğer",
  "Other[ug]": "باشقىلار",
  "Other[uk]": "Інші",
  "Other[ur]": "دیگر",
  "Other[ur_PK]": "دیگر",
  "Other[uz@cyrillic]": "Бошқа",
  "Other[vi]": "Khác",
  "Other[xh]": "Ezinye",
  "Other[zh_CN]": "其它",
  "Other[zh_HK]": "其它",
  "Other[zh_TW]": "其它"
}

# Computes a list of the current locale names, in most specific to least specific order.
# The empty string is always the last element.
def get_current_locale_names():
  # E.g. "en_US"
  lang = locale.getlocale()[0] or "en_US"
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
  # Keep the old names in keys with the value "_pmenu_raw_" + tag
  locale_names = get_current_locale_names()
  entry = {}
  for name in tree:
    if "" in tree[name]:
      entry["_pmenu_raw_" + name] = tree[name][""]
    for suffix in locale_names:
      if suffix in tree[name]:
        entry[name] = tree[name][suffix]
        if not suffix and entry[name]:
          entry[name] = _(entry[name])
        break
  return entry

# Reference: http://standards.freedesktop.org/desktop-entry-spec/desktop-entry-spec-1.1.html
# Loads a Desktop Entry file into a dictionary key -> value
# A special key "_path" stores the path of the file
def read_desktop_entry(path):
  entry = {}
  try:
    with open(path, "r", encoding="utf-8") as f:
      lines = f.read().split("\n")
      inside = False
      for line in lines:
        if line.startswith("["):
          inside = line == "[Desktop Entry]"
          continue
        if inside:
          if "=" in line:
            k, v = line.split("=", 1)
            if k == "NoDisplay" and v == "true":
                return {}
            entry[k] = v
  except UnicodeDecodeError:
    print("warn: ignoring '{}' as it is unicode (utf-8 needed)".format(path), file=sys.stderr)
  except:
    print("warn: error reading '{}'".format(path), file=sys.stderr)
  entry["_path"] = path
  return internationalized(entry)

# Reference: http://refspecs.linuxfoundation.org/FHS_3.0/fhs/index.html
# Reference: http://standards.freedesktop.org/basedir-spec/basedir-spec-0.8.html
def get_setting_locations():
  locations = []
  locations.append(os.path.expanduser("~/.local/share"))
  if "XDG_DATA_DIRS" in os.environ:
    dirs = os.environ["XDG_DATA_DIRS"]
    if dirs:
      dirs = dirs.split(":")
      for d in dirs:
        while d.endswith("/"):
          d = d[:-1]
        if d not in locations:
          locations.append(d)
  locations.append("/usr/share")
  locations.append("/usr/local/share")
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
        if "_pmenu_raw_Name" in entry and "Type" in entry and entry["Type"] == "Directory":
          categories[normalize_category(entry["_pmenu_raw_Name"])[-1]] = entry
  if "Other" not in categories:
    categories["Other"] = {"Name": strings["Other"], "Icon": "applications-other", "_path": "auto-generated"}
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
    if "Terminal" in app and app["Terminal"] == "true":
      cmd = "^term(" + cmd.strip() + ")"
    return cmd
  return None


# Returns the start menu hierarchy and the main application categories
# The menu is a dictionary category-name -> list of application entries
# The categories are in the same form as returned by load_categories()
def load_applications(hide_dirs):
  if hide_dirs:
    menu = []
  else:
    menu = {}
    categories = load_categories()
  all_filenames = []  # keep track of filenames to respect user overrides
  for d in get_setting_locations():
    d = d + "/applications/"
    for (dirpath, dirnames, filenames) in os.walk(d):
      for filename in filenames:
        if filename in all_filenames:
          continue
        all_filenames.append(filename)
        entry = read_desktop_entry(os.path.join(dirpath, filename))
        cmd = get_cmd(entry)
        if "Type" in entry and entry["Type"] == "Application" and cmd:
          entry["cmd"] = cmd
          if "Name" not in entry:
            entry["Name"] = filename.replace(".desktop", "")
            entry["Name"] = entry["Name"][:1].upper() + entry["Name"][1:]
          if hide_dirs:
            menu.append(entry)
            continue
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
              try:
                options = suggest_categories(entry["_pmenu_raw_Name"])
              except:
                print("warn: no 'Name' key in file '{}'".format(entry["_path"]), file=sys.stderr)
            for o in options:
              if o in categories:
                if o not in menu:
                  menu[o] = []
                menu[o].append(entry)
                added = True
                break
            if added:
              break
  if not hide_dirs:
    for c in menu:
      menu[c] = sorted(menu[c], key=lambda item: item["Name"])
  if hide_dirs:
    return menu
  else:
    return menu, categories

def cat_file(path):
  if path and os.path.isfile(path):
    with open(path, encoding='utf-8') as data_file:
      print(data_file.read())

def escape_markup(s):
    return s.replace("&", "&amp;")

# Creates the menu with directories
def create_menu(arg_append_file, arg_prepend_file):
  single_window = os.getenv("JGMENU_SINGLE_WINDOW")

#  print("jgmenu,^tag(pmenu)")
  cat_file(arg_prepend_file)
  tree, categories = load_applications(False)

  # If no menu-package exists, just dump the apps in the menu-root
  if len(categories) <= 1:
    for app in tree['Other']:
      icon = app["Icon"] if "Icon" in app else ""
      print(escape_markup(app["Name"]) + "," + app["cmd"] + "," + icon)
    cat_file(arg_append_file)
    print("warn: no menu package found; displaying apps without directories", file=sys.stderr)
    return

  for c in sorted(tree):
    category = categories[c]
    icon = category["Icon"] if "Icon" in category else "folder"
    try:
      if single_window:
          print(escape_markup(category["Name"]) + ",^root(" + category["Name"] + ")," + icon)
      else:
          print(escape_markup(category["Name"]) + ",^checkout(" + category["Name"] + ")," + icon)
    except:
      print("warn: category problem", file=sys.stderr)

  cat_file(arg_append_file)
  for c in sorted(tree):
    category = categories[c]
    print("")
    print("#", category["_path"])
    print("submenu,^tag(" + category["Name"] + ")")
    print(strings["Back"] + ",^back(),go-previous")
    for app in tree[c]:
      icon = app["Icon"] if "Icon" in app else "application-x-executable"
      print("#", app["_path"])
      print(escape_markup(app["Name"]) + "," + app["cmd"] + "," + icon)

# Creates menu without directories
def create_menu_no_dirs(arg_append_file, arg_prepend_file):
  cat_file(arg_prepend_file)
  menu = load_applications(True)
  for app in sorted(menu, key=lambda k: k['Name']):
    icon = app["Icon"] if "Icon" in app else "application-x-executable"
    print("#", app["_path"])
    print(escape_markup(app["Name"]) + "," + app["cmd"] + "," + icon)
  cat_file(arg_append_file)

def setup_gettext():
  global _
  try:
    gettext.translation("gnome-menus-3.0", languages=get_current_locale_names()).install()
  except:
    try:
      gettext.translation("gnome-menus", languages=get_current_locale_names()).install()
    except:
      pass
  try:
    _("")
  except:
    def _(s):
      return s

def main():
  global strings
  parser = argparse.ArgumentParser(prog="jgmenu_run pmenu")
  parser.add_argument("--append-file", help="Path to menu file to append to the root menu", metavar="FILE")
  parser.add_argument("--prepend-file", help="Path to menu file to prepend to the root menu", metavar="FILE")
  parser.add_argument("--locale", help="Use a custom locale (e.g. 'en_US.UTF-8'; available locales can be shown " +
                                       "by running 'locale -a')", default="")
  args = parser.parse_args()
  if args.append_file:
    append_file = args.append_file
  else:
    append_file = os.getenv("HOME") + "/.config/jgmenu/append.csv"
  if args.prepend_file:
    prepend_file = args.prepend_file
  else:
    prepend_file = os.getenv("HOME") + "/.config/jgmenu/prepend.csv"
  try:
    locale.setlocale(locale.LC_ALL, args.locale)
  except:
    print("Warning: setting locale failed! Use an available locale as listed by 'locale -a'.", file=sys.stderr)
  setup_gettext()
  strings = internationalized(strings)
  if os.getenv("JGMENU_NO_DIRS"):
    create_menu_no_dirs(append_file, prepend_file)
  else:
    create_menu(append_file, prepend_file)

if __name__ == '__main__':
  main()
