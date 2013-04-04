/* GRAPH MANAGEMENT functions */
/* Clemence Magnien and Matthieu Latapy */
/* September 2007 */
/* clemence.magnien@lip6.fr */
#ifndef GRAPH_H
#define GRAPH_H

#include <stdio.h>

typedef struct _Graph {
  int n;
  int m;
  int **links;
  int *degrees;
  int *capacities;
} Graph;

void free_graph_old_start(Graph *g, int old_0);
void free_graph(Graph *g);
Graph *graph_from_file(FILE *f);
void sort_graph(Graph *g);
int *sort_nodes_by_degrees(Graph *g); /* in O(m) time and O(n) space */
void renumbering(Graph *g, int *perm);
int random_renumbering(Graph *g);
#endif
