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

ASAN_FLAGS = -O0 -fsanitize=address -fno-common -fno-omit-frame-pointer -rdynamic
CFLAGS    += $(ASAN_FLAGS)
LDFLAGS   += $(ASAN_FLAGS)

SCRIPTS  = jgmenu_run
PROGS	 = jgmenu jgmenu_xdg

OBJS =  x11-ui.o config.o util.o geometry.o isprog.o sbuf.o icon-find.o icon-cache.o

all: $(PROGS) $(OBJS)

jgmenu: jgmenu.c $(OBJS)
	@echo $(CC) $@
	@$(CC) $(CFLAGS) -o jgmenu $(OBJS) jgmenu.c -pthread $(LDFLAGS)

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

isprog.o: isprog.c isprog.h list.h
	@echo $(CC) $@
	@$(CC) $(CFLAGS) $(LIBS) -c isprog.c

sbuf.o: sbuf.c sbuf.h
	@echo $(CC) $@
	@$(CC) $(CFLAGS) $(LIBS) -c sbuf.c

icon-find.o: icon-find.c icon-find.h sbuf.h
	@echo $(CC) $@
	@$(CC) $(CFLAGS) $(LIBS) -c icon-find.c sbuf.c

icon-cache.o: icon-cache.c icon-cache.h icon-find.h sbuf.h
	@echo $(CC) $@
	@$(CC) $(CFLAGS) $(LIBS) -c icon-cache.c

jgmenu_xdg: jgmenu_xdg.c util.o
	@echo $(CC) $@
	@$(CC) -o jgmenu_xdg jgmenu_xdg.c util.o $(LDFLAGS)

install: $(PROGS) $(SCRIPTS)
	@install -d $(DESTDIR)$(bindir)
	@install -m755 $(PROGS) $(SCRIPTS) $(DESTDIR)$(bindir)

clean:
	@rm -f $(PROGS) *.o

test:
	$(MAKE) -C tests/ all

ex:
	$(MAKE) -C examples/ all
