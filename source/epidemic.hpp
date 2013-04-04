/*
  SIMPLE EPIDEMIC CASCADE SIMULATION:
  SIR process such that infected nodes become recovered in one time step
  Output: the complete trace of the spreading -- ie, including the spread
  attempts to removed individuals.

  Daniel.Bernardes@lip6.fr, winter 2012/13

  Header: epidemic evolution
*/
#ifndef EPIDEMIC_HPP
#define EPIDEMIC_HPP
#include <queue>
#include <vector>
#include <stdio.h>
#include "graph.h"
#include "initialcondition.h"

using namespace std;

#define NodeAction pair<int,int>

class Smaller2nd {
public:
  int operator() ( const NodeAction& p1, const NodeAction& p2 ) {
    return p1.second > p2.second; }
};

class Epidemic {
private:
  int *initiali;            // list of initial inf nodes' id
  int *initialt;            // list of initial inf nodes' activation time
  int *infected;            // set of all infected nodes
  int *infctime;            // set of all infected nodes
  int *removed;             // set of all infected nodes
  int *visitedn;            // number of infected neighbors for each node
  int *depth;
  priority_queue<NodeAction, vector<NodeAction > , Smaller2nd > ActiveNodes;
  vector<pair<int,int> > connections;
  int id;                   // epidemic id
  int bound;                // time bound on epidemic evolution
  double p;
  double *mu;               // activity rate: inv. of mean inter event time
  Graph *graph;             // underlying graph (network)
  FILE *output;             // trace output

public:
  int max_depth;
  int num_infected;       // number of currently infected nodes
  int cascade_links;      // number of arcs in the infection cascade

  ~Epidemic();
  Epidemic(Graph *gr, FILE *output);
  void setup(InitialCondition *ic);
  void readconnections(char* path);
  int simulate();

  void nodeinfect(int u);
  void noderemove(int u);
  bool nodeinfected(int u);
  bool noderemoved(int u);
  bool nodeonline(int u, int t);
  bool nodedown(int u, int t);
};
#endif
