-include ../../config.mk

prefix    ?= /usr/local
libexecdir = $(prefix)/lib/jgmenu

all:

install:
	@install -d $(DESTDIR)$(libexecdir)
	@install -m775 jgmenu-gtktheme.py $(DESTDIR)$(libexecdir)

uninstall:
	@rm -f $(libexecdir)/gtktheme.py

clean:
