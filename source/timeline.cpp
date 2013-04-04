#include <assert.h>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <functional>
#include <map>

using namespace std;

class Interval {
private:
  int a,b; // interval limits

public:
  Interval(int aa, int bb);
  bool interior(int x);
  bool inter(Interval i);
  int begin();
  int end();
  string str();
};

Interval::Interval(int aa, int bb) {
  if (aa > bb)
    throw "a > b";
  a = aa; b = bb;
}
int Interval::begin(){ return a; }

int Interval::end()  { return b; }

bool Interval::interior(int x) { 
  return (x-a >= 0) && (b-x >=0);
}
bool Interval::inter(Interval i){ 
  return (a<i.begin())? (b-i.begin() >= 0):(i.end()-a >= 0);
}
string Interval::str() {
  stringstream ss;
  ss << "[" << a << ", " << b << "]";
  return ss.str();
}

class Timeline {
private:
  int index;
  vector<Interval> intervals;

public:
  Timeline(vector<Interval> intervls);
  void reset();
  Interval current();
  bool goClosestInterval(int t);
  int nextValidTime(int t0, function <int ()> dt);
};

Timeline::Timeline(vector<Interval> intervls) {
  index = 0;
  intervals = intervls;
}

void Timeline::reset() { index = 0; }

Interval Timeline::current() { return intervals[index]; }

/**
   Set current interval to the closest right interval
   ie, [a,b] st t < b and returns true one exists.
*/
bool Timeline::goClosestInterval(int t) {
  assert(intervals[index].begin()<= t || index == 0);

  while (intervals[index].end()  <  t && index < intervals.size()-1)
    index++;

  return(intervals[index].end()  >= t);
}

/**
   Returns the next valid event time (one in which the user is online)
   or a negative number otherwise. Params: initial time, random dt generator
*/
int Timeline::nextValidTime(int t0, function <int ()> dt) {
  int t = t0 + dt();

  while (goClosestInterval(t) && !current().interior(t))
    t = current().begin() + dt();

  if (current().interior(t))
    return t;
  else
    return -1;
}

////

#include "randfuncs.c"

int urandi(int n) { // uniform in [0..n[
  int i = rand() % n;
  cout << "dt=" << i << endl;
  return i;
}

int main(int argc, char* argv[]) {
  int a,b,t=0;
  vector<Interval> intervls;
  vector<Interval>::iterator it;
  function <int ()> deltat = bind(urandi,20);

  srand(rdtsc());

  while(cin >> a >> b) {
    cout << "[" << a << ", " << b << "]" << endl;
    intervls.push_back(Interval(a,b));
  }
  
  Timeline timeline(intervls);
  cout << endl;

  do {
    t = timeline.nextValidTime(t,deltat);
    cout << "Next event: " << t << endl;
  } while (t >= 0);

  return 0;
}
