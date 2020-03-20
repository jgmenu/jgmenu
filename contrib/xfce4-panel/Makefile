-include ../../config.mk
include ../../Makefile.inc

libdir    ?= `pkg-config --variable=libdir libxfce4panel-2.0`
LIBDIR     = $(DESTDIR)$(libdir)/xfce4/panel/plugins
PLUGIN_DIR = $(DESTDIR)$(prefix)/share/xfce4/panel/plugins

CFLAGS  += -Wno-strict-prototypes -Wno-declaration-after-statement
CFLAGS  += -s -shared -fPIC
CFLAGS  += `pkg-config --cflags libxfce4panel-2.0`
LDFLAGS += `pkg-config --libs   libxfce4panel-2.0`

xfce4-panel: libjgmenu.so

libjgmenu.so: jgmenu-applet.c
	$(QUIET_CC)$(CC) $(CFLAGS) -o libjgmenu.so jgmenu-applet.c $(LDFLAGS)

install:
	@install -d $(LIBDIR)
	@install -m644 libjgmenu.so $(LIBDIR)
	@install -d $(PLUGIN_DIR)
	@install -m644 jgmenu-applet.desktop $(PLUGIN_DIR)

uninstall:
	@rm -f $(LIBDIR)/libjgmenu.so
	@rm -f $(PLUGIN_DIR)/jgmenu-applet.desktop

clean:
	@rm -f libjgmenu.so
