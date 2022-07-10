#include "page_rank.h"
#include <iostream>
#include <stdlib.h>
#include <cmath>
#include <omp.h>
#include <utility>

#include "../common/CycleMeasuringTimer.h"
#include "../common/process_graph.h"
using namespace std;

void calculatePageRank(Graph g, double* solution, double damping, double convergence) {

    int numberOfNodes = number_of_nodes(g);

    double* oldScore = solution;
    double* solution_new = new double[numberOfNodes];
    double* newScore = solution_new;
    bool active = true;

    int iterations = 0;

    double broadcaseScoreSum = 0.0, globalDifference = 0.0;


    int i=0;
    while (i < numberOfNodes) {
        solution[i] = 1.0 / numberOfNodes;
        ++i;
    }

    while (active && iterations < MAX_ITERATIONS) {
        iterations++;
        broadcaseScoreSum = 0.0;
        globalDifference = 0.0;

        #pragma omp parallel for reduction(+: broadcaseScoreSum) schedule(dynamic, 16)
        for (int i = 0; i < numberOfNodes; ++i) {
            newScore[i] = 0.0;

            const Vertex* input_begin_node = incoming_edge_begin(g, i);
            const Vertex* input_end_node = incoming_edge_end(g, i);

            if (outgoing_edges_count(g, i) < 1){
                broadcaseScoreSum = broadcaseScoreSum + oldScore[i];
            }

            for (const Vertex* v = input_begin_node; v < input_end_node; ++v) {
                newScore[i] += oldScore[*v] / outgoing_edges_count(g, *v);
            }
            newScore[i] = damping * newScore[i] + (1.0 - damping) * (1.0 / numberOfNodes);
        }

        int i=0;
        while (i < numberOfNodes) {
            newScore[i] += damping * broadcaseScoreSum * (1.0 / numberOfNodes);
            globalDifference += abs(newScore[i] - oldScore[i]);
            ++i;
        }
        active = !(globalDifference < convergence);
        swap(newScore, oldScore);
    }

    if (newScore != solution) {
        memcpy(solution, newScore, sizeof(double) * numberOfNodes);
    }
    delete[] solution_new;
}
