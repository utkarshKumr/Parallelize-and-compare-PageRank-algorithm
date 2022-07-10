#ifndef __GRADE_H__
#define __GRADE_H__

#include <stdio.h>
#include <sstream>
#include <iomanip>
#include <chrono>

#include <type_traits>
#include <utility>

#include <float.h>
#include <cmath>

#include <omp.h>

#include "process_graph.h"
#include "graph_internal.h"
#include "contracts.h"

using namespace std;

template <class T>
bool compareArrays(Graph graph, T* ref, T* stu)
{
  int i=0;
  while (i < graph->number_of_nodes) {
    if (ref[i] != stu[i]) {
      cerr << "*** Results disagree at " << i << " expected " 
        << ref[i] << " found " << stu[i] << endl;
      return false;
    }
    i++;
  }
  return true;
}

template <class T>
bool compareApprox(Graph graph, T* ref, T* stu)
{
  int i=0;
  while (i < graph->number_of_nodes) {
    if (fabs(ref[i] - stu[i]) > 0.00000000001) {
      cerr << "*** Results disagree at " << i << " expected " 
        << ref[i] << " found " << stu[i] << endl;
      return false;
    }
    i++;
  }
  return true;
}

#endif /* __GRADE_H__ */
