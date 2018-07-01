#!/bin/bash -login
#PBS -N MPI
#PBS -j oe
#PBS -m ae
#PBS -l nodes=1:ppn=4
#PBS -l walltime=0:01:00
#PBS -l mem=1gb
#PBS -W x=PARTITION:lena

cd $BIGWORK/GameOfLife

module load foss

make full

mpirun ./main > log2.txt

./create_video.sh
