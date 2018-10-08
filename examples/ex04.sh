#!/bin/sh

(
printf "%b\n" 'theme1 i1.svg,:,i1'
printf "%b\n" 'theme2 i2.svg,:,i2'
printf "%b\n" '^sep(icon alignment)'
printf "%b\n" 'icon22x22 (png),:,icon22x22png'
printf "%b\n" 'icon22x22 (xpm),:,icon22x22xpm'
printf "%b\n" 'icon22x11 (png),:,icon22x11png'
printf "%b\n" 'icon22x11 (xpm),:,icon22x11xpm'
printf "%b\n" 'icon64x32 (svg),:,icon64x32svg'
printf "%b\n" 'icon32x64 (svg),:,icon32x64svg'
) | XDG_DATA_HOME=$(pwd)/ex04 ../jgmenu --simple --config-file=./ex04/jgmenurc
