#!/bin/bash -login
#PBS -N MPI
#PBS -j oe
#PBS -m ae
#PBS -l nodes=1:ppn=1
#PBS -l walltime=0:02:00
#PBS -l mem=1gb
#PBS -W x=PARTITION:lena

module load foss

make clean_full

mpirun ./main > log.txt
./create_video.sh