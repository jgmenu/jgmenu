#!/bin/bash

#
# Copyright (C) Bunsenlabs' misko_2083
# Copied with permission from 
# https://forums.bunsenlabs.org/viewtopic.php?pid=68098#p68098
#
# This script requires imagemagick, xprop, xdotool and wmctrl
#

# Temp dir to store the icons
TMPDIR=$(mktemp -d /tmp/XXXXXX)

trap "rm -rf $TMPDIR" EXIT

# With process substitution output from "wmctrl -l" is redirected to file descriptor 3
# while loop reads  "win_id" and "display" variables from file descriptor 3
while read -r win_id display text <&3
  do
     # filter windows with win_id that has X window properties of type normal or dialog
     if xprop -id $win_id | grep -e "^_NET_WM_WINDOW_TYPE(ATOM) = _NET_WM_WINDOW_TYPE_NORMAL" \
                                 -e "^_NET_WM_WINDOW_TYPE(ATOM) = _NET_WM_WINDOW_TYPE_DIALOG" &>/dev/null
     then
         # Get WM_CLASS X window property
         wm_class=$(xprop -id $win_id WM_CLASS | awk -F'"' '{print $4}')

         if  hash convert &>/dev/null;then
           # Convert icon to pam then use imagemagic to convert to png
           xprop -notype 32c -id $win_id _NET_WM_ICON \
                  |    perl -0777 -pe '@_=/\d+/g;
                  printf "P7\nWIDTH %d\nHEIGHT %d\nDEPTH 4\nMAXVAL 255\nTUPLTYPE RGB_ALPHA\nENDHDR\n", splice@_,0,2;
                  $_=pack "N*", @_;
                   s/(.)(...)/$2$1/gs' \
                  | convert pam:- $TMPDIR/$wm_class.png 2>/dev/null
         fi

         # if WM_CLASS is a "Wrapper"
         if [[ "${wm_class}" == "Wrapper" ]]
         then
             # Get WM_CLASS X window property _NET_WM_NAME
             wm_class=$(xprop -id $win_id _NET_WM_NAME | awk -F '"' '{print $2}')

             echo "\"\"\"$display $text\"\"\",xdotool windowactivate $win_id,xfwm4"
         else
             # print
             echo "\"\"\"$display $text\"\"\",xdotool windowactivate $win_id,$TMPDIR/$wm_class.png"
         fi
     fi
done 3< <(wmctrl -l | awk '{print $1,$2,substr($0, index($0,$4), 28)}') | jgmenu --simple --at-pointer

# Close file descriptor 3
exec 3<&-

exit
