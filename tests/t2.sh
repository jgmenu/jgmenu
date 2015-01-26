#!/bin/bash

(
echo -e 'ROOT MENU,^tag(root)'
echo -e 'terminal,xterm'
echo -e 'browser,firefox'
echo -e 'file manager,pcmanfm'
echo -e 'config,^checkout(config)'
echo -e 'CONFIG,^tag(config)'
echo -e '..,^checkout(root)'
echo -e 'background,nitrogen ~/bg/'
) | ../jgmenu
