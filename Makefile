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

REQUIRED_BINS := pkg-config $(CC) xml2-config
$(foreach bin,$(REQUIRED_BINS), \
        $(if $(shell type $(bin) 2>/dev/null),, \
                $(error fatal: could not find '$(bin)')))

REQUIRED_LIBS := x11 xrandr cairo pango pangocairo librsvg-2.0
$(foreach lib,$(REQUIRED_LIBS), \
        $(if $(shell pkg-config $(lib) && echo 1),, \
                $(error fatal: could not find library '$(lib)')))

VER      = $(shell ./scripts/version-gen.sh)

# Allow user to override build settings without making tree dirty
-include config.mk

include ./Makefile.inc

DEPDIR := .d
$(shell mkdir -p $(DEPDIR) >/dev/null)
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td

SCRIPTS_SHELL  = src/jgmenu_run noncore/init/jgmenu-init.sh

FRAGMENTS      = noncore/init/jgmenu-init--prepend.sh \
                 noncore/init/jgmenu-init--append.sh \
                 noncore/init/jgmenu-init--bunsenlabs.sh \
                 noncore/init/jgmenurc.archlabs_1803 \
                 noncore/init/jgmenurc.bunsenlabs_hydrogen \
                 noncore/init/jgmenurc.bunsenlabs_helium \
                 noncore/init/jgmenu-init--neon.sh \
                 noncore/init/jgmenurc.neon \
                 noncore/config/jgmenurc

SCRIPTS_PYTHON = src/jgmenu-pmenu.py src/jgmenu-unity-hack.py \
                 noncore/config/jgmenu-config.py

PROGS	 = jgmenu jgmenu-ob jgmenu-socket jgmenu-i18n jgmenu-greeneye

# wrap in ifneq to ensure we respect user defined NO_LX=1
ifneq ($(NO_LX),1)
NO_LX := $(shell pkg-config "libmenu-cache >= 1.1.0" "glib-2.0" || echo "1")
endif
ifneq ($(NO_LX),1)
PROGS += jgmenu-lx
endif

objects = $(patsubst ./src/%.c,%.o,$(shell find ./src -maxdepth 1 -name '*.c' -print))
mains = $(patsubst src/%,%.o,$(PROGS))
ifneq ($(NO_LX),1)
OBJS = $(filter-out $(mains),$(objects))
else
OBJS = $(filter-out $(mains) jgmenu-lx.o,$(objects))
endif
SRCS = $(patsubst %.o,src/%.c,$(OBJS))
JGMENU_LIB = libjgmenu.a

all: $(PROGS)

jgmenu: jgmenu.o x11-ui.o config.o util.o geometry.o isprog.o sbuf.o \
	icon-find.o icon.o xpm-loader.o xdgdirs.o xsettings.o \
	xsettings-helper.o filter.o compat.o lockfile.o argv-buf.o t2conf.o \
	t2env.o unix_sockets.o bl.o cache.o back.o terminal.o restart.o \
	theme.o gtkconf.o font.o args.o widgets.o pm.o socket.o workarea.o \
	charset.o watch.o spawn.o
jgmenu-ob: jgmenu-ob.o util.o sbuf.o i18n.o hashmap.o
jgmenu-socket: jgmenu-socket.o util.o sbuf.o unix_sockets.o socket.o
ifneq ($(NO_LX),1)
jgmenu-lx: jgmenu-lx.o util.o sbuf.o xdgdirs.o argv-buf.o back.o fmt.o
endif
jgmenu-i18n: jgmenu-i18n.o i18n.o hashmap.o util.o sbuf.o
jgmenu-greeneye: jgmenu-greeneye.o

$(PROGS):
	$(QUIET_LINK)$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o : src/%.c
%.o : src/%.c $(DEPDIR)/%.d
	$(QUIET_CC)$(CC) $(DEPFLAGS) $(CFLAGS) -c $<
	@mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d && touch $@

$(DEPDIR)/%.d: ;
.PRECIOUS: $(DEPDIR)/%.d

install: $(PROGS)
	@install -d $(DESTDIR)$(bindir)
	@install -m755 jgmenu src/jgmenu_run $(DESTDIR)$(bindir)
	@install -d $(DESTDIR)$(libexecdir)
	@install -m755 $(PROGS) $(SCRIPTS_SHELL) $(DESTDIR)$(libexecdir)
	@install -m755 $(SCRIPTS_PYTHON) $(DESTDIR)$(libexecdir)
	@install -m644 $(FRAGMENTS) $(DESTDIR)$(libexecdir)
	@./scripts/set-exec-path.sh $(DESTDIR)$(bindir)/jgmenu_run $(libexecdir)
	@./scripts/set-exec-path.sh $(DESTDIR)$(libexecdir)/jgmenu_run $(libexecdir)
ifdef PYTHON3_POLYGLOT
	@./scripts/python3-polyglot.sh $(DESTDIR)$(libexecdir) $(notdir $(SCRIPTS_PYTHON))
else
	@type python3 >/dev/null 2>&1 || printf "%s\n" "warning: python3 not \
	found. Suggest defining PYTHON3_POLYGLOT"
endif
	@$(MAKE) --no-print-directory -C docs/manual/ prefix=$(prefix) install
	@install -d $(DESTDIR)$(datarootdir)/icons/hicolor/scalable/apps/
	@install -d $(DESTDIR)$(datarootdir)/applications/
	@install -m644 ./data/jgmenu.svg $(DESTDIR)$(datarootdir)/icons/hicolor/scalable/apps/
	@install -m644 ./data/jgmenu.desktop $(DESTDIR)$(datarootdir)/applications/
ifeq ($(NO_LX),1)
	@echo "info: lx module not included as libmenu-cache >=1.1.0 not found"
endif

# We are not brave enough to uninstall in /usr/, /usr/local/ etc
uninstall:
ifneq ($(prefix),$(HOME))
	@$(error uninstall only works if prefix=$(HOME))
endif
	@rm -f ~/bin/jgmenu
	@rm -f ~/bin/jgmenu_run
	@rm -rf ~/lib/jgmenu/
	@-rmdir ~/lib 2>/dev/null || true
	@rm -f ~/share/man/man1/jgmenu*
	@rm -f ~/share/man/man7/jgmenu*
	@-rmdir ~/share/man/man1 2>/dev/null || true
	@-rmdir ~/share/man/man7 ~/share/man ~/share 2>/dev/null || true
	@rm -f ~/.local/share/icons/hicolor/scalable/apps/jgmenu.svg
	@rm -f ~/.local/share/applications/jgmenu.desktop
	@-rmdir ~/.local/share/applications 2>/dev/null || true
	@-rmdir ~/.local/share/icons/hicolor/scalable/apps 2>/dev/null || true
	@-rmdir ~/.local/share/icons/hicolor/scalable 2>/dev/null || true
	@-rmdir ~/.local/share/icons/hicolor 2>/dev/null || true
	@-rmdir ~/.local/share/icons 2>/dev/null || true

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
	@./scripts/checkpatch-wrapper.sh src/*.c
	@./scripts/checkpatch-wrapper.sh src/*.h

print-%:
	@echo '$*=$($*)'

include $(wildcard $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRCS))))
