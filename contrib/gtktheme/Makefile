-include ../../config.mk
include ../../Makefile.inc

gtktheme:
	@:

install:
	@install -d $(DESTDIR)$(libexecdir)/jgmenu
	@install -m775 jgmenu-gtktheme.py $(DESTDIR)$(libexecdir)/jgmenu

uninstall:
	@rm -f $(libexecdir)/jgmenu/jgmenu-gtktheme.py

clean:
