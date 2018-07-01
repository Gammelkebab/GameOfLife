#!/bin/bash -login
#PBS -N MPI
#PBS -j oe
#PBS -m ae
#PBS -l nodes=1:ppn=1
#PBS -l walltime=0:01:00
#PBS -l mem=1gb
#PBS -W x=PARTITION:taurus

cd $HOME/GameOfLifeSlow

module load foss

make clean_full
make

mpirun -np 1 ./main > log.txt
./create_video.sh
