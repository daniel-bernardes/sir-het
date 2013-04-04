#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "initialcondition.h"

/**
   Allocates a set of n infected nodes' id
*/
inline void ic_init(InitialCondition *ic, int n) {
  ic->num_infected = n;
  ic->infected = (int *) calloc(n, sizeof(int));
  assert(ic->infected != NULL);
  ic->infectedt= (int *) calloc(n, sizeof(int));
  assert(ic->infectedt != NULL);
}

/**
   De-allocates a set of n infected nodes' id
*/
void ic_clean(InitialCondition *ic) {
  if(ic) {
    free(ic->infected);
    free(ic->infectedt);
    ic->infected = NULL;
    ic->infectedt= NULL;
    ic->num_infected = 0;
  }
}

/**
   Returns the address of a new initial condition with one infected node (0)
*/
inline InitialCondition *ic_trivial() {
  InitialCondition *ic = (InitialCondition *) calloc(1,sizeof(InitialCondition));
  assert(ic != NULL);
  ic_init(ic, 1);
  ic->infected[0] = 0;
  return ic;
}

/**
   Returns 'epidemics' epidemics with one randomly infected node
*/
InitialCondition *ic_random_epidemics(int epidemics, int total_nodes) {
  int i;
  InitialCondition *ic = (InitialCondition *) calloc(epidemics, sizeof(InitialCondition));
  assert(ic != NULL);

  for (i = 0; i < epidemics; i++) {
    ic_init(ic+i, 1);
    (ic+i)->id = i;
    (ic+i)->infected[0] = rand() % total_nodes;
  }
  return ic;
}

/**
   Picks ic->num_infected distinct infected nodes from 0, ..., total_nodes
   and stores their ids into ic->infected.
*/
void ic_infect_randomly(InitialCondition *ic, int total_nodes) {
  int num_infected = ic->num_infected;
  int i, v, k, *infected = (int *) calloc(total_nodes, sizeof(int));
  assert(infected != NULL);
  assert(num_infected < total_nodes);

  // if num_infected > total_nodes/2, pick the non-infected nodes
  k = (num_infected <= total_nodes/2) ? num_infected : total_nodes-num_infected;
  for (i = 0; i < k; i++) {
    do
      v = rand() % total_nodes;
    while (infected[v]);
    infected[v] = !infected[v];
  }

  i = 0;
  for (v = 0; v < total_nodes; v++)
    if ((num_infected < total_nodes/2 &&  infected[v]) ||
	(num_infected > total_nodes/2 && !infected[v]))
      ic->infected[i++] = v;

  free(infected);
}

/**
   Import initial conditions from file into the array *ic. If 'total_nodes'
   is non-zero, pick infected nodes' id randomly from 0, ..., 'total_nodes',
   otherwise read the nodes' id from the corresponding line
   File format:
   <number of epidemics>
   <epidemic id> <number of infected nodes N> [<node 1> ... <node N>]
   ...
*/
int ic_import(InitialCondition **ic, FILE *input, int total_nodes) {
  int i, j, id, num_infected, tokens_read, epidemics = 0;
  assert(input != NULL);
  tokens_read = fscanf(input, "%d\n", &epidemics);
  assert(tokens_read == 1);
  assert(epidemics > 0);
  assert(ic != NULL);
  *ic = (InitialCondition *) calloc(epidemics, sizeof(InitialCondition));
  assert(*ic != NULL);

  for (i = 0; i < epidemics; i++) {
    tokens_read = fscanf(input, "%d %d", &id, &num_infected);
    assert(tokens_read == 2 && num_infected > 0);
    ic_init(*ic+i, num_infected);
    (*ic+i)->id = id;
    if(total_nodes)
      ic_infect_randomly(*ic+i, total_nodes);
    else
      for (j = 0; j < num_infected; j++) {
        tokens_read = fscanf(input, "%d,%d",&(*ic+i)->infected[j],
			     &(*ic+i)->infectedt[j]);
        assert(tokens_read == 2);
      }
  }
  return epidemics;
}

/**
   Import stop bounds for each epidemic in the array *ic from file
   composed of a collection of lines with: <id> <bound>
*/
void ic_import_bounds(InitialCondition *ic, int n, FILE *input) {
  int i, id, bound, tokens_read;
  assert(n > 0);
  assert(ic != NULL);
  assert(input != NULL);
  
  for (i = 0; i < n; i++) {
    tokens_read = fscanf(input, "%d %d\n", &id, &bound);
    assert(tokens_read == 2);
    assert(id == ic[i].id);
    assert(bound > 0);
    ic[i].bound = bound;
  }
}

/**
   Import vector of n doubles: <id> <double_val>
*/
double *import_dlist(int n, FILE *input) {
  int i, id, tokens_read;
  double *array; 
  assert(n > 0);
  assert(input != NULL);
  array = (double *) calloc(n,sizeof(double));
  assert(array != NULL);
  
  for (i = 0; i < n; i++) {
    tokens_read = fscanf(input, "%d %lf\n", &id, &(array[i]));
    assert(tokens_read == 2);
    assert(id == i);
    assert(array[i] >= 0.0);
  }
  return array;
}

/**
   Import vector of n ints: <id> <int_val>
*/
int *import_ilist(int n, FILE *input) {
  int i, id, tokens_read, *array; 
  assert(n > 0);
  assert(input != NULL);
  array = (int *) calloc(n,sizeof(int));
  assert(array != NULL);
  
  for (i = 0; i < n; i++) {
    tokens_read = fscanf(input, "%d %d\n", &id, &(array[i]));
    assert(tokens_read == 2);
    assert(id == i);
    assert(array[i] >= 0);
  }
  return array;
}
