#!/bin/sh

test_description='lx module'

. ./sharness.sh

if ! test -e ../../jgmenu-lx >/dev/null 2>&1
then
     skip_all='lx module is required'
     test_done
fi

if test -d /build
then
     skip_all='libmenu-cache does not like running in chroot'
     test_done
fi

generate_directories () {
	d="${PWD}/t1201/desktop-directories"
	printf "%b\n" ".directory files generated in: $d"
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
	d="${PWD}/t1201/applications"
	printf "%b\n" ".desktop files generated in: $d"
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

generate_menu_file () {
	d="${PWD}/t1201/menus"
	rm -rf "${d}"
	mkdir -p "${d}"
	cp ../t1201/menus/test2-applications.menu "$d"
}

test_menu () {

	d="${PWD}/t1201"
	export XDG_CONFIG_DIRS="${d}"
	export XDG_DATA_DIRS="${d}"
	printf "%b\n" "XDG_DATA_DIRS=$XDG_DATA_DIRS"
	export XDG_MENU_PREFIX="${1}-"

	rm -rf ~/.cache/menus &&
	cp "../t1201/${1}.expect" expect &&
	LANG=C LC_ALL=C ../../jgmenu-lx >actual &&
	cp "../t1201/${1}.expect" expect &&
	test_cmp expect actual
}

test_expect_success 'nested lx menu with <layout>' '

generate_directories &&
generate_apps &&
generate_menu_file &&
test_menu "test2"

'

test_done

