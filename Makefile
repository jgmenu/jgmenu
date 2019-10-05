#
# Define ASAN=1 to enable AddressSanitizer
#
# Define VERBOSE=1 for a more verbose compilation
#
# Define NO_LX=1 if you do not want to build jgmenu-lx (which requires
# libmenu-cache >=v1.1)
#
# Define CONTRIB_DIRS to include any contrib/ packages you wish to include
# The following are supported: CONTRIB_DIRS="xfce4-panel gtktheme"
#

VER      = $(shell ./scripts/version-gen.sh)

# Allow user to override build settings without making tree dirty
-include config.mk

RM       = rm -f

prefix    ?= /usr/local
bindir     = $(prefix)/bin
libexecdir = $(prefix)/lib/jgmenu

ifeq ($(prefix),$(HOME))
datarootdir= $(prefix)/.local/share
else
datarootdir= $(prefix)/share
endif

CFLAGS  += -g -Wall -Os -std=gnu99
CFLAGS  += -Wextra -Wdeclaration-after-statement -Wno-format-zero-length \
	   -Wold-style-definition -Woverflow -Wpointer-arith \
	   -Wstrict-prototypes -Wunused -Wvla -Wunused-result
CFLAGS  += -Wno-unused-parameter
CFLAGS  += -DVERSION='"$(VER)"'

jgmenu:     CFLAGS  += `pkg-config cairo pango pangocairo librsvg-2.0 --cflags`
jgmenu-ob:  CFLAGS  += `xml2-config --cflags`
jgmenu-lx:  CFLAGS  += `pkg-config --cflags glib-2.0 libmenu-cache`
jgmenu-obtheme: CFLAGS  += `xml2-config --cflags`

jgmenu:     LIBS += `pkg-config x11 xrandr cairo pango pangocairo librsvg-2.0 --libs`
jgmenu:     LIBS += -pthread -lpng
jgmenu-ob:  LIBS += `xml2-config --libs`
jgmenu-lx:  LIBS += `pkg-config --libs glib-2.0 libmenu-cache`
jgmenu-obtheme: LIBS += `xml2-config --libs`

LDFLAGS += $(LIBS)

ifdef ASAN
ASAN_FLAGS = -O0 -fsanitize=address -fno-common -fno-omit-frame-pointer -rdynamic
CFLAGS    += $(ASAN_FLAGS)
LDFLAGS   += $(ASAN_FLAGS) -fuse-ld=gold
endif

ifndef VERBOSE
QUIET_CC   = @echo '     CC    '$@;
QUIET_LINK = @echo '     LINK  '$@;
endif

DEPDIR := .d
$(shell mkdir -p $(DEPDIR) >/dev/null)
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td

SCRIPTS_LIBEXEC = src/jgmenu-init.sh \
                  src/jgmenu-pmenu.py \
                  src/jgmenu-unity-hack.py \
                  src/jgmenu-themes.sh \
                  src/jgmenu-hide-app.sh

PROGS_LIBEXEC   = jgmenu-ob jgmenu-socket jgmenu-i18n jgmenu-greeneye \
                  jgmenu-obtheme jgmenu-apps jgmenu-config

# wrap in ifneq to ensure we respect user defined NO_LX=1
ifneq ($(NO_LX),1)
NO_LX := $(shell pkg-config "libmenu-cache >= 1.1.0" "glib-2.0" || echo "1")
endif
ifneq ($(NO_LX),1)
PROGS_LIBEXEC += jgmenu-lx
endif

PROGS           = jgmenu $(PROGS_LIBEXEC)

all: checkdeps $(PROGS)
	@for dir in $(CONTRIB_DIRS); do			\
		$(MAKE) -C contrib/$$dir || exit 1;	\
	done

jgmenu: jgmenu.o x11-ui.o config.o util.o geometry.o isprog.o sbuf.o \
	icon-find.o icon.o xpm-loader.o xdgdirs.o xsettings.o \
	xsettings-helper.o filter.o compat.o lockfile.o argv-buf.o t2conf.o \
	ipc.o unix_sockets.o bl.o cache.o back.o terminal.o restart.o \
	theme.o gtkconf.o font.o args.o widgets.o pm.o socket.o workarea.o \
	charset.o watch.o spawn.o
jgmenu-ob: jgmenu-ob.o util.o sbuf.o i18n.o hashmap.o
jgmenu-socket: jgmenu-socket.o util.o sbuf.o unix_sockets.o socket.o
ifneq ($(NO_LX),1)
jgmenu-lx: jgmenu-lx.o util.o sbuf.o xdgdirs.o argv-buf.o back.o fmt.o
endif
jgmenu-i18n: jgmenu-i18n.o i18n.o hashmap.o util.o sbuf.o
jgmenu-greeneye: jgmenu-greeneye.o compat.o util.o sbuf.o
jgmenu-apps: jgmenu-apps.o compat.o util.o sbuf.o desktop.o charset.o \
             xdgdirs.o argv-buf.o
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

install: checkdeps $(PROGS)
	@install -d $(DESTDIR)$(bindir)
	@install -m755 jgmenu src/jgmenu_run $(DESTDIR)$(bindir)
	@install -d $(DESTDIR)$(libexecdir)
	@install -m755 $(PROGS_LIBEXEC) $(SCRIPTS_LIBEXEC) $(DESTDIR)$(libexecdir)
	@./scripts/set-exec-path.sh $(DESTDIR)$(bindir)/jgmenu_run $(libexecdir)
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
	@$(RM) $(PROGS) *.o *.a $(DEPDIR)/*.d checkdeps
	@$(RM) -r .d/
	@$(MAKE) --no-print-directory -C tests/ clean
	@$(MAKE) --no-print-directory -C tests/helper/ clean
	@for dir in $(CONTRIB_DIRS); do				\
		$(MAKE) -C contrib/$$dir clean || exit 1;	\
	done

test:
	@$(MAKE) --no-print-directory -C tests/helper/ all
	@$(MAKE) --no-print-directory -C tests/ all

prove:
	@$(MAKE) --no-print-directory -C tests/helper/ all
	@$(MAKE) --no-print-directory -C tests/ prove

ex:
	@$(MAKE) --no-print-directory -C examples/ all

check:
	@./scripts/checkpatch-wrapper.sh src/*.c
	@./scripts/checkpatch-wrapper.sh src/*.h

REQUIRED_BINS := pkg-config xml2-config
REQUIRED_LIBS := x11 xrandr cairo pango pangocairo librsvg-2.0
checkdeps:
	@echo '     CHECK build dependencies'
	@for b in $(REQUIRED_BINS); do \
                type $${b} >/dev/null 2>&1 || echo "warn: require ($${b})"; \
        done
	@for l in $(REQUIRED_LIBS); do \
                pkg-config $${l} || echo "fatal: require ($${l})"; \
        done
	@if ! pkg-config "libmenu-cache >= 1.1.0" "glib-2.0"; then \
                echo "      - Cannot install lx module as build dependencies are missing"; \
	        echo "        libmenu-cache >= 1.1.0 and glib-2.0 are needed"; \
	        echo "        This will not prevent you from running jgmenu"; \
	fi
	@touch checkdeps

print-%:
	@echo '$*=$($*)'

SRCS = $(patsubst ./%,%,$(shell find ./src -maxdepth 1 -name '*.c' -print))
include $(wildcard $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRCS))))

.PHONY: all install uninstall clean test prove ex check checkdeps
