all:
	g++ -std=c++17 main.cpp -o global_search_omp -fopenmp

mac:
	g++-14 -std=c++17 main.cpp -o global_search_omp -fopenmp

mac-run:
	g++-14 -fopenmp -std=c++17 main.cpp -o global_search_omp 
	./global_search_omp ../inputs/graph_6_nodes.txt