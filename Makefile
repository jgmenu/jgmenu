#
# Define ASAN=1 to enable AddressSanitizer
#
# Define VERBOSE=1 for a more verbose compilation
#

VER      = $(shell ./scripts/version-gen.sh)
CC       = gcc
MAKE     = make
RM       = rm -f

prefix   = $(HOME)
bindir   = $(prefix)/bin

CFLAGS   = -g -Wall -Os
CFLAGS  += -DVERSION='"$(VER)"'
CFLAGS  += -DXINERAMA
CFLAGS  += `pkg-config cairo pango pangocairo librsvg-2.0 --cflags`
CFLAGS  += `xml2-config --cflags`

LIBS  = `pkg-config x11 xinerama cairo pango pangocairo librsvg-2.0 --libs`
LIBS += `xml2-config --libs`
LIBS += -pthread

LDFLAGS  = $(LIBS)

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
		 jgmenu-csv.sh jgmenu-xdg.sh jgmenu-config.sh

SCRIPTS_PYTHON = jgmenu-parse-pmenu.py

# Python scripts with No file-Extension
SCRIPTS_PYTHON_NE = $(patsubst %.py,%,$(SCRIPTS_PYTHON))

PROGS	 = jgmenu jgmenu-parse-xdg jgmenu-icon-find jgmenu-xsettings

LIB_H = $(shell find . -name '*.h' -print)

OBJS =  x11-ui.o config.o util.o geometry.o isprog.o sbuf.o icon-find.o \
        icon.o xpm-loader.o xdgdirs.o xdgapps.o xsettings.o xsettings-helper.o \
	config-xs.o

all: $(PROGS)

$(PROGS): % : $(OBJS) %.o
	$(QUIET_LINK)$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c $(LIB_H)
	$(QUIET_CC)$(CC) $(CFLAGS) -c $*.c

$(SCRIPTS_PYTHON_NE): % : %.py
	$(shell ./scripts/set-python-path.sh $@)

install: $(PROGS) $(SCRIPTS_SHELL) $(SCRIPTS_PYTHON_NE)
	@install -d $(DESTDIR)$(bindir)
	@install -m755 $(PROGS) $(SCRIPTS_SHELL) $(DESTDIR)$(bindir)
	@install -m755 $(SCRIPTS_PYTHON_NE) $(DESTDIR)$(bindir)
	@$(MAKE) --no-print-directory -C docs/manual/ install

clean:
	@$(RM) $(PROGS) $(SCRIPTS_PYTHON_NE) *.o

test:
	@$(MAKE) --no-print-directory -C tests/ all

ex:
	@$(MAKE) --no-print-directory -C examples/ all

check:
	@./scripts/checkpatch-wrapper.sh *.{c,h}
