-include ../../config.mk
include ../../Makefile.inc

JGMENU_LIB = ../../libjgmenu.a

TEST_PROGS = $(basename $(shell find . -maxdepth 1 -name "*.c"))

all: $(TEST_PROGS) $(JGMENU_LIB)

$(TEST_PROGS): % : %.c
	$(QUIET_CC)$(CC) $(CFLAGS) -o $@ $^ $(JGMENU_LIB) -I../../src

clean :
	@$(RM) $(TEST_PROGS)

