variant = --from=markdown_strict+backtick_code_blocks+pipe_tables
css     = ../jg.css

man_pages  = jgmenu.1 jgmenu_run.1
man_pages += jgmenu-cache.1

all: $(man_pages) readme

$(man_pages): % : %.md
	@echo "    PANDOC    " $^
	@pandoc -s -t man $^ -o $@
	@pandoc -s $(variant) --css=$(css) -o $@.html $^

readme:
	@echo "    PANDOC    ../../README.md" 
	@pandoc -s $(variant) --css=$(css) -o ../../README.html ../../README.md

clean:
	rm -f $(man_pages) *.html ../../README.html
