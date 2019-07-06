LIBDIR     = /usr/lib/x86_64-linux-gnu/xfce4/panel/plugins
PLUGIN_DIR = /usr/share/xfce4/panel/plugins

all:
	gcc -s -Wall -shared -o libjgmenu.so -fPIC jgmenu-applet.c `pkg-config --cflags --libs libxfce4panel-1.0 gtk+-3.0`

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
