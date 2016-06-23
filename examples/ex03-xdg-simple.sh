#!/bin/bash

(
echo -e "XDG MENU,^tag(root)"
echo -e "Full XDG menu,^sub(../jgmenu_xdg --parent=./ex03-xdg-simple.sh | ../jgmenu --config-file=ex03-jgmenurc --icon-size=22),folder-documents"
echo -e "Audio,^sub(../jgmenu_xdg --parent=./ex03-xdg-simple.sh Audio | ../jgmenu --config-file=ex03-jgmenurc --icon-size=22),applications-multimedia"
echo -e "Video,^sub(../jgmenu_xdg --parent=./ex03-xdg-simple.sh Video | ../jgmenu --config-file=ex03-jgmenurc --icon-size=22),applications-multimedia"
echo -e "Development,^sub(../jgmenu_xdg --parent=./ex03-xdg-simple.sh Development | ../jgmenu --config-file=ex03-jgmenurc --icon-size=22),applications-development"
echo -e "Education,^sub(../jgmenu_xdg --parent=./ex03-xdg-simple.sh Education | ../jgmenu --config-file=ex03-jgmenurc --icon-size=22),applications-science"
echo -e "Game,^sub(../jgmenu_xdg --parent=./ex03-xdg-simple.sh Game | ../jgmenu --config-file=ex03-jgmenurc --icon-size=22),applications-games"
echo -e "Graphics,^sub(../jgmenu_xdg --parent=./ex03-xdg-simple.sh Graphics | ../jgmenu --config-file=ex03-jgmenurc --icon-size=22),applications-graphics"
echo -e "Network,^sub(../jgmenu_xdg --parent=./ex03-xdg-simple.sh Network | ../jgmenu --config-file=ex03-jgmenurc --icon-size=22),preferences-system-network"
echo -e "Office,^sub(../jgmenu_xdg --parent=./ex03-xdg-simple.sh Office | ../jgmenu --config-file=ex03-jgmenurc --icon-size=22),applications-office"
echo -e "Science,^sub(../jgmenu_xdg --parent=./ex03-xdg-simple.sh Science | ../jgmenu --config-file=ex03-jgmenurc --icon-size=22),applications-science"
echo -e "Settings,^sub(../jgmenu_xdg --parent=./ex03-xdg-simple.sh Settings | ../jgmenu --config-file=ex03-jgmenurc --icon-size=22),preferences-system"
echo -e "System,^sub(../jgmenu_xdg --parent=./ex03-xdg-simple.sh System | ../jgmenu --config-file=ex03-jgmenurc --icon-size=22),applications-system"
echo -e "Utility,^sub(../jgmenu_xdg --parent=./ex03-xdg-simple.sh Utility | ../jgmenu --config-file=ex03-jgmenurc --icon-size=22),applications-utilities"
) | ../jgmenu --config-file=ex03-jgmenurc --icon-size=22
