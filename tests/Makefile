ASAN_FLAGS = -O0 -fsanitize=address -fno-common -fno-omit-frame-pointer -rdynamic -fuse-ld=gold

all: t1011
#all: t0002 t0003 t0005 t0007 t0008

# string buffer tests
t0002:
	gcc -o t0002-sbuf t0002-sbuf.c ../sbuf.c ../util.c
	./t0002-sbuf

# icon-lookup tests
t0003:
	gcc -o t0003-icon-find t0003-icon-find.c ../icon-find.c ../xdgdirs.c \
		../sbuf.c ../util.c
	./t0003-icon-find
	@echo ""

t0005:
	gcc -o t0005-xdgdirs t0005-xdgdirs.c ../xdgdirs.c ../sbuf.c
	./t0005-xdgdirs

t0007:
	gcc -o t0007-xdgapps t0007-xdgapps.c ../xdgapps.c ../sbuf.c ../util.c \
		../xdgdirs.c $(ASAN_FLAGS)
	./t0007-xdgapps

t0008: t0008-xpm.c ../xpm-loader.c ../xpm-loader.h
	gcc -o t0008-xpm t0008-xpm.c ../xpm-loader.c $(ASAN_FLAGS) `pkg-config cairo --cflags --libs`
	./t0008-xpm t0008/*.xpm

t1011:
	./t1011-hashmap.sh

clean:
	@$(RM) actual expect
