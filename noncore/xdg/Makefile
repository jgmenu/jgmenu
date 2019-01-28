CC      =  gcc
CFLAGS  =  -g -Wall
CFLAGS  += `xml2-config --cflags`
LIBS    += `xml2-config --libs`

path = ../../src/
src  = util.c sbuf.c xdgdirs.c xdgapps.c argv-buf.c charset.c

jgmenu-xdg: jgmenu-xdg.c
	$(CC) $(CFLAGS) -o $@ $^ $(addprefix $(path),$(src)) -I$(path) $(LIBS)
