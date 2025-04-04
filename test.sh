#! /bin/bash

for item in samples/*g6; do
    echo "$item"

    mpirun -n 32 ./mpi $item
    ./a.out $item
done
