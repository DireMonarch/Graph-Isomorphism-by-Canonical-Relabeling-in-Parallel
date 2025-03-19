CC		=	gcc


all: main

main: main.c p_gtools_o util_o p_util_o partition_o pcanon.o
	# $(CC) main.c 
	$(CC) main.c lib/p_gtools.o lib/p_util.o lib/util.o lib/partition.o lib/pcanon.o

pcanon.o: inc/pcanon.c inc/pcanon.h
	$(CC) -c inc/pcanon.c -o lib/pcanon.o

p_gtools_o: inc/p_gtools.c inc/p_gtools.h p_util_o
	$(CC) -c inc/p_gtools.c -o lib/p_gtools.o

p_util_o: inc/p_util.c inc/p_util.h	
	$(CC) -c inc/p_util.c  -o lib/p_util.o

util_o: inc/util.c inc/util.h	
	$(CC) -c inc/util.c  -o lib/util.o	

partition_o: inc/partition.c inc/partition.h	
	$(CC) -c inc/partition.c  -o lib/partition.o	

run: all
	./a.out

clean:
	rm a.out lib/*.o