wish: shellLib.o main.o
	gcc shellLib.o main.o -o wish
main.o: main.c shellLib.h
	gcc main.c -c -std=c99 -pedantic -Wall -g
shellLib.o: shellLib.c shellLib.h
	gcc shellLib.c -c -std=c99 -pedantic -Wall -g