#!/bin/bash -login
#PBS -N GOL
#PBS -j oe
#PBS -m ae
#PBS -l nodes=4:ppn=16
#PBS -l walltime=0:00:05
#PBS -l mem=3gb
#PBS -W x=PARTITION:lena

cp -r GameOfLife/benchmark $BIGWORK/GOL_$(RND)
cd $BIGWORK/GOL_$(RND)

module load intel

make clean
make
mpirun ./bin/main
