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

INC		=		inc

serial: bin/serial
mpi: bin/mpi

bin/serial: main.c lib/p_gtools.o lib/util.o lib/p_util.o lib/partition.o pcanon.o lib/badstack.o lib/path.o lib/automorphismgroup.o
	$(GCC) -I $(INC) -o bin/serial main.c lib/p_gtools.o lib/p_util.o lib/util.o lib/partition.o lib/pcanon.o lib/badstack.o lib/path.o lib/automorphismgroup.o

bin/mpi: main.c lib/p_gtools.o lib/util.o lib/p_util.o lib/partition.o mpipcanon.o lib/badstack.o lib/path.o lib/automorphismgroup.o lib/mpi_routines.o
	$(CC) -lm -o bin/mpi -DMPI main.c lib/p_gtools.o lib/p_util.o lib/util.o lib/partition.o lib/mpipcanon.o lib/badstack.o lib/path.o lib/automorphismgroup.o lib/mpi_routines.o

mpipcanon.o:
	$(CC) -c  -lm  -DMPI src/pcanon.c -o lib/mpipcanon.o

pcanon.o: 
	$(GCC) -I $(INC) -c src/pcanon.c -o lib/pcanon.o

lib/mpi_routines.o: src/mpi_routines.c inc/mpi_routines.h
	$(CC) -c -lm -DMPI src/mpi_routines.c -o lib/mpi_routines.o

lib/p_gtools.o: src/p_gtools.c inc/p_gtools.h
	$(GCC) -I $(INC) -c src/p_gtools.c -o lib/p_gtools.o

lib/p_util.o: src/p_util.c inc/p_util.h	
	$(GCC) -I $(INC) -c src/p_util.c  -o lib/p_util.o

lib/util.o: src/util.c inc/util.h	
	$(GCC) -I $(INC) -c src/util.c  -o lib/util.o	

lib/partition.o: src/partition.c inc/partition.h	
	$(GCC) -I $(INC) -c src/partition.c  -o lib/partition.o	

lib/badstack.o: src/badstack.c inc/badstack.h	
	$(GCC) -I $(INC) -c src/badstack.c  -o lib/badstack.o

lib/path.o: src/path.c inc/path.h	
	$(GCC) -I $(INC) -c src/path.c  -o lib/path.o	

lib/automorphismgroup.o: src/automorphismgroup.c inc/automorphismgroup.h	
	$(GCC) -I $(INC) -c src/automorphismgroup.c  -o lib/automorphismgroup.o		

clean:
	rm lib/*.o bin/*