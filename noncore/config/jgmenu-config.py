#!/usr/bin/env python3
#
# Copyright (C) Johan Malm 2018
#

""" create/modify/query jgmenu config files """

import argparse
import sys
import os
import subprocess
import re

DEFAULT_CONFIG_FILE = "~/.config/jgmenu/jgmenurc"

def die(msg):
    """ exit with message """
    sys.stderr.write("fatal: {}\n".format(msg))
    sys.exit(1)

def read_pipe(cmd):
    """ return stdout from subprocess """
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE, shell=True)
    (out, err) = proc.communicate()
    if proc.returncode != 0:
        die(err)
    return out

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

def missing_keys(template_lines, conf_lines):
    """ compare {config,template} files and return missing keys """
    template_keys = get_keys(template_lines)
    conf_keys = get_keys(conf_lines)
    return [item for item in template_keys if item not in set(conf_keys)]

def nr_comment_lines_at_start(conf_lines):
    """ calculate number of non-key/value lines at start of file """
    i = 0
    for line in conf_lines:
        if "=" in line:
            break
        else:
            i = i + 1
    return i

def key_in_line(key, line):
    """ acurately determie if key exists in key/value line """
    return bool(re.search(r'[#\s]*' + key + r'\s*=.*', line))

def key_in_conf_lines(key, conf_lines):
    """ check if keys exists in conf file """
    for line in conf_lines:
        if key_in_line(key, line):
            return True
    return False

def insert_pos(template_lines, conf_lines, key):
    """ find position to insert new line in accordance with template """
    found_key = False
    line_to_add = ''
    for t_line in reversed(template_lines):
        if "=" not in t_line:
            continue
        if found_key:
            prev_key = get_key(t_line)
            # Consider the case where several conf_file items are missing
            if not key_in_conf_lines(prev_key, conf_lines):
                continue
            for c_index, c_line in enumerate(conf_lines):
                if key_in_line(prev_key, c_line):
                    return int(c_index) + 1, line_to_add
        if get_key(t_line) == key:
            line_to_add = t_line
            found_key = True
    # we'll onle get here if it's the first key/value line
    return nr_comment_lines_at_start(conf_lines), line_to_add

def resolve(filename):
    """ expand ~ and $foo """
    if filename is None or filename == '':
        die("filename must be specified")
    filename = os.path.expanduser(filename)
    filename = os.path.expandvars(filename)
    if not os.path.exists(filename):
        die("file '{}' does not exist".format(filename))
    return filename

def get_template_filename():
    """ get filename for template jgmenurc config file """
    s = read_pipe("jgmenu_run --exec-path").strip().decode("utf-8")
    s += "/jgmenurc"
    if not os.path.exists(s):
        die("$libexecdir/jgmenurc does not exist")
    return s

def adjust_line_spacing(conf_lines, template_lines):
    """ adjust line spacing in accordance with template """
    keys_with_space_after = []
    space = False
    for line in reversed(template_lines):
        if line == '':
            space = True
        elif "=" in line and space:
            keys_with_space_after.append(get_key(line))
            space = False
    new_list = []
    first_key_line = True
    for line in conf_lines:
        if line == '':
            continue
        if '=' in line and first_key_line:
            new_list.append('')
            first_key_line = False
        new_list.append(line)
        if '=' in line and get_key(line) in keys_with_space_after:
            new_list.append('')
    return new_list

def amend(conf_filename):
    """ amend config-file based on template """
    conf_filename = resolve(conf_filename)
    template_filename = get_template_filename()
    with open(conf_filename, "r+") as f:
        conf_lines = f.read().split("\n")
    with open(template_filename, "r+") as f:
        template_lines = f.read().split("\n")

    keys = missing_keys(template_lines, conf_lines)
    if keys:
        print("info: add keys {}".format(keys))
    # using insert_pos(), we may not get the correct line spacing
    for key in keys:
        i_pos, line = insert_pos(template_lines, conf_lines, key)
        conf_lines.insert(i_pos, line)

    conf_lines = adjust_line_spacing(conf_lines, template_lines)

    # delete empty lines at end of file
    for line in reversed(conf_lines):
        if line == '':
            conf_lines.pop()
            break
    # write config file
    with open(conf_filename, "w") as f:
        for line in conf_lines:
            f.write("{}\n".format(line))

def main():
    """ script main program """
    parser = argparse.ArgumentParser(prog="jgmenu_run config")
    subparsers = parser.add_subparsers(dest="command", help="commands")
    subparsers.required = True
    amend_parser = subparsers.add_parser("amend", help="amend config-file \
                                         with missing items")
    amend_parser.add_argument("--file", metavar="FILE", help="specify config \
                              file", default=DEFAULT_CONFIG_FILE)
    args = parser.parse_args()
    if args.command == "amend":
        amend(args.file)
    else:
        parser.print_help(sys.stderr)

if __name__ == '__main__':
    main()
