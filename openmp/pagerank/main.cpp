#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define USE_BINARY_GRAPH 1
#define PageRankDampeningFactor 0.3f
#define PageRankConvergenceFactor 1e-7d

#include <string>
#include <getopt.h>

#include <iostream>
#include <sstream>
#include <vector>


#include "../common/CycleMeasuringTimer.h"
#include "../common/process_graph.h"
#include "../common/utils.h"
#include "page_rank.h"
using namespace std;


// used for check correctness
void findSerialPageRank(Graph grph, double* solution, double damping,
                               double convergence) {
    int numberOfNodes = number_of_nodes(grph);
    double globalDifference = 0.0;
    double* solution_new = new double[numberOfNodes];
    double* oldScore = solution;
    double* newScore = solution_new;
    bool active = true;
    double broadcastScore = 0.0;
    
    int iterations = 0;

    int i=0;
    while (i < numberOfNodes) {
        solution[i] = 1.0 / numberOfNodes;
        i++;
    }

    while (active && iterations < MAX_ITERATIONS) {
        iterations++;
        broadcastScore = 0.0;
        globalDifference = 0.0;
        int i=0;

        while (i < numberOfNodes) {
            newScore[i] = 0.0;

            const Vertex* input_begin_node = incoming_edge_begin(grph, i);
            const Vertex* input_end_node = incoming_edge_end(grph, i);

            if (outgoing_edges_count(grph, i) == 0) {
                broadcastScore += oldScore[i];
            }

            for (const Vertex* v = input_begin_node; v < input_end_node; ++v) {

                newScore[i] += oldScore[*v] / outgoing_edges_count(grph, *v);

            }

            newScore[i] = damping * newScore[i] + (1.0 - damping) * (1.0 / numberOfNodes);
            ++i;

        }
        i=0;
        while (i < numberOfNodes) {
            newScore[i] += damping * broadcastScore * (1.0 / numberOfNodes);
            globalDifference = globalDifference + abs(newScore[i] - oldScore[i]);
            i++;
        }
        active = !(globalDifference < convergence);
        swap(newScore, oldScore);
    }

    if (newScore != solution) {
        memcpy(solution, newScore, sizeof(double) * numberOfNodes);
    }
    delete[] solution_new;
}

double findReferencePageRankTime(Graph grph, double* solution){
    double start;

    start = CycleMeasuringTimer::currentSeconds();

    findSerialPageRank(grph, solution, PageRankDampeningFactor, PageRankConvergenceFactor);
    double referencePageRankTime = CycleMeasuringTimer::currentSeconds() - start;


    return referencePageRankTime;
}   

double findPageRankTime(Graph grph, double* solution, int number_of_threads){
    double start;
    omp_set_num_threads(number_of_threads);
    start = CycleMeasuringTimer::currentSeconds();
    calculatePageRank(grph, solution, PageRankDampeningFactor, PageRankConvergenceFactor);
    double pagerank_time = CycleMeasuringTimer::currentSeconds() - start;
    return pagerank_time;
}

int main(int argc, char** argv) {

    int num_threads = -1;
    string filename_graph;

    if (argc < 3) {
        cerr << "Usage: <path/to/graph/file> <manual_set_thread_count>\n";
        exit(1);
    }

    int number_of_threads = -1;
    if (argc == 3) {
        number_of_threads = atoi(argv[2]);
    }
    if (number_of_threads <= 0) {
        cerr << "<manual_set_thread_count> must > 0\n";
        exit(1);
    }
    filename_graph = argv[1];

    Graph grph;


    printf("Running with %d number of threads\n", number_of_threads);
    printf("----------------------------------------------------------\n");


    if (USE_BINARY_GRAPH) {
        grph = load_binary_graph(filename_graph.c_str());
    } else {
        grph = load_graph(argv[1]);
        printf("storing binary form of graph!\n");
        store_graph_binary(filename_graph.append(".bin").c_str(), grph);
        delete grph;
        exit(1);
    }
    printf("\n");
    printf("Graph details:\n");
    printf("  Filename: %s\n", argv[1]);
    printf("  Nodes: %d\n", grph->number_of_nodes);
    printf("  Edges: %d\n", grph->num_edges);

    double* sol1;
    sol1 = (double*) malloc(sizeof(double) * grph->number_of_nodes);
    double* sol2;
    sol2 = (double*) malloc(sizeof(double) * grph->number_of_nodes);

    double pagerank_base;
    double pagerank_time;

    double reference_base_pagerank;
    double reference_base_pagerank2;
    double reference_base_pagerank3;
    double referencePageRankTime;


    stringstream timing;
    stringstream reference_timing;

    timing << "Threads Page Rank\n";
    reference_timing << "Serial Reference Page Rank\n";

    referencePageRankTime = findReferencePageRankTime(grph, sol2);
    pagerank_time = findPageRankTime(grph, sol1, number_of_threads);

    printf("\n");
    cout << "Testing Page Rank Correctness\n";
    if (!compareApprox(grph, sol2, sol1)) {
        cout << "Incorrect Pagerank" << endl;
    } else {
        cout << "Correct Pagerank" << endl;
    }

    char buf[1024];
    char ref_buf[1024];
    sprintf(buf, "%4d:   %.6f s\n",
            number_of_threads, pagerank_time);
    sprintf(ref_buf, "   1:   %.6f s\n",
            referencePageRankTime);

    timing << buf;
    reference_timing << ref_buf;
    delete grph;

    printf("----------------------------------------------------------\n");
    cout << "Serial (Reference) Summary" << endl;
    cout << reference_timing.str() << endl;

    printf("----------------------------------------------------------\n");
    cout << "Timing Summary" << endl;
    cout << timing.str();

    printf("----------------------------------------------------------\n");

    return 0;
}
