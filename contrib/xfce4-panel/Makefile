ifeq ($(shell uname -m),i686)
LIBDIR     = $(DESTDIR)$(prefix)/lib/i386-linux-gnu/xfce4/panel/plugins
else
LIBDIR     = $(DESTDIR)$(prefix)/lib/x86_64-linux-gnu/xfce4/panel/plugins
endif

ifndef VERBOSE
QUIET_CC   = @echo '     CC    '$@;
endif

PLUGIN_DIR = $(DESTDIR)$(prefix)/share/xfce4/panel/plugins

CFLAGS  ?= -s -Wall -shared -std=c99 -fPIC \
           -Wextra -Wno-format-zero-length \
	   -Wold-style-definition -Woverflow -Wpointer-arith \
	   -Wunused -Wvla -Wunused-result \
           -Wno-unused-parameter

all: libjgmenu.so

libjgmenu.so: jgmenu-applet.c jgmenu-applet.h
	$(QUIET_CC)$(CC) $(CFLAGS) -o libjgmenu.so jgmenu-applet.c `pkg-config --cflags --libs libxfce4panel-1.0 gtk+-3.0`

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
