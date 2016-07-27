#!/bin/bash

(
echo -e 'theme1 i1.svg,:,i1'
echo -e 'theme2 i2.svg,:,i2'
echo -e 'symlink to Numix icon,:,emblem-shared-symbolic'
) | XDG_DATA_HOME=$(pwd)/ex04 ../jgmenu --config-file=ex04-jgmenurc
