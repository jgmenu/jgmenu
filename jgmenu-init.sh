#!/bin/sh

# `jgmenu_run init` creates/updates jgmenurc 

tmp_jgmenurc=$(mktemp)
jgmenurc=~/.config/jgmenu/jgmenurc
jgmenurc_bak=${jgmenurc}.$(date +%Y%m%d%H%M)

mkdir -p ~/.config/jgmenu

populate_tmp_file () {
cat <<'EOF' >>"${tmp_jgmenurc}"
stay_alive	  = 0
hide_on_startup	  = 0
menu_margin_x	  = 0
menu_margin_y	  = 32
menu_width	  = 200
menu_radius	  = 1
menu_border	  = 0
menu_halign	  = left
menu_valign	  = bottom
at_pointer	  = 0
item_margin_x	  = 3
item_margin_y	  = 3
item_height	  = 25
item_padding_x	  = 4
item_radius	  = 1
item_border	  = 0
sep_height	  = 5
font		  = Cantarell 10
icon_size	  = 22
icon_theme	  = Adwaita
ignore_xsettings  = 0
ignore_icon_cache = 0
show_title	  = 0
arrow_show	  = 1
search_all_items  = 1
color_menu_bg	  = #000000 70
color_menu_fg	  = #eeeeee 20
color_menu_border = #eeeeee 8
color_norm_bg	  = #000000 00
color_norm_fg	  = #eeeeee 100
color_sel_bg	  = #ffffff 20
color_sel_fg	  = #eeeeee 100
color_sel_border  = #eeeeee 8
color_noprog_fg	  = #eeeeee 100
color_title_bg	  = #ffffff 20
color_sep_fg	  = #ffffff 20
EOF
}

backup_jgmenurc () {
	cp -p ${jgmenurc} ${jgmenurc_bak}
}

amend_jgmenurc () {
	while IFS= read -r line
	do
		v=$(echo ${line%%=*} | tr -d ' ')
		test -z "${v}" && continue
		if ! grep "^${v}" "${jgmenurc}" >/dev/null
		then
			printf "%b\n" "${line}"
			printf "%b\n" "${line}" >> ${jgmenurc}
		fi
	done <${tmp_jgmenurc}
}

# START OF SCRIPT

populate_tmp_file

if test -e ${jgmenurc}
then
	backup_jgmenurc
	printf "%s\n" "Amending jgmenurc"
	amend_jgmenurc
else
	printf "%s\n" "Creating jgmenurc"
	cp ${tmp_jgmenurc} ${jgmenurc}
fi

rm -f ${tmp_jgmenurc}
