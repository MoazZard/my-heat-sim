#!/bin/bash -l
#SBATCH -D ./
#SBATCH --export=ALL
#SBATCH -p course
#SBATCH -t 02:00:00
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=32

module load intel/oneapi-hpc-toolkit-2024.2
module load mpi/2021.13

echo "====================================="
echo "COMP328 Benchmark Started: $(date)"
echo "====================================="

# -----------------------------------
# Compile using Mkefile
# -----------------------------------
make gccnearly
make gcccomplete

# -----------------------------------
# Common Parameters
# -----------------------------------
N=512
ITER=1000000

# ===================================
# PART 1: STRONG SCALING (OpenMP)
# ===================================
# Strong scaling uses the SAME input file for every run.
# Per Section 3.4: input_K.dat where K is number of values.

K_STRONG=1
INPUT_STRONG="input_${K_STRONG}.dat"

echo ""
echo "===== OPENMP STRONG SCALING ====="

for T in 1 2 4 8 16 32
do
    export OMP_NUM_THREADS=$T
    # Format: output_K_N_maxIter
    OUTPUT_STRONG="output_${K_STRONG}_${N}_${ITER}_T${T}"

    echo "Running Strong Scaling: Threads=$T"
    /usr/bin/time -f "Threads=$T Real=%e User=%U CPU=%P" \
    ./heat_nearly $N $ITER $INPUT_STRONG $OUTPUT_STRONG
done

# -----------------------------------
# Set fastest thread count (8th is the fastest)
# -----------------------------------
BEST_THREADS=8

# ===================================
# PART 2: MPI WEAK SCALING
# ===================================
# Weak scaling: "Ensure you use as many temperatures as there are ranks"
# This means K must equal P (Ranks).

: <<'BLOCK'
echo ""
echo "===== MPI WEAK SCALING ====="
export OMP_NUM_THREADS=$BEST_THREADS

for P in 1 2 4 8 16 32
do
    K_WEAK=$P
    INPUT_WEAK="input_${K_WEAK}.dat"
    # Format: output_K_N_maxIter
    OUTPUT_WEAK="output_${K_WEAK}_${N}_${ITER}"

    echo "Running Weak Scaling: Ranks=$P Threads=$BEST_THREADS"

    /usr/bin/time -f "Ranks=$P Real=%e User=%U CPU=%P" \
    mpirun -np $P ./heat_complete $N $ITER $INPUT_WEAK $OUTPUT_WEAK 
done
BLOCK

echo ""
echo "====================================="
echo "Benchmark Complete: $(date)"
echo "====================================="
