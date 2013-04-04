/*
  SIMPLE EPIDEMIC CASCADE SIMULATION:
  SIR process such that infected nodes become recovered in one time step
  Output: the complete trace of the spreading -- ie, including the spread
  attempts to removed individuals.

  Daniel.Bernardes@lip6.fr, winter 2012/13
*/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

#include <iostream>

#include "randfuncs.h"
#include "graph.h"
#include "initialcondition.h"
#include "epidemic.hpp"

// misc defs and utils
#define VERBOSE 1
#define MAX_PATH_LENGTH 4096
#define EPSILON  0.00001

// auxiliary functions
inline char *tstamp() {
  time_t now = time(NULL);
  char *str = asctime(localtime(&now));
  str[strlen(str)-1]=' ';
  return str;
}
inline void techo(char *str) {
#if VERBOSE > 1
    fprintf(stdout, "%s -- %s\n", tstamp(), str);
    fflush(stdout);
#endif
}

void parse_params (int argc,char **argv,int *epidemics,int *sample_epidemics,
		   FILE **ic_list_input,FILE **graph_input,char** conn_path,
		   double *mu,FILE **mu_list_input,FILE **bounds_list_input,
		   int *maxtime,char **trace_output_path,FILE **data_output,
		   double *p);
/**
   Main
*/
int main(int argc, char **argv) {
  int i, j;
  Graph *g;
  InitialCondition *ic;
  FILE *epidemic_output   = NULL;
  char epidemic_output_path[MAX_PATH_LENGTH] = "";

  // default parameters
  int maxtime             = 0;      // global maximum epidemic simulation time
  int epidemics           = 1;      // number of epidemics
  int sample_epidemics    = 1;      // number of sample epidemics
  double p                = 1.0;    // global infection probability
  double mu               = 0.0;    // global random time parameter -- Exp=1/mu
  double *mulist          = NULL;   // random time parameter -- Exp=1/mu
  FILE *graph_input       = stdin;  // input for graph (network)
  FILE *ic_list_input     = NULL;   // input for list of epidemic initial params
  FILE *bounds_list_input = NULL;   // input for list of epidemic bounds
  FILE *mu_list_input     = NULL;   // input for list of avg. inter-event delay 
  FILE *data_output       = NULL;   // output for extra simulation info
  char *trace_output_path = NULL;   // output for trace (global)
  char *conn_path         = NULL;

  // parameter parsing
  parse_params(argc,argv,&epidemics,&sample_epidemics,&ic_list_input,
	       &graph_input,&conn_path,&mu,&mu_list_input,&bounds_list_input,
	       &maxtime,&trace_output_path,&data_output,&p);

  assert(graph_input && conn_path);
  assert(mu_list_input || (mu > 0.0));
  assert(bounds_list_input || maxtime > 0);
 
  // preliminaires
  srand(rdtsc());  // (unsigned)time(NULL) // rdtsc in randfuncs.h

  // load underlying graph
  fprintf(stderr,"%s\nLoading the graph...\n", tstamp());
  fflush(stderr);
  g = graph_from_file(graph_input);
  if (graph_input != stdin)
    fclose(graph_input);
  fprintf(stderr,"  Loaded graph with %d nodes, %d links.\n\n", g->n, g->m);
  fflush(stderr);

  // set global epidemic_output /* simplified solution Jan/2012 */
  assert(sample_epidemics == 1); // watch this!
  if (trace_output_path && strlen(trace_output_path) > 0) {
    sprintf(epidemic_output_path,"%s-%s.trace",trace_output_path,"maxtime");
    epidemic_output = fopen(epidemic_output_path, "w");
    assert(epidemic_output != NULL);
  }
  Epidemic epidemic(g,epidemic_output);
  fprintf(stderr,"%s\nLoading connection data from list...\n\n", tstamp());
  fflush(stderr);
  epidemic.readconnections(conn_path);

  // load initial conditions for the epidemics
  fprintf(stderr,"%s\nLoading list of epidemics...\n", tstamp());
  fflush(stderr);
  if (ic_list_input) {
    epidemics = ic_import(&ic, ic_list_input, 0);
    fclose(ic_list_input);
  } else { // infect randomly 'epidemics' epidemics
    fprintf(stderr,"  %s %d %s\n","No list of initial conditions given; loading",
	    epidemics,"epidemics with 1 randomly infected node...");
    ic = ic_random_epidemics(epidemics, g->n);
  }

  // set epidemics' random inter event duration rate
  if (mu_list_input) {
    fprintf(stderr,"Setting activity rate for epidemics from list...\n");
    mulist = import_dlist(g->n, mu_list_input);
    fclose(mu_list_input);
  } else {
    fprintf(stderr,
	    "Setting global activity rate for epidemics (mu=%f)...\n",mu);
    mulist = (double*) calloc(g->n,sizeof(double));
    assert(mulist != NULL);
    for(i = 0; i < g->n; i++)
      mulist[i] = mu;
  }
  for(i = 0; i < epidemics; i++) 
    ic[i].mu = mulist;
  fflush(stderr);
  
  // set bounds
  if (bounds_list_input) {
    fprintf(stderr,"Setting bounds for epidemics from list...\n");
    fflush(stderr);
    ic_import_bounds(ic, epidemics, bounds_list_input);
    fclose(bounds_list_input);
  } else {
    fprintf(stderr,"Setting global bound for epidemics (t=%d)...\n",maxtime);
    fflush(stderr);
    for(i = 0; i < epidemics; i++)
      ic[i].bound = maxtime;
  }

  fprintf(stderr,"Setting global infection probability (p=%f)...\n",p);
  fflush(stderr);
  for(i = 0; i < epidemics; i++)
    ic[i].p = p;

  fprintf(stderr,"  Loaded %d epidemics.\n\n", epidemics);
  fflush(stderr);

  for (j = 0; j < epidemics; j++) {
    fprintf(stderr,"%s: running epidemic %d up to %s = %d ...\n",
	    tstamp(), ic[j].id, "maxtime", ic[j].bound);
    fflush(stderr);
    
    for (i = 1; i <= sample_epidemics; i++) {
      epidemic.setup(ic+j);
      
      if (data_output) {
	fprintf(data_output,
		"Epidemic %d #%d: started with %d / %d ( %.2f%% ) infected nodes\n",
		ic[j].id,i, epidemic.num_infected,
		g->n, 100.0*(float)epidemic.num_infected/(float)g->n);
	fflush(data_output);
      }

      epidemic.simulate();
      
      if (epidemic_output)
	fflush(epidemic_output);
      
      if (data_output) {
	fprintf(data_output, 
"Epidemic %d #%d: stopped with %d depth, %d / %d ( %.2f%% ) infected nodes and %d links\n",
		ic[j].id,i,epidemic.max_depth,epidemic.num_infected,
		g->n, 100.0*(float)epidemic.num_infected/(float)g->n,
      		  epidemic.cascade_links);
	fflush(data_output);
      }
    }
    ic_clean(ic+j);
  }
  
  // close global epidemic_output /* simplified solution Jan/2012 */
  if (epidemic_output)
    fclose(epidemic_output);

  // clean up and exit
  if (data_output && data_output != stdout)
    fclose(data_output);

  fputc('\n', stderr);
  fprintf(stderr,"%s\nDone.\n", tstamp());
  fflush(stderr);
  free_graph(g);
  free(ic);
  return 0;
}

