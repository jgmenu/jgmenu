#
# Define ASAN=1 to enable AddressSanitizer
#
# Define VERBOSE=1 for a more verbose compilation
#
# Define PYTHON3_POLYGLOT=1 if '#!/usr/bin/env python3' is not going to work
# on your system.
#

VER      = $(shell ./scripts/version-gen.sh)
CC       = gcc
MAKE     = make
RM       = rm -f

prefix     = $(HOME)
bindir     = $(prefix)/bin
libexecdir = $(prefix)/lib/jgmenu

CFLAGS   = -g -Wall -Os -std=gnu89
CFLAGS  += -DVERSION='"$(VER)"'
CFLAGS  += -DXINERAMA
CFLAGS  += `pkg-config cairo pango pangocairo librsvg-2.0 --cflags`
CFLAGS  += `xml2-config --cflags`

LIBS  = `pkg-config x11 xinerama cairo pango pangocairo librsvg-2.0 --libs`
LIBS += `xml2-config --libs`
LIBS += -pthread

LDFLAGS  = $(LIBS)

# Allow user to override build settings without making tree dirty
-include config.mk

ifdef ASAN
ASAN_FLAGS = -O0 -fsanitize=address -fno-common -fno-omit-frame-pointer -rdynamic
CFLAGS    += $(ASAN_FLAGS)
LDFLAGS   += $(ASAN_FLAGS) -fuse-ld=gold
endif

ifndef VERBOSE
QUIET_CC   = @echo '     CC    '$@;
QUIET_LINK = @echo '     LINK  '$@;
endif

SCRIPTS_SHELL  = jgmenu_run jgmenu-cache.sh jgmenu-pmenu.sh \
		 jgmenu-csv.sh jgmenu-xdg.sh jgmenu-config.sh \
		 jgmenu-init.sh

SCRIPTS_PYTHON = jgmenu-parse-pmenu.py jgmenu-unity-hack.py

PROGS	 = jgmenu jgmenu-parse-xdg jgmenu-icon-find jgmenu-xsettings

LIB_H = $(shell find . -name '*.h' -print)

OBJS =  x11-ui.o config.o util.o geometry.o isprog.o sbuf.o icon-find.o \
        icon.o xpm-loader.o xdgdirs.o xdgapps.o xsettings.o xsettings-helper.o \
	config-xs.o filter.o compat.o

all: $(PROGS)
	@echo 'WARNING: The Makefile has been refactored since v0.4.4'
	@echo 'Please remove all files in $$prefix/bin before doing a'
	@echo "'make install'"
	@echo ""

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
	@$(RM) $(PROGS) *.o

test:
	@$(MAKE) --no-print-directory -C tests/ all

ex:
	@$(MAKE) --no-print-directory -C examples/ all

check:
	@./scripts/checkpatch-wrapper.sh *.c
	@./scripts/checkpatch-wrapper.sh *.h
