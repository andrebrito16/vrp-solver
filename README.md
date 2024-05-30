# Vehicle Routing Problem (VRP) Solver

## Table of Contents
- [Vehicle Routing Problem (VRP) Solver](#vehicle-routing-problem-vrp-solver)
  - [Table of Contents](#table-of-contents)
  - [Overview](#overview)
  - [Directory Structure](#directory-structure)
  - [Installation](#installation)
    - [Example for Global Search](#example-for-global-search)
  - [Results:](#results)
- [License](#license)

## Overview
This repository contains various implementations for solving the Vehicle Routing Problem (VRP) using different search techniques. The implementations are provided in C++, and there are accompanying Jupyter notebooks for generating input graphs and analyzing the results.

## Directory Structure

- **global-search/**: Contains the implementation using a global search technique.
- **global-search-mpi/**: Contains the MPI-based implementation of the global search.
- **global-search-omp/**: Contains the OpenMP-based implementation of the global search.
- **local-search/**: Contains the implementation using a local search technique.
- **local-search-mpi/**: Contains the MPI-based implementation of the local search.
- **local-search-omp/**: Contains the OpenMP-based implementation of the local search.
- **inputs/**: Contains various input graph files.
- **public/**: Contains the images for result analysis.
- **generate-report.ipynb**: Jupyter notebook for generating the report.
- **graph-generator.ipynb**: Jupyter notebook for generating input graphs.

## Installation
To compile and run the C++ implementations, navigate to the respective directory and use the provided `makefile`.

### Example for Global Search
```bash
cd global-search
make
./global_search
```

## Results:
Using the ipynb to generate reports for each strategy used to solve the VRP
problem.

Here are the both plots:

![Comparison](/public/aggregated_results_20240530174209.png)

![Comparison Log](/public/aggregated_results_log_20240530174209.png)

The conclusion for this problem is that the global search is a strategy that always give the best results, but it is the most expensive in terms of time. The local search is a good strategy that gives good results and is faster than the global search. The local search with MPI is a good strategy that gives good results and is faster than the local search. The local search with OpenMP is a good strategy that gives good results and is faster than the local search with MPI.

The MPI strategy took a little bit more time in some cases, for example, when running for 7 cities, this is because the network latency to communicate between the nodes is higher than the time saved by the parallelism.

# License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

Project developed by André Brito and oriented by Prof. Michel Fornaciali.


