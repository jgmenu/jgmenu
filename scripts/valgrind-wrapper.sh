#!/bin/sh

menu_file=$(mktemp)

cat >${menu_file} <<EOF
xterm,xterm
firefox,firefox
EOF

if test $# -lt 1
then
	cmd="./jgmenu --die-when-loaded --vsimple --csv-file=${menu_file}"
else
	cmd="$@"
fi

valgrind --leak-check=full \
	 --leak-resolution=high \
	 --show-leak-kinds=all \
	 --num-callers=20 \
	 --log-file=valgrind.log \
	 --suppressions=scripts/valgrind.supp \
	 --gen-suppressions=all \
	 ${cmd}

printf "nr_lines="; cat valgrind.log | wc -l

rm -f ${menu_file}
