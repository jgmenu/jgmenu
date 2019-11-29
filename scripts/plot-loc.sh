#!/bin/bash

type git >/dev/null 2>&1 || { printf 'fatal: need git\n'; exit 1; }
type gnuplot >/dev/null 2>&1 || { printf 'fatal: need gnuplot\n'; exit 1; }
[[ $(git diff --stat) != '' ]] && { printf 'fatal: repo is dirty\n'; exit 1; }

versions="$(git tag | tr '\n' ' ')"
#versions="v3.2 v3.3 v3.4 v3.5"

loc_file="locdata"
loc_graph="loc.png"
gnuplot_commands="gnuplotcommands"
cat >$gnuplot_commands <<'EOF'
set title "jgmenu C LOC vs time"
#unset multiplot
set xdata time
set term png
set timefmt "%Y-%m-%d"
set format x "%Y"
set xlabel "Time"
set ylabel "LOC(C)"
set autoscale x
#set xrange ["2015-01-01":"2020-12-31"]
set yrange ["0":"12000"]
#set autoscale y
EOF
printf '%b\n' "set output \"$loc_graph\"" >>$gnuplot_commands
printf '%b\n' "plot \"$loc_file\" using 1:2" >>$gnuplot_commands

trap "rm -f ${loc_file} ${gnuplot_commands}" EXIT

generate_loc_data () {
	for v in $versions
	do
		git checkout $v 2>/dev/null
		if [[ -d src/ ]]; then
			dir="src/"
		else
			dir="."
		fi
		tag_date=$(git log --tags --simplify-by-decoration \
				   --pretty="format:%ai %d" \
				   | grep "$v)" \
				   | awk '{ print $1 }')
		tag_loc=$(cloc $dir | grep '^C ' | awk '{ print $5 }')
		printf '%b\n' "[$v] $tag_date $tag_loc"
		printf '%b\n' "$tag_date $tag_loc" >>$loc_file

	done
	git checkout master 2>/dev/null
}

generate_loc_data
gnuplot <$gnuplot_commands
