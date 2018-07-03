#!/bin/bash -login
#PBS -N MPI
#PBS -j oe
#PBS -m ae
#PBS -l nodes=4:ppn=4
#PBS -l walltime=0:00:30
#PBS -l mem=1gb
#PBS -W x=PARTITION:tane

cd $BIGWORK/GameOfLife

module load foss

make full

cat $PBS_NODEFILE

mpirun -np 16 --map-by node -display-map -display-allocation  ./main > log3.txt

./create_video.sh
