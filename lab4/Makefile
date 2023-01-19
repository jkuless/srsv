all: server generator
server: server.c lab4.h
	-gcc -g -Wall -Wextra -lpthread server.c -o server
generator: generator.c lab4.h
	-gcc -g -Wall -Wextra -lpthread generator.c -o generator
clean:
	-rm -f generator server *.o