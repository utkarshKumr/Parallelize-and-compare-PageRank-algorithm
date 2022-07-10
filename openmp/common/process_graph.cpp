#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>

#include "process_graph.h"
#include "graph_internal.h"

#define GRAPH_HEADER_TOKEN ((int) 0xDEADBEEF)
using namespace std;



void build_start(graph* graph, int* scratch)
{
  int number_of_nodes = graph->number_of_nodes;
  graph->outgoing_starts = (int*)malloc(sizeof(int) * number_of_nodes);
  int i=0;
  while(i < number_of_nodes)
  {
    graph->outgoing_starts[i] = scratch[i];
    i++;
  }
}

void store_graph_binary(const char* filename, Graph graph) {

    FILE* output = fopen(filename, "wb");

    if (!output) {
        fprintf(stderr, "Could not open: %s\n", filename);
        exit(1);
    }

    int header[3];
    header[0] = GRAPH_HEADER_TOKEN;
    header[1] = graph->number_of_nodes;
    header[2] = graph->num_edges;

    if (fwrite(header, sizeof(int), 3, output) != 3) {
        fprintf(stderr, "Error writing header.\n");
        exit(1);
    }

    if (fwrite(graph->outgoing_starts, sizeof(int), graph->number_of_nodes, output) != (size_t) graph->number_of_nodes) {
        fprintf(stderr, "Error writing nodes.\n");
        exit(1);
    }

    if (fwrite(graph->outgoing_edges, sizeof(int), graph->num_edges, output) != (size_t) graph->num_edges) {
        fprintf(stderr, "Error writing edges.\n");
        exit(1);
    }

    fclose(output);
}



void build_edges(graph* graph, int* scratch)
{
  int number_of_nodes = graph->number_of_nodes;
  graph->outgoing_edges = (int*)malloc(sizeof(int) * graph->num_edges);
  int i=0;
  while(i < graph->num_edges)
  {
    graph->outgoing_edges[i] = scratch[number_of_nodes + i];
    i++;
  }
}

void build_incoming_edges(graph* graph) {


    int number_of_nodes = graph->number_of_nodes;
    graph->incoming_starts = (int*)malloc(sizeof(int) * number_of_nodes);
    graph->incoming_edges = (int*)malloc(sizeof(int) * graph->num_edges);

    int* node_counts = (int*)malloc(sizeof(int) * number_of_nodes);
    int* node_scatter = (int*)malloc(sizeof(int) * number_of_nodes);


    int i=0;
    while (i<number_of_nodes){
        node_counts[i] = node_scatter[i] = 0;
        i++;
    }
    int total_edges = 0;
    while (i<number_of_nodes) {
        int start_edge = graph->outgoing_starts[i];
        int end_edge;
        if(i == graph->number_of_nodes-1){
            end_edge=graph->num_edges;
        } else {
            end_edge = graph->outgoing_starts[i+1];
        }
        for (int j=start_edge; j<end_edge; j++) {
            node_counts[graph->outgoing_edges[j]]++;
            total_edges++;
        }
        i++;
    }

    graph->incoming_starts[0] = 0;
    i=1;
    while (i<number_of_nodes) {
        graph->incoming_starts[i] = graph->incoming_starts[i-1] + node_counts[i-1];
        i++;
    }

    while (i<number_of_nodes) {
        int start_edge = graph->outgoing_starts[i];
        int end_edge = (i == graph->number_of_nodes-1) ? graph->num_edges : graph->outgoing_starts[i+1];
        for (int j=start_edge; j<end_edge; j++) {
            graph->incoming_edges[graph->incoming_starts[graph->outgoing_edges[j]] + node_scatter[graph->outgoing_edges[j]]] = i;
            node_scatter[graph->outgoing_edges[j]]++;
        }
        i++;
    }


    free(node_counts);
    free(node_scatter);
}

