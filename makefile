# /**
#  * Copyright 2025 Jim Haslett
#  * 
#  * Licensed under the Apache License, Version 2.0 (the "License");
#  * you may not use this file except in compliance with the License.
#  * You may obtain a copy of the License at
#  * 
#  *     http://www.apache.org/licenses/LICENSE-2.0
#  * 
#  * Unless required by applicable law or agreed to in writing, software
#  * distributed under the License is distributed on an "AS IS" BASIS,
#  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  * See the License for the specific language governing permissions and
#  * limitations under the License.
#  */



CC		=	gcc


all: main

main: main.c p_gtools.o util.o p_util.o partition.o pcanon.o badstack.o path.o automorphismgroup.o
	# $(CC) main.c 
	$(CC) main.c lib/p_gtools.o lib/p_util.o lib/util.o lib/partition.o lib/pcanon.o lib/badstack.o lib/path.o lib/automorphismgroup.o

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

badstack.o: inc/badstack.c inc/badstack.h	
	$(CC) -c inc/badstack.c  -o lib/badstack.o

path.o: inc/path.c inc/path.h	
	$(CC) -c inc/path.c  -o lib/path.o	

automorphismgroup.o: inc/automorphismgroup.c inc/automorphismgroup.h	
	$(CC) -c inc/automorphismgroup.c  -o lib/automorphismgroup.o		

run: all
	./a.out

clean:
	rm a.out lib/*.o