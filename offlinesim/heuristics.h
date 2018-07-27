// want to create ordering based on heuristics, input -> vector of coflows, output, vector of coflows

#ifndef HEURISTICS_H
#define HEURISTICS_H

#include "coflow.h"
#include <vector>

double gamma(Coflow C);

std::vector<Coflow> Smallest_Coflow_First_Ordering(std::vector<Coflow> C);
std::vector<Coflow> Shortest_Coflow_First_Ordering(std::vector<Coflow> C);
std::vector<Coflow> Narrowest_Coflow_First_Ordering(std::vector<Coflow> C);
std::vector<Coflow> SEBF_Ordering(std::vector<Coflow> C);

#endif
