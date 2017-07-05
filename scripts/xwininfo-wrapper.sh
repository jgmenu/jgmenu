#!/bin/sh

type xwininfo 2>/dev/null || { echo "fatal: xwininfo needed"; exit 1; }

(sleep 2; xwininfo -name jgmenu)& jgmenu_run
