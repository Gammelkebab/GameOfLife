#!/bin/bash -login
#PBS -N MPI
#PBS -j oe
#PBS -m ae
#PBS -l nodes=4:ppn=1
#PBS -l walltime=0:00:30
#PBS -l mem=1gb
#PBS -W x=PARTITION:tane

cd $BIGWORK/GameOfLife

module load intel

make full

lfs setstripe –c 4 images

mpirun ./main

./create_video.sh
