VER = $(shell git describe 2>/dev/null)
CC = gcc

CFLAGS   = -g -Wall -Os
#CFLAGS  += -pedantic 
CFLAGS  += -DVERSION='"$(VER)"'
CFLAGS  += -DXINERAMA
CFLAGS  += `pkg-config xrender cairo pango pangocairo --cflags`

LIBS  = `pkg-config x11 xinerama xrender cairo pango pangocairo --libs`

LDFLAGS  = $(LIBS)

OBJS =  x11-ui.o config.o util.o geometry.o prog-finder.o

all: jgmenu $(OBJS)

jgmenu: jgmenu.c $(OBJS)
	@echo $(CC) $@
	@$(CC) $(LDFLAGS) $(CFLAGS) -o jgmenu $(OBJS) jgmenu.c


x11-ui.o: x11-ui.c x11-ui.h
	@echo $(CC) $@
	@$(CC) $(CFLAGS) $(LIBS) -c x11-ui.c

config.o: config.c config.h util.c
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

install: jgmenu
	@echo installing...
	@sudo cp -pv ./jgmenu /usr/bin/
	@sudo cp -pv ./jgmenu_run /usr/bin/

clean:
	@echo cleaning...
	@rm -f jgmenu
	@rm -f *.o

test:
	$(MAKE) -C tests/ all
