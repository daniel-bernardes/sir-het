#ifndef RANDFUNCS_H
#define RANDFUNCS_H

// rdtsc is a CPU cycle-counter as a random seed
unsigned long long rdtsc();

// random variables generator
double urand();          // uniform in [0,1[
double erand(double mu); // exponential with rate 1/mu
int grand(double p);     // geom(p): if X~E(1/mu), [X]~Geo(p), p=1-e^(-1/mu) 
int g2rand(double mu);   // geometric from rate mu
#endif
