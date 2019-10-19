-include ../../config.mk

ifeq ($(shell dpkg-architecture -q DEB_HOST_ARCH 2>/dev/null),i386)
LIBDIR     = $(DESTDIR)$(prefix)/lib/i386-linux-gnu/xfce4/panel/plugins
else ifeq ($(shell uname -m),i686)
LIBDIR     = $(DESTDIR)$(prefix)/lib/i386-linux-gnu/xfce4/panel/plugins
else
LIBDIR     = $(DESTDIR)$(prefix)/lib/x86_64-linux-gnu/xfce4/panel/plugins
endif

ifndef VERBOSE
QUIET_CC   = @echo '     CC    '$@;
endif

PLUGIN_DIR = $(DESTDIR)$(prefix)/share/xfce4/panel/plugins

CFLAGS  ?= -Wall -std=c99 \
           -Wextra -Wno-format-zero-length \
	   -Wold-style-definition -Woverflow -Wpointer-arith \
	   -Wunused -Wvla -Wunused-result \
           -Wno-unused-parameter

CFLAGS  += -Wno-strict-prototypes -Wno-declaration-after-statement

CFLAGS  += -s -shared -fPIC
CFLAGS  += `pkg-config --cflags libxfce4panel-1.0 gtk+-3.0`
LDFLAGS += `pkg-config --libs   libxfce4panel-1.0 gtk+-3.0`

all: libjgmenu.so

libjgmenu.so: jgmenu-applet.c jgmenu-applet.h
	@echo xfce4-panel plugin requires libxfce4panel-1.0 and gtk+-3.0
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
