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

ifdef ASAN
ASAN_FLAGS = -O0 -fsanitize=address -fno-common -fno-omit-frame-pointer -rdynamic
CFLAGS    += $(ASAN_FLAGS)
LDFLAGS   += $(ASAN_FLAGS) -fuse-ld=gold
endif

ifndef VERBOSE
QUIET_CC   = @echo '     CC    '$@;
QUIET_LINK = @echo '     LINK  '$@;
endif

MAKEFLAGS += --no-print-directory
