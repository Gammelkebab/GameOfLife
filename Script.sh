#!/bin/bash -login
#PBS -N GOL
#PBS -j oe
#PBS -m ae
#PBS -l nodes=4:ppn=16
#PBS -l walltime=0:00:05
#PBS -l mem=3gb
#PBS -W x=PARTITION:lena

cd $BIGWORK/GameOfLife

module load intel

make clean
make
mpirun ./bin/main
