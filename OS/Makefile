all: baboon

baboon: baboon.o
	gcc -O0 -Wall -Wextra -Werror -pedantic baboon.o -o baboon -lpthread -lrt

baboon.o: baboon.c
	gcc -Wall -Wextra -Werror -pedantic -c baboon.c -lpthread -lrt

clean: 
	rm -rf *.o 

test: short long

short:
	./baboon input.dat 1 

long:
	./baboon input.dat 8

run:
	./baboon input.dat 4