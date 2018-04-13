#!/bin/sh

# --variable-rgx allows variable length of 1 (for i, j, x, etc)

pylint \
	--variable-rgx='[a-z_][a-z0-9_]{0,30}$' \
	--module-rgx='jgmenu-config' \
	./noncore/config/jgmenu-config.py
