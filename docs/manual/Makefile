markdown_variant = --from=markdown_strict+backtick_code_blocks+pipe_tables
css_template     = jg.css

man_pages  = jgmenu.1 jgmenu_run.1
man_pages += jgmenu-cache.1

all: $(man_pages)

$(man_pages): % : %.md
	@echo "    PANDOC    " $^
	@pandoc -s -t man $^ -o $@


#	@pandoc -s $(markdown_variant) --css=$(css_template) \
#		-o jgmenu.1.html jgmenu.1.md
	
