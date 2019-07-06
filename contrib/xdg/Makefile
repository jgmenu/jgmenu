CC      =  gcc
CFLAGS  =  -g -Wall
CFLAGS  += `xml2-config --cflags`
LIBS    += `xml2-config --libs`

path = ../../src/
src  = util.c sbuf.c xdgdirs.c argv-buf.c charset.c compat.c

jgmenu-xdg: jgmenu-xdg.c xdgapps.c
	$(CC) $(CFLAGS) -o $@ $^ $(addprefix $(path),$(src)) -I$(path) $(LIBS)
