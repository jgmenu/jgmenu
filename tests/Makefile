all: t0002 t0003 t0005

# string buffer tests
t0002:
	gcc -o t0002-sbuf t0002-sbuf.c ../sbuf.c
	./t0002-sbuf

# icon-lookup tests
t0003:
	gcc -o t0003-icon-find t0003-icon-find.c ../icon-find.c ../xdgdirs.c ../sbuf.c ../util.c
	./t0003-icon-find
	@echo ""

t0005:
	gcc -o t0005-xdgdirs t0005-xdgdirs.c ../xdgdirs.c ../sbuf.c
	./t0005-xdgdirs
