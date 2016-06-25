VER      = $(shell git describe 2>/dev/null)
CC       = gcc

prefix   = $(HOME)
bindir   = $(prefix)/bin

CFLAGS   = -g -Wall -Os
CFLAGS  += -DVERSION='"$(VER)"'
CFLAGS  += -DXINERAMA
CFLAGS  += `pkg-config cairo pango pangocairo librsvg-2.0 --cflags`

LIBS  = `pkg-config x11 xinerama cairo pango pangocairo librsvg-2.0 --libs`

LDFLAGS  = $(LIBS)

SCRIPTS  = jgmenu_run
PROGS	 = jgmenu jgmenu_xdg

OBJS =  x11-ui.o config.o util.o geometry.o prog-finder.o sbuf.o xdgicon.o

all: $(PROGS) $(OBJS)

jgmenu: jgmenu.c $(OBJS)
	@echo $(CC) $@
	@$(CC) $(CFLAGS) -o jgmenu $(OBJS) jgmenu.c $(LDFLAGS) -pthread

x11-ui.o: x11-ui.c x11-ui.h
	@echo $(CC) $@
	@$(CC) $(CFLAGS) $(LIBS) -c x11-ui.c

config.o: config.c config.h util.h
	@echo $(CC) $@
	@$(CC) $(CFLAGS) $(LIBS) -c config.c util.c

util.o: util.c util.h
	@echo $(CC) $@
	@$(CC) $(CFLAGS) $(LIBS) -c util.c

geometry.o: geometry.c geometry.h
	@echo $(CC) $@
	@$(CC) $(CFLAGS) $(LIBS) -c geometry.c

prog-finder.o: prog-finder.c prog-finder.h list.h
	@echo $(CC) $@
	@$(CC) $(CFLAGS) $(LIBS) -c prog-finder.c

sbuf.o: sbuf.c sbuf.h
	@echo $(CC) $@
	@$(CC) $(CFLAGS) $(LIBS) -c sbuf.c

xdgicon.o: xdgicon.c xdgicon.h sbuf.h
	@echo $(CC) $@
	@$(CC) $(CFLAGS) $(LIBS) -c xdgicon.c sbuf.c

jgmenu_xdg: jgmenu_xdg.c util.o
	@echo $(CC) $@
	@$(CC) -o jgmenu_xdg jgmenu_xdg.c util.o

install: $(PROGS) $(SCRIPTS)
	@install -d $(DESTDIR)$(bindir)
	@install -m755 $(PROGS) $(SCRIPTS) $(DESTDIR)$(bindir)

clean:
	@rm -f $(PROGS) *.o

test:
	$(MAKE) -C tests/ all

ex:
	$(MAKE) -C examples/ all
