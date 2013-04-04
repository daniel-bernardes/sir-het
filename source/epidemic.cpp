/*
  SIMPLE EPIDEMIC CASCADE SIMULATION:
  SIR process such that infected nodes become recovered in one time step
  Output: the complete trace of the spreading -- ie, including the spread
  attempts to removed individuals.

  Daniel.Bernardes@lip6.fr, winter 2012/13

  Source: epidemic evolution
*/
#include <assert.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <limits>
#include <string>
#include <sstream>
#include <fstream>

#include "epidemic.hpp"
#include "randfuncs.c"

#define VERBOSE 1
#define UNSETVAL numeric_limits<int>::min()
#define EPSILON  0.00001

using namespace std;

// Epidemic class
Epidemic::Epidemic(Graph *gr, FILE *outp) {
  assert(gr != NULL);
  assert(gr->n > 0);
  graph    = gr;
  output   = outp;
  removed  = new int[graph->n];
  infected = new int[graph->n];
  infctime = new int[graph->n];
  visitedn = new int[graph->n];
  depth    = new int[graph->n];
  connections.resize(graph->n);
  fill_n(depth,   graph->n,UNSETVAL);
  fill_n(removed, graph->n,UNSETVAL);
  fill_n(infected,graph->n,UNSETVAL);
  fill_n(infctime,graph->n,UNSETVAL);
}

Epidemic::~Epidemic() {
  delete[] depth;
  delete[] removed;
  delete[] infected;
  delete[] infctime;
  delete[] visitedn;
}
void Epidemic::setup(InitialCondition *ic) {
  assert(ic->id >= 0);
  id            = ic->id;
  bound         = ic->bound;
  mu            = ic->mu;
  p             = ic->p;
  initiali      = ic->infected;
  initialt      = ic->infectedt;
  num_infected  = ic->num_infected;
  cascade_links = 0;
  max_depth     = 0;
}

inline void Epidemic::nodeinfect(int v)  { infected[v]= id; }
inline void Epidemic::noderemove(int u)  { removed[u] = id; }
inline bool Epidemic::nodeinfected(int u){ return (infected[u]== id); }
inline bool Epidemic::noderemoved(int u) { return (removed[u] == id); }
inline bool Epidemic::nodedown(int u,int t) {
  return (t > connections[u].second); } 
inline bool Epidemic::nodeonline(int u,int t) {
  return (t >= connections[u].first && t <= connections[u].second); }

/**
   Runs epidemic up to the specified time bound
*/
int Epidemic::simulate() {
  int i,u,v,t,dt,randindex;
  fill_n(visitedn,graph->n,0); // reset number of infected neighbors
  fill_n(depth,   graph->n,0); // reset number of infected neighbors

  // activate initial grains
  for (i = 0; i < num_infected; i++) {
    v = initiali[i];
    t = initialt[i];
    nodeinfect(v);
    depth[v] = 1;
    infctime[v] = -t; // negative to mark initial nodes
    ActiveNodes.push(NodeAction(v,t));
    #if VERBOSE > 1
    cout << "push: (" << v << "," << t << ")" << endl;
    #endif
  }

  max_depth = 1;

  // run the epidemic
  while (!ActiveNodes.empty()) {
    u = ActiveNodes.top().first;  // current provider
    t = ActiveNodes.top().second; // current time
    ActiveNodes.pop();
    noderemove(u);
    #if VERBOSE > 1
    cout << "pop: (" << u << "," << t << ")" << endl;
    #endif
    
    // select a random neighbor from u, which was not visited by u
    randindex = rand() % (graph->degrees[u]-visitedn[u]);
    v = graph->links[u][randindex];
    if (nodeonline(v,t) || nodedown(v,t)) {
      // can be consided from now on visited by v
      swap(graph->links[u][randindex],
	   graph->links[u][(graph->degrees[u]-visitedn[u])-1]);
      visitedn[u]++;
    }
    #if VERBOSE > 1
    cout << u << " --> " << v 
	 << "  nodeonline("<<v<<","<<t<<"): "<<(nodeonline(v,t)?"Yes" : "No") 
	 << ", nodedown("  <<v<<","<<t<<"): "<<(nodedown(v,t)?  "Yes" : "No")
	 << endl;
    #endif

    if (nodeonline(v,t) && urand() <= p) {
      if (!nodeinfected(v)) {
	cascade_links++;
	num_infected++;
	
	nodeinfect(v);
	infctime[v] = t;
	depth[v] = depth[u]+1;
	max_depth= max(max_depth,depth[v]);

	if (mu[v] > EPSILON) { // ie, mu != 0.0
	  dt = g2rand(mu[v]);
	  if (nodeonline(v,t+dt) && (t+dt <= bound)) {
	    ActiveNodes.push(NodeAction(v,t+dt));
	  }
          #if VERBOSE > 1
	    cout <<"push attempt: ("<<v<<","<<t+dt<<") -- "
		 << "nodeonline(" <<v<<","<<t+dt<<"): "
		 <<(nodeonline(v,t+dt)?"Yes" : "No")<< endl;
          #endif
	}
	if (output) // print output: t P C F
	  fprintf(output, "%d %d %d %d\n",t,u,v,id);
	
      } else if (nodeinfected(v) && !noderemoved(v) && infctime[v] == t) {
	cascade_links++;
	depth[v] = max(depth[v],depth[u]+1);
	max_depth= max(depth[v],max_depth);
	if (output) // print output: t P C F
	  fprintf(output, "%d %d %d %d\n",t,u,v,id);
      }
    }
    
    // keep u active if within activity bounds
    if (mu[u] > EPSILON && graph->degrees[u] > visitedn[u]) {
      dt = g2rand(mu[u]*graph->degrees[u]/(graph->degrees[u]-visitedn[u]));
      #if VERBOSE > 1
      cout <<"self push attempt: ("<<u<<"," <<t+dt<<") -- "
	   << "nodeonline(" <<u<<","<<t+dt<<"): "
	   <<(nodeonline(u,t+dt)?"Yes" : "No")<< endl;
      #endif
      if (nodeonline(u,t+dt) && (t+dt <= bound)) {
	ActiveNodes.push(NodeAction(u,t+dt));
        #if VERBOSE > 1
        cout <<"self push confirmed : ("<<u<<"," <<t+dt<<")" << endl;
        #endif
      }
    }
  }
  return t;
}

void Epidemic::readconnections(char* path) {
  int login,logout,u;
  string line;
  ifstream infile(path);

  for(int i=0; i<graph->n; i++) {
    getline(infile, line);
    istringstream iss(line);
    if (!(iss >> u >> login >> logout))
      { throw 10; }
    if (u != i)
      { throw 11; }
    if (login < 0 || login > logout)
      { throw 12; }
    connections[i] = pair<int,int>(login,logout);
  }  
  infile.close();
}
