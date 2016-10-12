#!/bin/sh

(
printf "%b\n" 'theme1 i1.svg,:,i1'
printf "%b\n" 'theme2 i2.svg,:,i2'
printf "%b\n" 'symlink to Numix icon,:,emblem-shared-symbolic'
) | XDG_DATA_HOME=$(pwd)/ex04 ../jgmenu --config-file=ex04-jgmenurc
