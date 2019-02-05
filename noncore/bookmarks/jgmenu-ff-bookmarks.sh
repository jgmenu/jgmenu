#!/bin/sh

# Inspired-by:
#   - https://forum.archlabslinux.com/t/jgmenu-bookmarks/696
#   - https://blog.sleeplessbeastie.eu/2014/01/02/how-to-open-firefox-bookmarks-from-openbox-menu/

# If you use iceweasel, set JGMENU_BROWSER=iceweasel before running script
: ${JGMENU_BROWSER="firefox"}
database=$(find ~/.mozilla/${JGMENU_BROWSER}/ -name "places.sqlite")
submenus=$(mktemp)

usage () {
	printf "%b\n" "\
usage: jgmenu_run ff-bookmarks\n\
Generate jgmenu flavoured CSV menu data for firefox/iceweasel bookmarks"
}

check_dependency () {
	if ! type "$1" >/dev/null 2>&1
	then
		printf "%b\n" "'$1' is required to run module [ff-bookmarks]"
		exit 1
	fi
}

process_bookmarks () {
	query="select b.title, p.url from moz_bookmarks as b left outer join \
               moz_places as p on b.fk=p.id where b.type = 1 and p.hidden=0 \
               and b.title not null and parent=$1"
	sqlite3 -separator ^ "$database" "$query" | while IFS=^ read title url
	do
		test -z "$title" && title=$url
		echo "$title, firefox $url" >>$submenus
	done
}

process_folders () {
	query="select id, title from moz_bookmarks where parent=$1 and type=2 \
               and (select count(*) from moz_bookmarks as b2 where \
               b2.parent=moz_bookmarks.id)>0"
	sqlite3 -separator ^ "$database" "$query" | while IFS=^ read id title
	do
		test -z "$title" && title="(no title)"
		echo "$title,^checkout($title)"
		echo "$title,^tag($title)" >>$submenus
		process_folders $id
		process_bookmarks $id 
	done
}

if test "$1" = "--help"
then
	usage
	exit 0
fi

check_dependency ${JGMENU_BROWSER}
check_dependency sqlite3

# unfiled bookmarks
root="(select id from moz_bookmarks where rtrim(guid,'_')='unfiled')"
process_bookmarks "$root"

# filed bookmarks
root="(select id from moz_bookmarks where rtrim(guid,'_')='menu')"
process_bookmarks "$root"
process_folders "$root"
cat $submenus
rm -f $submenus
