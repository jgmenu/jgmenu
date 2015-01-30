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
echo -e 'foo,echo foo/'
echo -e 'bar,echo foo/'
echo -e 'baz,echo foo/'
echo -e 'quax,echo foo/'
echo -e 'foobar,echo foo/'
echo -e 'foobarbaz,echo foo/'
) | ../jgmenu
