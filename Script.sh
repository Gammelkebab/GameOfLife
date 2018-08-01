#!/bin/bash -login
#PBS -N GameOfLife
#PBS -j oe
#PBS -m ae
#PBS -l nodes=4:ppn=4
#PBS -l walltime=0:00:30
#PBS -l mem=256mb
#PBS -W x=PARTITION:lena

cd $BIGWORK/GameOfLife

module load intel

make clean
make

lfs setstripe --count -1 images

mpirun ./main
