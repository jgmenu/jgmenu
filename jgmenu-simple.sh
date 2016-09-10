#!/bin/sh

(
echo -e "Terminal,xterm,utilities-terminal"
echo -e "File Manager,pcmanfm,folder"
echo -e "Web Browser,firefox,firefox"
echo -e "Suspend,systemctl -i suspend,system-log-out"
echo -e "Reboot,systemctl -i reboot,system-reboot"
echo -e "Poweroff,systemctl -i poweroff,system-shutdown"
) | jgmenu
