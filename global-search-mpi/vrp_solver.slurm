#!/bin/bash
#SBATCH --job-name=VRP_Solver
#SBATCH --output=vrp_solver_%j.out
#SBATCH --error=vrp_solver_%j.err
#SBATCH --partition=espec             # Explicitly specify the partition if needed
#SBATCH --nodes=2                     # Total number of nodes requested
#SBATCH --ntasks-per-node=16          # Number of tasks per node
#SBATCH --cpus-per-task=1             # Number of CPUs per task
#SBATCH --mem-per-cpu=200M            # Memory per CPU core

total_procs=$((SLURM_NTASKS_PER_NODE * SLURM_NNODES))


INPUT_DIR="../inputs"
VEHICLE_CAPACITY=20
MAX_CITIES_PER_ROUTE=3

for input_file in "$INPUT_DIR"/graph_*_nodes.txt; do
    base_name=$(basename "$input_file" .txt)
    
    echo "Processing $input_file with vehicle capacity $VEHICLE_CAPACITY and max cities per route $MAX_CITIES_PER_ROUTE"

    mpirun -np $total_procs ./global_search_mpi "$input_file" "$VEHICLE_CAPACITY" "$MAX_CITIES_PER_ROUTE"
    
    echo "-------------------------------------------------------------"
done
