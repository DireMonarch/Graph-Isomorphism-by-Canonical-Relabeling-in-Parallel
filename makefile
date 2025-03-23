CC		=	gcc


all: main

main: main.c p_gtools.o util.o p_util.o partition.o pcanon.o permutation.o
	# $(CC) main.c 
	$(CC) main.c lib/p_gtools.o lib/p_util.o lib/util.o lib/partition.o lib/pcanon.o lib/permutation.o

pcanon.o: inc/pcanon.c inc/pcanon.h
	$(CC) -c inc/pcanon.c -o lib/pcanon.o

p_gtools.o: inc/p_gtools.c inc/p_gtools.h p_util.o
	$(CC) -c inc/p_gtools.c -o lib/p_gtools.o

p_util.o: inc/p_util.c inc/p_util.h	
	$(CC) -c inc/p_util.c  -o lib/p_util.o

util.o: inc/util.c inc/util.h	
	$(CC) -c inc/util.c  -o lib/util.o	

partition.o: inc/partition.c inc/partition.h	
	$(CC) -c inc/partition.c  -o lib/partition.o	

permutation.o: inc/permutation.c inc/permutation.h	
	$(CC) -c inc/permutation.c  -o lib/permutation.o

run: all
	./a.out

clean:
	rm a.out lib/*.o