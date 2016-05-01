#!/bin/bash

(
echo -e "XDG MENU,^tag(root)"
echo -e "Full XDG menu,../jgmenu_xdg | ../jgmenu"
echo -e "Audio,../jgmenu_xdg Audio | ../jgmenu"
echo -e "Video,../jgmenu_xdg Video | ../jgmenu"
echo -e "Development,../jgmenu_xdg Development | ../jgmenu"
echo -e "Education,../jgmenu_xdg Education | ../jgmenu"
echo -e "Game,../jgmenu_xdg Game | ../jgmenu"
echo -e "Graphics,../jgmenu_xdg Graphics | ../jgmenu"
echo -e "Network,../jgmenu_xdg Network | ../jgmenu"
echo -e "Office,../jgmenu_xdg Office | ../jgmenu"
echo -e "Science,../jgmenu_xdg Science | ../jgmenu"
echo -e "Settings,../jgmenu_xdg Settings | ../jgmenu"
echo -e "System,../jgmenu_xdg System | ../jgmenu"
echo -e "Utility,../jgmenu_xdg Utility | ../jgmenu"
) | ../jgmenu
