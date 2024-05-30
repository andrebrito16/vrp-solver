all:
	mpic++ -std=c++17 -Wall -O3 main.cpp -o global_search_omp

mac:
	mpicxx -fopenmp -o global_search_mpi main.cpp

mac-run:
	g++-14 -fopenmp -std=c++17 main.cpp -o global_search_omp 
	./global_search_omp ../inputs/graph_6_nodes.txt