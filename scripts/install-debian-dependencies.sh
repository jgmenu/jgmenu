#!/bin/sh

printf "%b\n" "info: installing build dependencies for jgmenu..."

sudo apt-get install \
	libx11-dev \
	libxrandr-dev \
	libcairo2-dev \
	libpango1.0-dev \
	librsvg2-dev \
	libxml2-dev \
	libglib2.0-dev \
	libmenu-cache-dev
