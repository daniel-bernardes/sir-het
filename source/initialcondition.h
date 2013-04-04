/*
  SIMPLE EPIDEMIC CASCADE SIMULATION:
  SIR process such that infected nodes become recovered in one time step
  Output: the complete trace of the spreading -- ie, including the spread
  attempts to removed individuals.

  Daniel.Bernardes@lip6.fr, winter 2012/13

  Header: initial condition functions
*/
#ifndef INITIALCONDITION_H
#define INITIALCONDITION_H
#include <stdio.h>

typedef struct _InitialCondition {
  int id;                  // epidemic id
  int num_infected;        // number of infected nodes
  int *infected;           // list of infected nodes' id
  int *infectedt;          // list of infected nodes' activation time
  int *bounds;             //list of time corresponding to activity bound
  int bound;               // bounds on epidemic evolution in terms of time
  double p;                // global neighbor infection probability
  double *mu;              // global mean inter event delay (inv. activity rate)
} InitialCondition;

/**
   Allocates a set of n infected nodes' id
*/
inline void ic_init(InitialCondition *ic, int n);

/**
   De-allocates a set of n infected nodes' id
*/
void ic_clean(InitialCondition *ic);

/**
   Returns the address of a new initial condition with one infected node (0)
*/
inline InitialCondition *ic_trivial();

/**
   Returns 'epidemics' epidemics with one randomly infected node
*/
InitialCondition *ic_random_epidemics(int epidemics, int total_nodes);

/**
   Picks ic->num_infected distinct infected nodes from 0, ..., total_nodes
   and stores their ids into ic->infected.
*/
void ic_infect_randomly(InitialCondition *ic, int total_nodes);

/**
   Import initial conditions from file into the array *ic. If 'total_nodes'
   is non-zero, pick infected nodes' id randomly from 0, ..., 'total_nodes',
   otherwise read the nodes' id from the corresponding line
   File format:
   <number of epidemics>
   <epidemic id> <number of infected nodes N> [<node 1> ... <node N>]
   ...
*/
int ic_import(InitialCondition **ic, FILE *input, int total_nodes);

/**
   Import stop bounds for each epidemic in the array *ic from file
   composed of a collection of lines with: <id> <bound>
*/
void ic_import_bounds(InitialCondition *ic, int n, FILE *input);

/**
   Import vector of n doubles: <id> <double>
*/
double *import_dlist(int n, FILE *input);

/**
   Import vector of n ints: <id> <int_val>
*/
int *import_ilist(int n, FILE *input);

#endif
