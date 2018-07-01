#!/bin/bash -login
#PBS -N MPI
#PBS -j oe
#PBS -m ae
#PBS -l nodes=4:ppn=12
#PBS -l walltime=2:00:00
#PBS -l mem=1gb
#PBS -W x=PARTITION:tane

cd $BIGWORK/GameOfLife

module load foss

make full

mpirun -np 1 ./main > log1.txt
mpirun -np 4 ./main > log2.txt
mpirun -np 12 ./main > log3.txt
mpirun -np 48 ./main > log4.txt
