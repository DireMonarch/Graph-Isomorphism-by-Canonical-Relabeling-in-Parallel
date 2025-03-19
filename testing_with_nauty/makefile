CC		=	gcc


all: main

main: main.c mpiinclude
	# $(CC) main.c ../../nauty2_8_9/gtools.o 
	$(CC) main.c mpinauty.o ../../nauty2_8_9/nauty.a

mpiinclude: mpinauty.c
	$(CC) -c mpinauty.c 

run: all
	./a.out

clean:
	rm a.out mpinauty.o