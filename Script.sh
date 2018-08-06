#!/bin/bash -login
#PBS -N GOL
#PBS -j oe
#PBS -m ae
#PBS -l nodes=4:ppn=4
#PBS -l walltime=0:01:00
#PBS -l mem=256mb
#PBS -W x=PARTITION:lena

cd $BIGWORK/GameOfLife

module load intel

make clean
make
mpirun ./main
