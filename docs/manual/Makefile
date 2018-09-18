INSTALL = install
variant = --from=markdown_strict+backtick_code_blocks+pipe_tables+pandoc_title_block
css     = jg.css

man1pages  = jgmenu.1 jgmenu_run.1

man1pages += jgmenu-pmenu.1 jgmenu-xdg.1 jgmenu-lx.1 jgmenu-ob.1 \
             jgmenu-config.1

man7pages  = jgmenututorial.7

manpages   = $(man1pages) $(man7pages)

readmefile = ../../README.html

prefix ?= /usr/local
mandir ?= $(prefix)/share/man
man1dir = $(mandir)/man1
man7dir = $(mandir)/man7

all: $(manpages) $(readmefile)

$(manpages): % : %.md
	@echo "    PANDOC    " $^
	@pandoc -s -t man $^ -o $@
	@pandoc -s $(variant) --css=$(css) -o $@.html $^

$(readmefile): ../../README.md
	@echo "    PANDOC    ../../README.md"
	@pandoc -s $(variant) --css=$(css) -o ../../README.html ../../README.md

install:
	@$(INSTALL) -d -m 755 $(DESTDIR)$(man1dir)
	@$(INSTALL) -d -m 755 $(DESTDIR)$(man7dir)
	@$(INSTALL) -m 644 $(man1pages) $(DESTDIR)$(man1dir)
	@$(INSTALL) -m 644 $(man7pages) $(DESTDIR)$(man7dir)

clean:
	rm -f $(manpages) *.html ../../README.html
