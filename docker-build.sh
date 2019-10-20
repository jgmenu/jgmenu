#!/usr/bin/env bash

DOCKER_IMAGE=$DOCKER_IMAGE

if [[ -z "$DOCKER_IMAGE" ]]; then
	CHANGELOG_HEAD=$(head -n1 debian/changelog)
	case "$CHANGELOG_HEAD" in
		*lithium*|*buster*) 	DOCKER_IMAGE=debian:buster ;;
		*helium*|*stretch*) 	DOCKER_IMAGE=debian:stretch ;;
		*hydrogen*|*jessie*)	DOCKER_IMAGE=debian:jessie ;;
	esac
	if [[ -n "$USE_I386" ]]; then
		DOCKER_IMAGE="i386/$DOCKER_IMAGE"
	fi
fi

echo "DOCKER IMAGE: $DOCKER_IMAGE"

sudo rm -rf -- "$PWD"/build
sudo docker run --rm -v "$PWD:/mnt" "${DOCKER_IMAGE}" /bin/bash -c '
  set -e;

  echo -ne "\e[0;34m";
  source /etc/os-release;
  cat >/etc/apt/sources.list.d/sources.list <<<"
deb-src http://deb.debian.org/debian/ ${VERSION_CODENAME} main contrib non-free
deb-src http://security.debian.org/ ${VERSION_CODENAME}/updates main contrib non-free";
  apt-get update && apt-get -yy upgrade;
  apt-get install -yy debhelper devscripts lsb-release rsync git;

  echo "Copying source...";
  mkdir -p /tmp/build;
  rsync --archive /mnt/ /tmp/build/;

  echo "Building..."; set -x;
  cd /tmp/build/;
  yes | mk-build-deps -i debian/control; rm jgmenu-build-deps*.deb;
  echo -ne "\e[0m";
  lastref=$(git describe --tags --abbrev=0);
  lastref=${lastref%-*};
  gitbranch=$(git rev-parse --abbrev-ref HEAD);
  pkgbase=$(grep Source: debian/control|cut -d" " -f2);
  git archive --format=tgz "$gitbranch" > ../"${pkgbase}_${lastref}.orig.tar.gz";
  dpkg-buildpackage -F -rfakeroot -us -uc;
  echo -ne "\e[0;34m"; set +x;

  echo "Copying results...";
  arch=$(dpkg --print-architecture);
  mkdir -p /mnt/build/$arch;
  rsync /tmp/*.{deb,dsc,tar.*,changes} /mnt/build/$arch;
'
