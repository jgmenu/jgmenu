# This shell script fragment is designed to be sourced by jgmenu-init.sh

set_theme_bunsenlabs () {
cat <<'EOF' >"${jgmenurc}"
tint2_look          = 0
at_pointer          = 1
EOF

if test -e ~/.config/openbox/menu.xml
then
	printf "%s\n" "csv_cmd             = ob"  >>"${jgmenurc}"
else
	printf "%s\n" "csv_cmd             = pmenu"  >>"${jgmenurc}"
fi

cat <<'EOF' >>"${jgmenurc}"
menu_width          = 120
menu_padding_top    = 0
menu_padding_right  = 0
menu_padding_bottom = 0
menu_padding_left   = 0
menu_radius         = 1
sub_spacing         = 3
item_margin_x       = 1
item_margin_y       = 1
item_height         = 19
sep_height          = 4
icon_size           = 0
arrow_width         = 8
color_menu_bg       = #3a3a3a 100
EOF
}

