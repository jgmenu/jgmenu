#!/bin/bash

(
echo -e "XDG MENU,^tag(root)"
echo -e "Full XDG menu,^sub(../jgmenu_xdg --parent=./t3.sh | ../jgmenu)"
echo -e "Audio,^sub(../jgmenu_xdg --parent=./t3.sh Audio | ../jgmenu)"
echo -e "Video,^sub(../jgmenu_xdg --parent=./t3.sh Video | ../jgmenu)"
echo -e "Development,^sub(../jgmenu_xdg --parent=./t3.sh Development | ../jgmenu)"
echo -e "Education,^sub(../jgmenu_xdg --parent=./t3.sh Education | ../jgmenu)"
echo -e "Game,^sub(../jgmenu_xdg --parent=./t3.sh Game | ../jgmenu)"
echo -e "Graphics,^sub(../jgmenu_xdg --parent=./t3.sh Graphics | ../jgmenu)"
echo -e "Network,^sub(../jgmenu_xdg --parent=./t3.sh Network | ../jgmenu)"
echo -e "Office,^sub(../jgmenu_xdg --parent=./t3.sh Office | ../jgmenu)"
echo -e "Science,^sub(../jgmenu_xdg --parent=./t3.sh Science | ../jgmenu)"
echo -e "Settings,^sub(../jgmenu_xdg --parent=./t3.sh Settings | ../jgmenu)"
echo -e "System,^sub(../jgmenu_xdg --parent=./t3.sh System | ../jgmenu)"
echo -e "Utility,^sub(../jgmenu_xdg --parent=./t3.sh Utility | ../jgmenu)"
) | ../jgmenu
