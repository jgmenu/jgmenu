#!/bin/sh

# `jgmenu_run init` creates/updates jgmenurc

tmp_jgmenurc=$(mktemp)
jgmenurc=~/.config/jgmenu/jgmenurc
jgmenurc_bak=${jgmenurc}.$(date +%Y%m%d%H%M)
regression_items="max_items min_items ignore_icon_cache color_noprog_fg \
color_title_bg show_title search_all_items"

populate_tmp_file () {
cat <<'EOF' >>"${tmp_jgmenurc}"
stay_alive          = 1
hide_on_startup     = 0
csv_cmd             = jgmenu_run parse-pmenu
tint2_look          = 1
tint2_button        = 1
tint2_rules         = 1
at_pointer          = 0
menu_margin_x       = 0
menu_margin_y       = 32
menu_width          = 200
menu_radius         = 1
menu_border         = 0
menu_halign         = left
menu_valign         = bottom
item_margin_x       = 3
item_margin_y       = 3
item_height         = 25
item_padding_x      = 4
item_radius         = 1
item_border         = 0
item_halign         = left
sep_height          = 5
font                =
font_fallback       = xtg
icon_size           = 22
icon_text_spacing   = 10
icon_theme          =
icon_theme_fallback = xtg
arrow_string	    = ▸
arrow_show          = 1
color_menu_bg       = #000000 70
color_menu_fg       = #eeeeee 20
color_menu_border   = #eeeeee 8
color_norm_bg       = #000000 00
color_norm_fg       = #eeeeee 100
color_sel_bg        = #ffffff 20
color_sel_fg        = #eeeeee 100
color_sel_border    = #eeeeee 8
color_sep_fg        = #ffffff 20
EOF
}

backup_jgmenurc () {
	cp -p ${jgmenurc} ${jgmenurc_bak}
}

print_start_msg () {
	if test -z ${printed+x}
	then
		printf "%b\n" "Amending config file with missing items..."
		printf "\n\n%b\n\n" "### the items below were added by 'jgmenu_run init'" >> ${jgmenurc}
	fi
	printed=1
}

amend_jgmenurc () {
	prefix="      - "
	while IFS= read -r line
	do
		v=$(echo ${line%%=*} | tr -d ' ')
		test -z "${v}" && continue
		if ! grep "^${v}\|^#${v}" "${jgmenurc}" >/dev/null
		then
			print_start_msg
			printf "${prefix}%b\n" "${line}"
			printf "#%b\n" "${line}" >> ${jgmenurc}
		fi
	done <${tmp_jgmenurc}
}

# START OF SCRIPT

mkdir -p ~/.config/jgmenu

populate_tmp_file

if test -e ${jgmenurc}
then
	backup_jgmenurc
	amend_jgmenurc
else
	printf "%s\n" "Creating jgmenurc"
	cp ${tmp_jgmenurc} ${jgmenurc}
fi

rm -f ${tmp_jgmenurc}

# Check for jgmenurc items which are no longer valid
for r in ${regression_items}
do
	if grep ${r} ${jgmenurc} >/dev/null
	then
		printf "%b\n" "warning: ${r} is no longer a valid key"
	fi
done
