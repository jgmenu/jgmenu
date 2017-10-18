#!/bin/sh
#
# This assumes you have build-essential, devscripts, debhelper, git
#

printf "%b" "This script still experimental. Are you sure you want to continue [yN] "
read answer
test "$answer" = "y" || exit 1
test -d "debian" || { echo "fatal: need to run from packacking/"; exit 1; }

ver=$(../scripts/version-gen.sh | sed -e 's/^jgmenu v//' | cut -f1 -d"-")
dest_dir="jgmenu-${ver}"
rm -rf jgmenu*
echo "version ${ver}"
#git describe --exact-match HEAD 2>/dev/null || echo "warn: not at tag"
cd ..
git archive --format=tar --prefix="${dest_dir}/" v${ver} | gzip >packaging/jgmenu_${ver}.orig.tar.gz
cd packaging
tar -xf jgmenu_${ver}.orig.tar.gz
cp -a debian/ ${dest_dir}
cd ${dest_dir}
debuild -us -uc
