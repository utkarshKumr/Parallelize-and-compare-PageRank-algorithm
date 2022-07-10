#ifndef __GRAPH_H__
#define __GRAPH_H__

using Vertex = int;

struct graph
{
    int num_edges;
    int number_of_nodes;

    int* outgoing_starts;
    Vertex* outgoing_edges;

    int* incoming_starts;
    Vertex* incoming_edges;
};

using Graph = graph*;

/* Getters */
static inline int number_of_nodes(const Graph);
static inline int num_edges(const Graph);

static inline const Vertex* outgoing_begin(const Graph, Vertex);
static inline const Vertex* outgoing_end(const Graph, Vertex);
static inline int outgoing_edges_count(const Graph, Vertex);

static inline const Vertex* incoming_edge_begin(const Graph, Vertex);
static inline const Vertex* incoming_edge_end(const Graph, Vertex);
static inline int incoming_edges_count(const Graph, Vertex);


/* IO */
Graph load_graph(const char* filename);
Graph load_binary_graph(const char* filename);
void store_graph_binary(const char* filename, Graph);

void print_graph(const graph*);


/* Deallocation */
void free_graph(Graph);


/* Included here to enable inlining. Don't look. */
#include "graph_internal.h"

#endif
