-include ../../config.mk
include ../../Makefile.inc

pmenu:
	@:

install:
	@install -d $(DESTDIR)$(libexecdir)/jgmenu
	@install -m775 jgmenu-pmenu.py $(DESTDIR)$(libexecdir)/jgmenu

uninstall:
	@rm -f $(libexecdir)/jgmenu-pmenu.py

clean:
