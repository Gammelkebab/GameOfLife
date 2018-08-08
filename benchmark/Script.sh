#!/bin/bash -login
#PBS -N GOL_Benchmark
#PBS -j oe
#PBS -m ae
#PBS -l nodes=1:ppn=1
#PBS -l walltime=0:01:00
#PBS -l mem=256mb
#PBS -W x=PARTITION:lena

cd $BIGWORK/GameOfLifeBenchmark

module load intel

make clean
make
./main
