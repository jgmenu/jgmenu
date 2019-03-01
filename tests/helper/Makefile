CC         = @echo '     CC    '$@;gcc
CFLAGS     = -g -Wall
ifdef ASAN
ASAN_FLAGS = -O0 -fsanitize=address -fno-common -fno-omit-frame-pointer -rdynamic
CFLAGS    += $(ASAN_FLAGS)
LDFLAGS   += $(ASAN_FLAGS) -fuse-ld=gold
endif
RM         = rm -f

src = ../../src/
util = $(src)util.c $(src)sbuf.c

TEST_PROGS = filter-out test-argv-buf test-hashmap test-sbuf test-xpm

all: $(TEST_PROGS)

filter-out: filter-out.c
	$(CC) $(CFLAGS) -o $@ $^ -I$(src) $(LDFLAGS)

test-hashmap: test-hashmap.c $(src)hashmap.c $(util)
	$(CC) $(CFLAGS) -o $@ $^ -I$(src) $(LDFLAGS)

test-argv-buf: test-argv-buf.c $(src)argv-buf.c $(util)
	$(CC) $(CFLAGS) -o $@ $^ -I$(src) $(LDFLAGS)

test-sbuf: test-sbuf.c $(src)sbuf.c $(util)
	$(CC) $(CFLAGS) -o $@ $^ -I$(src) $(LDFLAGS)

test-xpm: test-xpm.c $(src)xpm-loader.c $(util)
	$(CC) $(CFLAGS) -o $@ $^ -I$(src) $(LDFLAGS) `pkg-config cairo --cflags --libs`

clean :
	@$(RM) $(TEST_PROGS)

