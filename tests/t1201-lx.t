#!/bin/sh

test_description='lx module'

. ./sharness.sh

if ! test -e ../../jgmenu-lx >/dev/null 2>&1
then
     skip_all='lx module is required'
     test_done
fi

generate_directories () {
	d="${PWD}/../t1201/desktop-directories"
	rm -rf "${d}"
	mkdir -p "${d}"
	for i in $(seq 9)
	do
		printf "%b\n" "[Desktop Entry]" >>"${d}/${i}.directory"
		printf "%b\n" "Name=${i}" >>"${d}/${i}.directory"
		printf "%b\n" "Type=Directory" >>"${d}/${i}.directory"
	done
}

generate_apps () {
	d="${PWD}/../t1201/applications"
	rm -rf "${d}"
	mkdir -p "${d}"
	for i in $(seq 9)
	do
		for j in $(seq 9)
		do
			printf "%b\n" "[Desktop Entry]" >>"${d}/${i}-${j}.desktop"
			printf "%b\n" "Name=${i}-${j}" >>"${d}/${i}-${j}.desktop"
			printf "%b\n" "Exec=${i}-${j}" >>"${d}/${i}-${j}.desktop"
			printf "%b\n" "Type=Application" >>"${d}/${i}-${j}.desktop"
			printf "%b\n" "Categories=${i}" >>"${d}/${i}-${j}.desktop"
		done
	done
}

test_menu () {

	d="${PWD}/../t1201"
	export XDG_CONFIG_DIRS="${d}"
	export XDG_DATA_HOME="${d}"
	export XDG_MENU_PREFIX="${1}-"

	rm -rf ~/.cache/menus &&
	LANG=C LC_ALL=C ../../jgmenu-lx >actual &&
	cp "${d}/${1}.expect" expect &&
	test_cmp expect actual
}

test_expect_success 'nested lx menu with <layout>' '

generate_directories &&
generate_apps &&
test_menu "test2"

'

test_done

