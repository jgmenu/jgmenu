CC      =  gcc
CFLAGS  =  -g -Wall
CFLAGS  += `xml2-config --cflags`
LIBS    += `xml2-config --libs`

all:
	gcc $(CFLAGS) -o jgmenu-xdg jgmenu-xdg.c ../../util.c ../../sbuf.c ../../xdgdirs.c ../../xdgapps.c ../../argv-buf.c ../../charset.c -I../.. $(LIBS)

