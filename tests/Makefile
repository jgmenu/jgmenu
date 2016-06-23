all: t0002 t0003

# string buffer tests
t0002:
	gcc -o t0002-sbuf t0002-sbuf.c ../sbuf.c
	./t0002-sbuf

# icon-lookup tests
t0003:
	gcc -o t0003-xdgicon t0003-xdgicon.c ../xdgicon.c ../sbuf.c
	./t0003-xdgicon
