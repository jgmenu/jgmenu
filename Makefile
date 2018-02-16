#
# Define ASAN=1 to enable AddressSanitizer
#
# Define VERBOSE=1 for a more verbose compilation
#
# Define PYTHON3_POLYGLOT=1 if '#!/usr/bin/env python3' is not going to work
# on your system.
#
# Define NO_LX=1 if you do not want to build jgmenu-lx (which requires
# libmenu-cache >=v1.1)
#

VER      = $(shell ./scripts/version-gen.sh)

# Allow user to override build settings without making tree dirty
-include config.mk

include ./Makefile.inc

DEPDIR := .d
$(shell mkdir -p $(DEPDIR) >/dev/null)
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td

SCRIPTS_SHELL  = jgmenu_run jgmenu-init.sh

SCRIPTS_PYTHON = jgmenu-pmenu.py jgmenu-unity-hack.py

PROGS	 = jgmenu jgmenu-xdg jgmenu-ob jgmenu-socket

NO_LX := $(shell pkg-config "libmenu-cache < 1.1.0" && echo "1")
ifneq ($(NO_LX),1)
PROGS += jgmenu-lx
endif

objects = $(patsubst ./%.c,%.o,$(shell find . -maxdepth 1 -name '*.c' -print))
mains = $(patsubst %,%.o,$(PROGS))
OBJS = $(filter-out $(mains),$(objects))
SRCS = $(patsubst %.o,%.c,$(OBJS))
JGMENU_LIB = libjgmenu.a

all: $(PROGS)

jgmenu: jgmenu.o x11-ui.o config.o util.o geometry.o isprog.o sbuf.o \
	icon-find.o icon.o xpm-loader.o xdgdirs.o xsettings.o \
	xsettings-helper.o filter.o compat.o lockfile.o argv-buf.o t2conf.o \
	t2env.o unix_sockets.o bl.o cache.o back.o terminal.o restart.o \
	theme.o gtkconf.o font.o args.o widgets.o pm.o socket.o workarea.o \
	charset.o
jgmenu-xdg: jgmenu-xdg.o util.o sbuf.o xdgdirs.o xdgapps.o argv-buf.o \
	charset.o
jgmenu-ob: jgmenu-ob.o util.o sbuf.o
jgmenu-socket: jgmenu-socket.o util.o sbuf.o unix_sockets.o socket.o
ifneq ($(NO_LX),1)
jgmenu-lx: jgmenu-lx.o util.o sbuf.o xdgdirs.o argv-buf.o back.o fmt.o
endif
$(PROGS):
	$(QUIET_LINK)$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o : %.c
%.o : %.c $(DEPDIR)/%.d
	$(QUIET_CC)$(CC) $(DEPFLAGS) $(CFLAGS) -c $<
	@mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d && touch $@

$(DEPDIR)/%.d: ;
.PRECIOUS: $(DEPDIR)/%.d

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
	@$(RM) $(PROGS) *.o *.a $(DEPDIR)/*.d
	@$(RM) -r .d/ 
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

print-%:
	@echo '$*=$($*)'

include $(wildcard $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRCS))))
