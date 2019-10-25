-include ../../config.mk
include ../../Makefile.inc

CFLAGS  += `pkg-config --cflags glib-2.0 libmenu-cache`
LDFLAGS += `pkg-config --libs glib-2.0 libmenu-cache`

path = ../../src/
src  = util.c sbuf.c xdgdirs.c argv-buf.c back.c fmt.c

jgmenu-lx: jgmenu-lx.c
	$(QUIET_CC)$(CC) $(CFLAGS) -o $@ $^ $(addprefix $(path),$(src)) -I$(path) $(LDFLAGS)

install:
	@install -d $(DESTDIR)$(libexecdir)
	@install -m755 jgmenu-lx $(DESTDIR)$(libexecdir)

clean:
	@rm -f jgmenu-lx
