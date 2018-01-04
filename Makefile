#
# Define ASAN=1 to enable AddressSanitizer
#
# Define VERBOSE=1 for a more verbose compilation
#
# Define PYTHON3_POLYGLOT=1 if '#!/usr/bin/env python3' is not going to work
# on your system.
#

VER      = $(shell ./scripts/version-gen.sh)

# Allow user to override build settings without making tree dirty
-include config.mk

include ./Makefile.inc

SCRIPTS_SHELL  = jgmenu_run jgmenu-init.sh

SCRIPTS_PYTHON = jgmenu-pmenu.py jgmenu-unity-hack.py

PROGS	 = jgmenu jgmenu-xdg jgmenu-ob jgmenu-socket jgmenu-lx

OBJS =  x11-ui.o config.o util.o geometry.o isprog.o sbuf.o icon-find.o \
        icon.o xpm-loader.o xdgdirs.o xdgapps.o xsettings.o xsettings-helper.o \
	filter.o compat.o hashmap.o lockfile.o argv-buf.o t2conf.o t2env.o \
	unix_sockets.o bl.o cache.o back.o terminal.o restart.o theme.o \
	gtkconf.o font.o args.o widgets.o pm.o socket.o

LIB_H = $(shell find . -name '*.h' -print)

JGMENU_LIB = libjgmenu.a

all: $(PROGS)
	@echo ""
	@echo "Warning: The CLI has changed. Please read release notes."

$(PROGS): % : $(OBJS) %.o
	$(QUIET_LINK)$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c $(LIB_H)
	$(QUIET_CC)$(CC) $(CFLAGS) -c $*.c


install: $(PROGS)
	@install -d $(DESTDIR)$(bindir)
	@install -m755 jgmenu jgmenu_run $(DESTDIR)$(bindir)
	@install -d $(DESTDIR)$(libexecdir)
	@install -m755 $(PROGS) $(SCRIPTS_SHELL) $(DESTDIR)$(libexecdir)
	@install -m755 $(SCRIPTS_PYTHON) $(DESTDIR)$(libexecdir)
	@install -m644 ./noncore/init/* $(DESTDIR)$(libexecdir)
	@./scripts/set-exec-path.sh $(DESTDIR)$(bindir)/jgmenu_run $(libexecdir)
	@./scripts/set-exec-path.sh $(DESTDIR)$(libexecdir)/jgmenu_run $(libexecdir)
ifdef PYTHON3_POLYGLOT
	@./scripts/python3-polyglot.sh $(DESTDIR)$(libexecdir) $(SCRIPTS_PYTHON)
else
	@type python3 >/dev/null 2>&1 || printf "%s\n" "warning: python3 not \
	found. Suggest defining PYTHON3_POLYGLOT"
endif
	@$(MAKE) --no-print-directory -C docs/manual/ install
	@./scripts/create_desktop_file.sh $(DESTDIR)$(prefix)

clean:
	@$(RM) $(PROGS) *.o *.a
	@$(MAKE) --no-print-directory -C tests/ clean
	@$(MAKE) --no-print-directory -C tests/helper/ clean

test: $(OBJS)
	@$(RM) $(JGMENU_LIB)
	@$(MAKE) --no-print-directory -C tests/helper/ clean
	@echo '     AR    libjgmenu.a';$(AR) rcs $(JGMENU_LIB) $(OBJS)
	@$(MAKE) --no-print-directory -C tests/helper/ all
	@$(MAKE) --no-print-directory -C tests/ all

ex:
	@$(MAKE) --no-print-directory -C examples/ all

check:
	@./scripts/checkpatch-wrapper.sh *.c
	@./scripts/checkpatch-wrapper.sh *.h
