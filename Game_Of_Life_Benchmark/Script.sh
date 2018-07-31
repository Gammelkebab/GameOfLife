#!/bin/bash -login
#PBS -N Benchmark_GOL
#PBS -j oe
#PBS -m ae
#PBS -l nodes=1:ppn=1
#PBS -l walltime=00:01:00
#PBS -l mem=512mb
#PBS -W x=PARTITION:lena

module load intel

cd $BIGWORK/GameOfLife/Game_Of_Life_Benchmark

make

./main