void get_meta_data(ifstream& file, graph* graph)
{
  file.clear();
  file.seekg(0, ios::beg);
  string buffer;
  getline(file, buffer);
  if ((buffer.compare(string("AdjacencyGraph"))))
  {
    cout << "Invalid input file" << buffer << endl;
    exit(1);
  }
  buffer.clear();

  do {
      getline(file, buffer);
  } while (buffer.size() == 0 || buffer[0] == '#');

  graph->number_of_nodes = atoi(buffer.c_str());
  buffer.clear();

  do {
      getline(file, buffer);
  } while (buffer.size() == 0 || buffer[0] == '#');

  graph->num_edges = atoi(buffer.c_str());

}

void read_graph_file(ifstream& file, int* scratch)
{
  string buffer;
  int idx = 0;
  while(!file.eof())
  {
    buffer.clear();
    getline(file, buffer);

    if (buffer.size() > 0 && buffer[0] == '#')
        continue;

    stringstream parse(buffer);
    while (!parse.fail()) {
        int v;
        parse >> v;
        if (parse.fail())
        {
            break;
        }
        scratch[idx] = v;
        idx++;
    }
  }
}

void print_graph(const graph* graph)
{
    printf("Graph pretty print:\n");
    printf("number_of_nodes=%d\n", graph->number_of_nodes);
    printf("num_edges=%d\n", graph->num_edges);
    int i=0;
    while (i<graph->number_of_nodes) {

        int start_edge = graph->outgoing_starts[i];
        int end_edge = (i == graph->number_of_nodes-1) ? graph->num_edges : graph->outgoing_starts[i+1];
        printf("node %02d: out=%d: ", i, end_edge - start_edge);
        int j=start_edge;
        while (j<end_edge) {
            int target = graph->outgoing_edges[j];
            printf("%d ", target);
            j++;
        }
        printf("\n");

        start_edge = graph->incoming_starts[i];
        end_edge = (i == graph->number_of_nodes-1) ? graph->num_edges : graph->incoming_starts[i+1];
        printf("         in=%d: ", end_edge - start_edge);
        j=start_edge;
        while (j<end_edge) {
            int target = graph->incoming_edges[j];
            printf("%d ", target);
            j++;
        }
        printf("\n");
        i++;
    }
}

Graph load_binary_graph(const char* filename)
{
    graph* graph = (struct graph*)(malloc(sizeof(struct graph)));

    FILE* input = fopen(filename, "rb");

    if (!input) {
        fprintf(stderr, "Could not open: %s\n", filename);
        exit(1);
    }

    int header[3];

    if (fread(header, sizeof(int), 3, input) != 3) {
        fprintf(stderr, "Error reading header.\n");
        exit(1);
    }

    if (header[0] != GRAPH_HEADER_TOKEN) {
        fprintf(stderr, "Invalid graph file header. File may be corrupt.\n");
        exit(1);
    }

    graph->number_of_nodes = header[1];
    graph->num_edges = header[2];

    graph->outgoing_starts = (int*)malloc(sizeof(int) * graph->number_of_nodes);
    graph->outgoing_edges = (int*)malloc(sizeof(int) * graph->num_edges);

    if (fread(graph->outgoing_starts, sizeof(int), graph->number_of_nodes, input) != (size_t) graph->number_of_nodes) {
        fprintf(stderr, "Error reading nodes.\n");
        exit(1);
    }

    if (fread(graph->outgoing_edges, sizeof(int), graph->num_edges, input) != (size_t) graph->num_edges) {
        fprintf(stderr, "Error reading edges.\n");
        exit(1);
    }

    fclose(input);

    build_incoming_edges(graph);

    return graph;
}



void free_graph(Graph graph)
{
  free(graph->outgoing_starts);
  free(graph->outgoing_edges);

  free(graph->incoming_starts);
  free(graph->incoming_edges);
  free(graph);
}

Graph load_graph(const char* filename)
{
  graph* graph = (struct graph*)(malloc(sizeof(struct graph)));

  // open the file
  ifstream graph_file;
  graph_file.open(filename);
  get_meta_data(graph_file, graph);

  int* scratch = (int*) malloc(sizeof(int) * (graph->number_of_nodes + graph->num_edges));
  read_graph_file(graph_file, scratch);

  build_start(graph, scratch);
  build_edges(graph, scratch);
  free(scratch);

  build_incoming_edges(graph);

  //print_graph(graph);

  return graph;
}