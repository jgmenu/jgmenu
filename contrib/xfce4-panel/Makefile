-include ../../config.mk
include ../../Makefile.inc

ifeq ($(shell dpkg-architecture -q DEB_HOST_ARCH 2>/dev/null),i386)
LIBDIR     = $(DESTDIR)$(prefix)/lib/i386-linux-gnu/xfce4/panel/plugins
else ifeq ($(shell uname -m),i686)
LIBDIR     = $(DESTDIR)$(prefix)/lib/i386-linux-gnu/xfce4/panel/plugins
else
LIBDIR     = $(DESTDIR)$(prefix)/lib/x86_64-linux-gnu/xfce4/panel/plugins
endif

PLUGIN_DIR = $(DESTDIR)$(prefix)/share/xfce4/panel/plugins

CFLAGS  += -Wno-strict-prototypes -Wno-declaration-after-statement
CFLAGS  += -s -shared -fPIC
CFLAGS  += `pkg-config --cflags libxfce4panel-1.0`
LDFLAGS += `pkg-config --libs   libxfce4panel-1.0`

xfce4-panel: libjgmenu.so

libjgmenu.so: jgmenu-applet.c
	$(QUIET_CC)$(CC) $(CFLAGS) -o libjgmenu.so jgmenu-applet.c $(LDFLAGS)

install:
	@install -d $(LIBDIR)
	@install -m775 libjgmenu.so $(LIBDIR)
	@install -d $(PLUGIN_DIR)
	@install -m644 jgmenu-applet.desktop $(PLUGIN_DIR)

uninstall:
	@rm -f $(LIBDIR)/libjgmenu.so
	@rm -f $(PLUGIN_DIR)/jgmenu-applet.desktop

clean:
	@rm -f libjgmenu.so
