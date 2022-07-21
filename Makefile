#
# Define ASAN=1 to enable AddressSanitizer
#
# Define VERBOSE=1 for a more verbose compilation
#
# Define CONTRIB_DIRS to include any contrib/ packages you wish to include
# The following are supported: CONTRIB_DIRS="xfce4-panel gtktheme lx"
#

VER      = $(shell ./scripts/version-gen.sh)

-include config.mk
include Makefile.inc

jgmenu:         CFLAGS += `pkg-config cairo pango pangocairo librsvg-2.0 --cflags`
jgmenu-ob:      CFLAGS += `xml2-config --cflags`
jgmenu-obtheme: CFLAGS += `xml2-config --cflags`
jgmenu-config:  CFLAGS += `pkg-config --cflags glib-2.0`
jgmenu-apps:    CFLAGS += `pkg-config --cflags glib-2.0`

jgmenu:         LIBS   += `pkg-config x11 xrandr cairo pango pangocairo librsvg-2.0 --libs`
jgmenu:         LIBS   += -pthread -lpng
jgmenu-ob:      LIBS   += `xml2-config --libs`
jgmenu-obtheme: LIBS   += `xml2-config --libs`
jgmenu-config:  LIBS   += `pkg-config --libs glib-2.0`
jgmenu-apps:    LIBS   += `pkg-config --libs glib-2.0`

LDFLAGS += $(LIBS)

DEPDIR := .d
$(shell mkdir -p $(DEPDIR) >/dev/null)
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td

SCRIPTS_LIBEXEC = src/jgmenu-init.sh \
                  src/jgmenu-unity-hack.py \
                  src/jgmenu-themes.sh \
                  src/jgmenu-hide-app.sh

PROGS_LIBEXEC   = jgmenu-ob jgmenu-socket jgmenu-i18n jgmenu-greeneye \
                  jgmenu-obtheme jgmenu-apps jgmenu-config

PROGS           = jgmenu $(PROGS_LIBEXEC)

all: config_mk $(PROGS)
	@for dir in $(CONTRIB_DIRS); do \
		$(MAKE) -C contrib/$$dir || exit 1; \
	done

config_mk:
	@if test ! -e config.mk; then \
		echo "fatal: you have not run ./configure"; \
		exit 1; \
	fi
	@:

jgmenu: jgmenu.o x11-ui.o config.o util.o geometry.o isprog.o sbuf.o \
	icon-find.o icon.o xpm-loader.o xdgdirs.o xsettings.o \
	xsettings-helper.o filter.o compat.o lockfile.o argv-buf.o t2conf.o \
	ipc.o unix_sockets.o bl.o cache.o back.o terminal.o restart.o \
	theme.o gtkconf.o font.o args.o widgets.o pm.o socket.o workarea.o \
	charset.o hooks.o spawn.o
jgmenu-ob: jgmenu-ob.o util.o sbuf.o i18n.o hashmap.o
jgmenu-socket: jgmenu-socket.o util.o sbuf.o unix_sockets.o socket.o compat.o
jgmenu-i18n: jgmenu-i18n.o i18n.o hashmap.o util.o sbuf.o
jgmenu-greeneye: jgmenu-greeneye.o compat.o util.o sbuf.o
jgmenu-apps: jgmenu-apps.o compat.o util.o sbuf.o desktop.o charset.o \
             xdgdirs.o argv-buf.o dirs.o lang.o fmt.o i18n.o hashmap.o isprog.o
jgmenu-obtheme: jgmenu-obtheme.o util.o sbuf.o compat.o set.o
jgmenu-config: jgmenu-config.o util.o sbuf.o compat.o set.o spawn.o

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
	@install -d $(DESTDIR)$(libexecdir)/jgmenu
	@install -m755 $(PROGS_LIBEXEC) $(SCRIPTS_LIBEXEC) $(DESTDIR)$(libexecdir)/jgmenu
	@./scripts/set-exec-path.sh $(DESTDIR)$(bindir)/jgmenu_run $(libexecdir)/jgmenu
	@$(MAKE) --no-print-directory -C docs/manual/ prefix=$(prefix) install
	@install -d $(DESTDIR)$(datarootdir)/icons/hicolor/scalable/apps/
	@install -d $(DESTDIR)$(datarootdir)/applications/
	@install -m644 ./data/jgmenu.svg $(DESTDIR)$(datarootdir)/icons/hicolor/scalable/apps/
	@install -m644 ./data/jgmenu.desktop $(DESTDIR)$(datarootdir)/applications/
	@for dir in $(CONTRIB_DIRS); do				\
		$(MAKE) -C contrib/$$dir install || exit 1;	\
	done

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
	@for dir in $(CONTRIB_DIRS); do				\
		$(MAKE) -C contrib/$$dir uninstall || exit 1;	\
	done

clean:
	@$(RM) $(PROGS) *.o *.a $(DEPDIR)/*.d
	@$(RM) -r .d/
	@$(MAKE) --no-print-directory -C tests/ clean
	@$(MAKE) --no-print-directory -C tests/helper/ clean
	@for dir in $(CONTRIB_DIRS); do				\
		$(MAKE) -C contrib/$$dir clean || exit 1;	\
	done

distclean: clean
	@$(RM) config.mk clang-format-tmpfile

test:
	@$(MAKE) --no-print-directory -C tests/helper/ all
	@$(MAKE) --no-print-directory -C tests/ all

prove:
	@$(MAKE) --no-print-directory -C tests/helper/ all
	@$(MAKE) --no-print-directory -C tests/ prove

ex:
	@$(MAKE) --no-print-directory -C examples/ all

check:
	@./scripts/check src/*.sh src/*.c src/*.h

print-%:
	@echo '$*=$($*)'

SRCS = $(patsubst ./%,%,$(shell find ./src -maxdepth 1 -name '*.c' -print))
include $(wildcard $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRCS))))

.PHONY: all install uninstall clean distclean test prove ex check
