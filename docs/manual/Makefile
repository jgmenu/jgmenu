man1pages  = jgmenu.1 jgmenu_run.1 jgmenu-pmenu.1 jgmenu-lx.1 jgmenu-ob.1 \
	     jgmenu-apps.1
man7pages  = jgmenututorial.7 jgmenuunicode.7
manpages   = $(man1pages) $(man7pages)

prefix ?= /usr/local
mandir ?= $(prefix)/share/man
man1dir = $(mandir)/man1
man7dir = $(mandir)/man7

all: $(manpages)

$(manpages): % : %.md
	@echo "    PANDOC    " $^
	@pandoc -s -t man $^ -o $@
	@sed -i 's/\\\\\\\\/\\\\/' $@

install:
	@install -d -m 755 $(DESTDIR)$(man1dir)
	@install -d -m 755 $(DESTDIR)$(man7dir)
	@install -m 644 $(man1pages) $(DESTDIR)$(man1dir)
	@install -m 644 $(man7pages) $(DESTDIR)$(man7dir)

clean:
	@rm -f $(manpages)
