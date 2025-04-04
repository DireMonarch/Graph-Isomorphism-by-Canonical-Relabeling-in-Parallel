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



GCC	=	gcc
CC      =       /opt/ohpc/pub/mpi/openmpi3-gnu7/3.1.0/bin/mpicc
CCLINK  =       /opt/ohpc/pub/mpi/openmpi3-gnu7/3.1.0/bin/mpicc
SHELL   =       /bin/sh

all: main mpi


main: main.c inc/p_gtools.o lib/util.o lib/p_util.o lib/partition.o pcanon.o lib/badstack.o lib/path.o lib/automorphismgroup.o
	# $(GCC) main.c 
	$(GCC) main.c lib/p_gtools.o lib/p_util.o lib/util.o lib/partition.o lib/pcanon.o lib/badstack.o lib/path.o lib/automorphismgroup.o

mpi: main.c lib/p_gtools.o lib/util.o lib/p_util.o lib/partition.o mpipcanon.o lib/badstack.o lib/path.o lib/automorphismgroup.o lib/mpi_routines.o
	$(CC) -lm -o mpi -DMPI main.c lib/p_gtools.o lib/p_util.o lib/util.o lib/partition.o lib/mpipcanon.o lib/badstack.o lib/path.o lib/automorphismgroup.o lib/mpi_routines.o

mpipcanon.o:
	$(CC) -c  -lm  -DMPI inc/pcanon.c -o lib/mpipcanon.o

pcanon.o: 
	$(GCC) -c inc/pcanon.c -o lib/pcanon.o

lib/mpi_routines.o: inc/mpi_routines.c inc/mpi_routines.h
	$(CC) -c -lm -DMPI inc/mpi_routines.c -o lib/mpi_routines.o

lib/p_gtools.o: inc/p_gtools.c inc/p_gtools.h
	$(GCC) -c inc/p_gtools.c -o lib/p_gtools.o

lib/p_util.o: inc/p_util.c inc/p_util.h	
	$(GCC) -c inc/p_util.c  -o lib/p_util.o

lib/util.o: inc/util.c inc/util.h	
	$(GCC) -c inc/util.c  -o lib/util.o	

lib/partition.o: inc/partition.c inc/partition.h	
	$(GCC) -c inc/partition.c  -o lib/partition.o	

lib/badstack.o: inc/badstack.c inc/badstack.h	
	$(GCC) -c inc/badstack.c  -o lib/badstack.o

lib/path.o: inc/path.c inc/path.h	
	$(GCC) -c inc/path.c  -o lib/path.o	

lib/automorphismgroup.o: inc/automorphismgroup.c inc/automorphismgroup.h	
	$(GCC) -c inc/automorphismgroup.c  -o lib/automorphismgroup.o		

clean:
	rm a.out lib/*.o mpi