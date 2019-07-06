#!/bin/sh

#
# This assumes you have build-essential, devscripts, debhelper, git
#

test -d "debian" || { echo "fatal: need to run from project root"; exit 1; }
test -f "src/jgmenu.c" || { echo "fatal: need to run from project root"; exit 1; }

ver=$(./scripts/version-gen.sh | sed -e 's/^jgmenu v//' | cut -f1 -d"-")
#git describe --exact-match HEAD 2>/dev/null || echo "warn: not at tag"

rm ../jgmenu_*
dh_make -p jgmenu_${ver} --createorig --addmissing --single

rm -f debian/watch.ex
rm -f debian/prerm.ex
rm -f debian/preinst.ex
rm -f debian/postrm.ex
rm -f debian/postinst.ex
rm -f debian/package.doc-base.EX
rm -f debian/package.default.ex
rm -f debian/package.cron.d.ex
rm -f debian/menu.ex
rm -f debian/manpage.xml.ex
rm -f debian/manpage.sgml.ex
rm -f debian/manpage.1.ex
rm -f debian/jgmenu.doc-base.EX
rm -f debian/jgmenu.default.ex
rm -f debian/jgmenu.cron.d.ex
rm -f debian/init.d.ex

echo "======================================="
echo "Now run 'debuild -us -uc' or 'pdebuild'"
