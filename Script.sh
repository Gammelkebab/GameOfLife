#!/bin/bash -login
#PBS -N MPI
#PBS -j oe
#PBS -m ae
#PBS -l nodes=1:ppn=2
#PBS -l walltime=2:00:00
#PBS -l mem=4gb

module load foss

make

mpirun ./main
