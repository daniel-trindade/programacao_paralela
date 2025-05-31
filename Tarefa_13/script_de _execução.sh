#!/bin/bash 
#SBATCH --partition=intel-128
#SBATCH --job-name=afinidade-160
#SBATCH --time=0-10:30
#SBATCH --cpus-per-task=32

for BIND in true false spread master close
do
    export OMP_PROC_BIND=$BIND
    echo "Testando com OMP_PROC_BIND=$BIND"
    ./test
done