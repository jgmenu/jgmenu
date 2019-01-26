ASAN_FLAGS = -O0 -fsanitize=address -fno-common -fno-omit-frame-pointer -rdynamic \
	     -fuse-ld=gold

all:
	./t1011-hashmap.sh
	./t1012-argv-buf.sh
	./t1013-config-amend.sh
	./t1100-ob.sh
	./t1101-i18n.sh


# string buffer tests
t0002:
	gcc -o t0002-sbuf t0002-sbuf.c ../src/sbuf.c ../src/util.c
	./t0002-sbuf

# icon-lookup tests
t0003:
	gcc -o t0003-icon-find t0003-icon-find.c ../src/icon-find.c ../src/xdgdirs.c \
		../src/sbuf.c ../src/util.c
	./t0003-icon-find
	@echo ""

t0005:
	gcc -o t0005-xdgdirs t0005-xdgdirs.c ../src/xdgdirs.c ../src/sbuf.c
	./t0005-xdgdirs

t0007:
	gcc -o t0007-xdgapps t0007-xdgapps.c ../src/xdgapps.c ../src/sbuf.c ../src/util.c \
		../src/xdgdirs.c $(ASAN_FLAGS)
	./t0007-xdgapps

t0008: t0008-xpm.c ../src/xpm-loader.c ../src/xpm-loader.h
	gcc -o t0008-xpm t0008-xpm.c ../src/xpm-loader.c $(ASAN_FLAGS) `pkg-config cairo --cflags --libs`
	./t0008-xpm t0008/*.xpm

t9000:
	@./t9000-icon-load.sh
	@./t9000-icon-load.sh
	@./t9000-icon-load.sh

clean:
	@$(RM) actual expect
