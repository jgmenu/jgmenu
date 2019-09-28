MATE_APPLET_DIR = /usr/share/mate-panel/applets
SERVICE_DIR = /usr/share/dbus-1/services
BINDIR = $(HOME)/bin

install:
	install -d $(MATE_APPLET_DIR)
	install -m644 org.mate.panel.JgMenuApplet.mate-panel-applet $(MATE_APPLET_DIR)
	install -d $(SERVICE_DIR)
	install -m644 org.mate.panel.applet.JgMenuAppletFactory.service $(SERVICE_DIR)
	install -m755 mate-jgmenu.py /home/johan/bin
