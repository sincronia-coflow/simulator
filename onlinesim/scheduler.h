#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "coflow.h"

#include <set>
#include <vector>

int check_any_flow_left(Coflow c);

unsigned long long assign_rates_to_flows_greedy(unsigned long long max_time);

double find_oracle_time(Coflow coflow, int no_ports);
#endif
