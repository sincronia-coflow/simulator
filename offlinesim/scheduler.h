#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "coflow.h"

#include <set>
#include <vector>


int check_any_coflow_left(std::vector<Coflow> v);

double assign_rates_to_flows();

double assign_MADD_rates_to_flows();

double find_oracle_time(Coflow coflow, int no_ports);
#endif
