INSTALL = install
variant = --from=markdown_strict+backtick_code_blocks+pipe_tables
css     = jg.css

man_pages  = jgmenu.1 jgmenu_run.1

# High level commands
man_pages += jgmenu-cache.1 jgmenu-pmenu.1 jgmenu-xdg.1 jgmenu-csv.1

prefix ?= $(HOME)
mandir ?= $(prefix)/share/man
man1dir = $(mandir)/man1

all: $(man_pages) readme

$(man_pages): % : %.md
	@echo "    PANDOC    " $^
	@pandoc -s -t man $^ -o $@
	@pandoc -s $(variant) --css=$(css) -o $@.html $^

readme: ../../README.md
	@echo "    PANDOC    ../../README.md" 
	@pandoc -s $(variant) --css=$(css) -o ../../README.html ../../README.md

install:
	@$(INSTALL) -d -m 755 $(DESTDIR)$(man1dir)
	@$(INSTALL) -m 644 $(man_pages) $(DESTDIR)$(man1dir)

clean:
	rm -f $(man_pages) *.html ../../README.html
