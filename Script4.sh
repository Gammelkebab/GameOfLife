#!/bin/bash -login
#PBS -N MPI
#PBS -j oe
#PBS -m ae
#PBS -l nodes=4:ppn=12
#PBS -l walltime=0:00:10
#PBS -l mem=1mb
#PBS -W x=PARTITION:tane

cd $BIGWORK/GameOfLife

module load foss

make full

mpirun -np 48 -npernode 12 -display-map ./main > log4.txt

./create_video.sh
