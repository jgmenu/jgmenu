VER=0.1
CC = gcc

CFLAGS   = -g -pedantic -Wall -Os
CFLAGS  += -DVERSION='"$(VER)"'
CFLAGS  += -DXINERAMA
CFLAGS  += `pkg-config cairo pango pangocairo --cflags`

LIBS  = `pkg-config x11 xinerama cairo pango pangocairo --libs`

LDFLAGS  = $(LIBS)

OBJS =  x11-ui.o config.o util.o

all: clean jgmenu

jgmenu: $(OBJS)
	@echo $(CC) $@
	@$(CC) $(LDFLAGS) $(CFLAGS) $(INCS) -o jgmenu $(OBJS) jgmenu.c


x11-ui.o: x11-ui.c x11-ui.h config.h
	@echo $(CC) $@
	@$(CC) $(CFLAGS) $(LIBS) $(INCS) -c x11-ui.c

config.o: config.c config.h
	@echo $(CC) $@
	@$(CC) $(CFLAGS) $(LIBS) $(INCS) -c config.c

util.o: util.c util.h
	@echo $(CC) $@
	@$(CC) $(CFLAGS) $(LIBS) $(INCS) -c util.c

install: jgmenu
	@echo installing...
	@sudo cp -pv ./jgmenu /usr/bin/
	@sudo cp -pv ./jgmenu_run /usr/bin/

clean:
	@echo cleaning...
	@rm -f *.o

test: all
	$(MAKE) -C tests/ all
