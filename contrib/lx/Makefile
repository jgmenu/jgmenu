-include ../../config.mk

ifndef VERBOSE
QUIET_CC   = @echo '     CC    '$@;
endif

libexecdir = $(prefix)/lib/jgmenu

CFLAGS  ?= -Wall -std=gnu99 -Wextra -Wno-format-zero-length \
	   -Wold-style-definition -Woverflow -Wpointer-arith \
	   -Wunused -Wvla -Wunused-result -Wno-unused-parameter

CFLAGS  += `pkg-config --cflags glib-2.0 libmenu-cache`
LDFLAGS += `pkg-config --libs glib-2.0 libmenu-cache`

path = ../../src/
src  = util.c sbuf.c xdgdirs.c argv-buf.c back.c fmt.c

jgmenu-lx: jgmenu-lx.c
	$(QUIET_CC)$(CC) $(CFLAGS) -o $@ $^ $(addprefix $(path),$(src)) -I$(path) $(LDFLAGS)

install:
	@install -d $(DESTDIR)$(libexecdir)
	@install -m755 $(PROGS_LIBEXEC) jgmenu-lx $(DESTDIR)$(libexecdir)

clean:
	@rm -f jgmenu-lx
