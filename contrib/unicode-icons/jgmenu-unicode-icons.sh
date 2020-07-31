#!/bin/bash

# This is a template script that makes it easy to generate a menu with hand-picked
# Unicode glyphs (from Font Awesome and the like) acting as icons, with
# their size and alignment being separated from actual labels.

# The script should be set as a CSV source in the jgmenurc config file,
# i.e.: csv_cmd = bash ~/.config/jgmenu/jgmenu-unicode-icons.sh

readonly defaultIconFont="FontAwesome"
readonly defaultIconSize="15px"
readonly defaultIconLeftOffset="0px"
readonly defaultIconRightOffset="6px"
readonly defaultIconVerticalOffset="-2px"

formatSpacing() {
    local output
    output+="<span font='Monospace 1px'>"
    for i in $(seq 1 ${1::-2}); do
        output+=" "
    done
    output+="</span>"
    echo -n "$output"
}

formatIconAndLabel() {
    local icon="$1"
    local iconFont="$2" # optional
    local iconSize="$3" # optional
    local iconLeftOffset="$4" # optional
    local iconRightOffset="$5" # optional
    local iconVerticalOffset="$6" # optional
    local label="$7"

    [[ -z "$iconFont" ]] && iconFont="$defaultIconFont"
    [[ -z "$iconSize" ]] && iconSize="$defaultIconSize"
    [[ -z "$iconLeftOffset" ]] && iconLeftOffset="$defaultIconLeftOffset"
    [[ -z "$iconRightOffset" ]] && iconRightOffset="$defaultIconRightOffset"
    [[ -z "$iconVerticalOffset" ]] && iconVerticalOffset="$defaultIconVerticalOffset"

    local iconRise=$(echo "${iconVerticalOffset::-2} * 0.0625 * 10000 / 1" | bc)

    local output
    output+="$(formatSpacing $iconLeftOffset)"
    output+="<span font='$iconFont $iconSize' rise='$iconRise'>$icon</span>"
    output+="$(formatSpacing $iconRightOffset)"
    output+="$label"
    echo -n "$output"
}

echo -n "\
$(formatIconAndLabel "" "" "" "" "" "" "Example label 1"), example_command_1
$(formatIconAndLabel "" "" "" "" "" "" "Example label 2"), example_command_2
$(formatIconAndLabel "" "" "" "" "7px" "" "Example label 3"), example_command_3
$(formatIconAndLabel "" "" "" "5px" "10px" "" "Example label 4"), example_command_4
"

# Note: The default values of horizontal icon offsets for the 3rd and 4th entry
# have been overriden. That's because glyphs used in these entries have variable width.
