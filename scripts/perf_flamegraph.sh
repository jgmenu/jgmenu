#!/bin/sh

# Based on https://gitlab.com/o9000/tint2/wikis/profile
# and https://github.com/brendangregg/FlameGraph

fgdir="$HOME/src/FlameGraph"
cmd="jgmenu_run csv"
svgfile=log.svg

sudo sh -c 'echo 0 > /proc/sys/kernel/kptr_restrict'
cd ${fgdir}
perf record -F 999 -g --call-graph dwarf ${cmd}
perf script | ./stackcollapse-perf.pl| ./flamegraph.pl > ${svgfile}
xdg-open ${svgfile}
sudo sh -c 'echo 1 > /proc/sys/kernel/kptr_restrict'
