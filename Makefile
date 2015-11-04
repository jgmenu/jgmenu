VER=0.1
CC = gcc

CFLAGS   = -g -pedantic -Wall -Os
CFLAGS  += -DVERSION='"$(VER)"'
CFLAGS  += -DXINERAMA
CFLAGS  += `pkg-config cairo pango pangocairo --cflags`

LIBS  = `pkg-config x11 xinerama cairo pango pangocairo --libs`

LDFLAGS  = $(LIBS)

OBJS =  x11-ui.o config.o util.o geometry.o

all: jgmenu x11-ui.o config.o util.o geometry.o

jgmenu: jgmenu.c $(OBJS)
	@echo $(CC) $@
	@$(CC) $(LDFLAGS) $(CFLAGS) -o jgmenu $(OBJS) jgmenu.c


x11-ui.o: x11-ui.c x11-ui.h
	@echo $(CC) $@
	@$(CC) $(CFLAGS) $(LIBS) -c x11-ui.c

config.o: config.c config.h
	@echo $(CC) $@
	@$(CC) $(CFLAGS) $(LIBS) -c config.c

util.o: util.c util.h
	@echo $(CC) $@
	@$(CC) $(CFLAGS) $(LIBS) -c util.c

geometry.o: geometry.c geometry.h
	@echo $(CC) $@
	@$(CC) $(CFLAGS) $(LIBS) -c geometry.c

install: jgmenu
	@echo installing...
	@sudo cp -pv ./jgmenu /usr/bin/
	@sudo cp -pv ./jgmenu_run /usr/bin/

clean:
	@echo cleaning...
	@rm -f jgmenu
	@rm -f *.o

test: all
	$(MAKE) -C tests/ all
