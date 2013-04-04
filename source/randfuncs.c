#include <math.h>
#include <stdlib.h>
#include "randfuncs.h"

// rdtsc is a CPU cycle-counter as a random seed
unsigned long long rdtsc(){
  unsigned int lo,hi;
  __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
  return ((unsigned long long)hi << 32) | lo;
}

// random variables generator
double urand() { // uniform in [0,1[
  return (double)rand()/(double)RAND_MAX;
}
int urandn(int n) { // uniform in [0..n[
  return rand() % n;
}
double erand(double mu) { // exponential with rate 1/mu
  return -log(urand())*mu;
}
int grand(double p) { // geometric with prob p;
  double mu = -1.0/log(1-p); // if X~E(1/mu), [X]~Geo(p), p=1-e^(-1/mu) 
  return (int)floor(erand(mu));
}
int g2rand(double mu) { // geometric from rate mu
  return (int)floor(erand(mu));
}
