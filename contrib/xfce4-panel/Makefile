ifeq ($(shell uname -m),i686)
LIBDIR     = /usr/lib/i386-linux-gnu/xfce4/panel/plugins
else
LIBDIR     = /usr/lib/x86_64-linux-gnu/xfce4/panel/plugins
endif

PLUGIN_DIR = /usr/share/xfce4/panel/plugins

CFLAGS  += -s -Wall -shared -std=c99 -fPIC
CFLAGS  += -Wextra -Wno-format-zero-length \
	   -Wold-style-definition -Woverflow -Wpointer-arith \
	   -Wunused -Wvla -Wunused-result
CFLAGS  += -Wno-unused-parameter

all:
	gcc $(CFLAGS) -o libjgmenu.so jgmenu-applet.c `pkg-config --cflags --libs libxfce4panel-1.0 gtk+-3.0`

install:
	install -d $(LIBDIR)
	install -m775 libjgmenu.so $(LIBDIR)
	install -d $(PLUGIN_DIR)
	install -m644 jgmenu-applet.desktop $(PLUGIN_DIR)

uninstall:
	rm -f $(LIBDIR)/libjgmenu.so
	rm -f $(PLUGIN_DIR)/jgmenu-applet.desktop

clean:
	rm -f libjgmenu.so
