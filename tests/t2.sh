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
echo -e 'foo,foo'
echo -e 'bar,bar'
echo -e 'baz,baz'
echo -e 'qux,qux'
echo -e 'foobar,foobar'
) | ../jgmenu --config-file=./jgmenurc
