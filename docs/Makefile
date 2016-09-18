markdown_variant = --from=markdown_strict+backtick_code_blocks+pipe_tables
css_template     = jg.css

all:
	@echo "    PANDOC    README.html"
	@pandoc -s $(markdown_variant) --css=$(css_template) \
		-o ../README.html ../README.md
