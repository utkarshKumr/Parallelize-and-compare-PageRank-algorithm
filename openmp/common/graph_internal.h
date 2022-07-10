#ifndef __GRAPH_INTERNAL_H__
#define __GRAPH_INTERNAL_H__

#include <stdlib.h>
#include "contracts.h"

static inline int number_of_nodes(const Graph graph)
{
  REQUIRES_CHECK(graph != NULL);
  return graph->number_of_nodes;
}

static inline int num_edges(const Graph graph)
{
  REQUIRES_CHECK(graph != NULL);
  return graph->num_edges;
}

static inline const Vertex* outgoing_begin(const Graph grph, Vertex vertex)
{
  REQUIRES_CHECK(grph != NULL);
  REQUIRES_CHECK(0 <= vertex && vertex < number_of_nodes(grph));
  return grph->outgoing_edges + grph->outgoing_starts[vertex];
}

static inline const Vertex* outgoing_end(const Graph grph, Vertex vertex)
{
  REQUIRES_CHECK(grph != NULL);
  REQUIRES_CHECK(0 <= vertex && vertex < number_of_nodes(grph));
  int offset_value;
  if(vertex == grph->number_of_nodes - 1){
    offset_value =  grph->num_edges;
  } else {
    offset_value = grph->outgoing_starts[vertex + 1];
  }
  return grph->outgoing_edges + offset_value;
}

//number of outgoing edges of a vertex
static inline int outgoing_edges_count(const Graph grph, Vertex vertex)
{
  REQUIRES_CHECK(grph != NULL);
  REQUIRES_CHECK(0 <= vertex && vertex < number_of_nodes(grph));

  if (vertex == grph->number_of_nodes - 1) {
    return grph->num_edges - grph->outgoing_starts[vertex];
  } else {
    return grph->outgoing_starts[vertex + 1] - grph->outgoing_starts[vertex];
  }
}

static inline const Vertex* incoming_edge_begin(const Graph grph, Vertex vertex)
{
  REQUIRES_CHECK(grph != NULL);
  REQUIRES_CHECK(0 <= vertex && vertex < number_of_nodes(grph));
  return grph->incoming_edges + grph->incoming_starts[vertex];
}

static inline const Vertex* incoming_edge_end(const Graph grph, Vertex vertex)
{
  REQUIRES_CHECK(grph != NULL);
  REQUIRES_CHECK(0 <= vertex && vertex < number_of_nodes(grph));
  int offset_value;
  if(vertex == grph->number_of_nodes - 1){
    offset_value = grph->num_edges;
  } else {
    offset_value = grph->incoming_starts[vertex + 1];
  }
  return grph->incoming_edges + offset_value;
}

static inline int incoming_edges_count(const Graph grph, Vertex vertex)
{
  REQUIRES_CHECK(grph != NULL);
  REQUIRES_CHECK(0 <= vertex && vertex < number_of_nodes(grph));
  int res;
  if (vertex == grph->number_of_nodes - 1) {
    res=grph->num_edges - grph->incoming_starts[vertex];
  } else {
    res=grph->incoming_starts[vertex + 1] - grph->incoming_starts[vertex];
  }
  return res;
}

#endif // __GRAPH_INTERNAL_H__
