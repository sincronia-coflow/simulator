#ifndef COFLOW_H
#define COFLOW_H

#include "flow.h"

#include <vector>

class Coflow{
public:
  int coflow_id;
  // int release_date;
  unsigned long long release_date;
  std::vector<Flow> flows;
  unsigned long long deadline;
  int priority;
  long long start_time;
  long long completion_time;
  double size;
  double size_left;
  int width;
  int length;
  int weight;
  double oracle_time;
  Coflow(int coflow_id, unsigned long long release_date, std::vector<Flow> flows, int no_senders, int no_receivers);


};




#endif
