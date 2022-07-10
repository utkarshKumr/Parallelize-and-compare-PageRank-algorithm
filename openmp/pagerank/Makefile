all: default

default: page_rank.cpp main.cpp
	g++ -std=c++11 -fopenmp -g -O3 -o pr main.cpp page_rank.cpp ../common/process_graph.cpp
clean:
	rm -rf pr *~ *.*~
