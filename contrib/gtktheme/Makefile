-include ../../config.mk

prefix    ?= /usr/local
libexecdir = $(prefix)/lib/jgmenu

all:

install:
	@install -d $(libexecdir)
	@install -m775 jgmenu-gtktheme.py $(libexecdir)

uninstall:
	@rm -f $(libexecdir)/gtktheme.py

clean:
