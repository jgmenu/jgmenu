#!/bin/bash

# Requirements: devscripts

PKG=jgmenu

set -x

rm -rf ${PKG}* 2>/dev/null || true

if [ ! -z "$1" ]
then
    MINOR="$1"
else
    MINOR="1"
fi

# Get version (and check that the repository is clean)
VERSION="$(git show -s --pretty=format:%cI.%ct.%h | tr -d ':' | tr -d '-' | tr '.' '-' | sed 's/T[0-9\+]*//g').$MINOR"
REPO="tint2-git"

set -e

# Export repository contents to source directory
DIR=$PKG-$VERSION
echo "Making release $DIR"

pushd .
cd ..
git checkout-index --prefix=ppa/$DIR/ -a
popd

# Copy the debian files into the source directory
cp -r ../debian $DIR/debian

for DISTRO in trusty xenial zesty artful
do
    # Cleanup from previous builds
    rm -rf ${PKG}_$VERSION-*

    # Update debian package changelog if necessary
    echo -e "$PKG ($VERSION-$DISTRO-1) $DISTRO; urgency=medium\n\n$(git log --pretty=format:'  * %h %an (%ci) %s %d')\n -- o9000 <mrovi9000@gmail.com>  $(date -R)\n" > $DIR/debian/changelog

    # Create source tarball
    ARCHIVE=${PKG}_$VERSION-$DISTRO.orig.tar.gz
    rm -rf $ARCHIVE
    tar -czf $ARCHIVE $DIR

    # Build package
    KEY=$(gpg --list-secret-keys | awk '/^sec/ { print $2 }' | cut -d / -f 2)

    pushd .
    cd $DIR
    debuild -S -k$KEY
    popd

    # Upload package
    dput ppa:o9000/$REPO ${PKG}_$VERSION-$DISTRO-1_source.changes
done

# Cleanup
rm -rf $DIR $ARCHIVE
rm -rf ${PKG}_$VERSION-*