/**
   Parameter parsing
*/
void parse_params (int argc,char **argv,int *epidemics,int *sample_epidemics,
		   FILE **ic_list_input,FILE **graph_input,char**conn_path,
		   double *mu,FILE **mu_list_input,FILE **bounds_list_input,
		   int *maxtime,char **trace_output_path,FILE **data_output,
		   double *p) {
  int i;
  char syntax[] = "\n\
 General parameters (required):\n\t\
 -g GRAPH_PATH\n\t\
 -c CONNECTION_DATA_PATH\n\n\
 Random time parameter (one required choice among the options):\n\t\
 -a GLOBAL_AVERAGE_INTER-EVENT_DELAY\n\t\
 -b AVERAGE_INTER-EVENT_DELAY_LIST\n\n\
 Simulation bounds (one required choice among the options):\n\t\
 -t GLOBAL_MAX_TIME\n\t\
 -m MAX_TIME_LIST_PATH\n\n\
 Initial conditions (one required choice among the options):\n\t\
 -i INITIAL_CONDITIONS_DATA_PATH\n\t\
 -x NUM_RAND_EPIDEMICS\n\n\
 Misc parameters (optional):\n\t\
 -s NUM_SAMPLE_EPIDEMICS (defaul: 1)\n\t\
 -e [STATUS_OUTPUT_PATH]\n\t\
 -o EPIDEMIC_DIR_OUTPUT\n\t\
 -p INFECTION_PROBABILITY (default=1.0)\n";

  fprintf(stderr, "SIMPLE EPIDEMIC CASCADE SIMULATION:\n\n");
  while ((i = getopt(argc, argv, "g:c:a:b:t:m:i:x:s:e::o:p:")) != -1)
    switch (i) {
    case 'g':
      *graph_input = fopen(optarg,"r");
      assert(*graph_input != NULL);
      break;
    case 'c':
      *conn_path = optarg;
      break;
    case 'a':
      assert(*mu_list_input == NULL);
      *mu = atof(optarg);
      assert(*mu > 0.0);
      break;
    case 'b':
      assert(*mu < 0.00001);
      *mu_list_input = fopen(optarg,"r");
      assert(*mu_list_input != NULL);
      break;
    case 't':
      assert(*bounds_list_input == NULL);
      *maxtime = atoi(optarg);
      assert(*maxtime > 0);
      break;
    case 'm':
      assert(*maxtime == 0);
      *bounds_list_input = fopen(optarg,"r");
      assert(*bounds_list_input != NULL);
      break;
    case 'i':
      *ic_list_input = fopen(optarg,"r");
      assert(*ic_list_input != NULL);
      break;
    case 'x':
      assert(*ic_list_input == NULL);
      *epidemics = atoi(optarg);
      assert(*epidemics > 0);
      break;
    case 's':
      *sample_epidemics = atoi(optarg);
      assert(*sample_epidemics > 0);
      break;
    case 'e':
      if (optarg) {
	*data_output = fopen(optarg,"w");
	assert(*data_output != NULL);
      } else
	*data_output = stderr;
      break;
    case 'o':
      *trace_output_path = optarg;
      break;
    case 'p':
      *p = atof(optarg);
      assert(*p > EPSILON && *p <= 1.0);
      break;
    case '?':
      fputs(syntax, stderr);
    default:
      abort();
    }
}
