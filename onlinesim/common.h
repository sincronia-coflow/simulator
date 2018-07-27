#ifndef COMMON_H
#define COMMON_H

#include "coflow.h"
#include "bigswitch.h"
#include "ports.h"
#include <vector>

extern std::vector<Coflow> vector_coflows;
extern Bigswitch B;
extern std::vector<Sender> S;
extern std::vector<Receiver> R;
extern unsigned long long counter;
extern std::vector<Coflow> work_conservation_coflows;
extern std::vector<Coflow> inadmissible_coflows;
#endif
