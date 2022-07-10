#ifndef __PAGE_RANK_H__
#define __PAGE_RANK_H__

#include "../common/process_graph.h"


#define MAX_ITERATIONS 20

void calculatePageRank(Graph g, double* solution, double damping, double convergence);

#endif
