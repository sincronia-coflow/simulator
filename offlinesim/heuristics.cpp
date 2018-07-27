#include "heuristics.h"
#include "coflow.h"
#include "common.h"

#include <vector>
#include <algorithm>
#include <numeric>
#include <iostream>

double EPS1 = 0.00000000000093132257;
double SIZE_EPS1 = 0.000000953674316;
double gamma(Coflow C){
  double term1 = -1;
  //term1 max over all ingress port
  std::vector<double> temp;
  for(int i=0;i<B.no_ports;i++){
    temp.push_back(0);
  }
  std::vector< std::vector<double> > d;
  for(int i=0;i<B.no_ports;i++){
    d.push_back(temp);
  }
  for(int i=0;i<C.flows.size();i++){
    d[C.flows[i].sender_id][C.flows[i].receiver_id] += C.flows[i].size_left;
  }
  for(int i=0;i<B.no_ports;i++){
    double temp_num = std::accumulate(d[i].begin(),d[i].end(),0.0);
    if(temp_num <= SIZE_EPS1){
      continue;
    }
    else if(temp_num > SIZE_EPS1 & S[i].left_bandwidth < EPS1){
      return -1;
    }
    double temp_val = temp_num / S[i].left_bandwidth;
    if(temp_val > term1){
      term1 = temp_val;
    }
  }
  //term2 max over all egress port
  double term2 = -1;
  std::vector<double> temp_2;
  for(int i=0;i<B.no_ports;i++){
    temp_2.push_back(0);
  }
  std::vector< std::vector<double> > d_2;
  for(int i=0;i<B.no_ports;i++){
    d_2.push_back(temp);
  }
  for(int i=0;i<C.flows.size();i++){
    d_2[C.flows[i].receiver_id][C.flows[i].sender_id] += C.flows[i].size_left;
  }
  for(int i=0;i<B.no_ports;i++){
    double temp_num_2 = std::accumulate(d_2[i].begin(),d_2[i].end(),0.0);
    if(temp_num_2 <= SIZE_EPS1){
      continue;
    }
    else if(temp_num_2 > SIZE_EPS1 & R[i].left_bandwidth < EPS1){
      return -1;
    }
    double temp_val_2 = temp_num_2 / R[i].left_bandwidth;
    if(temp_val_2 > term2){
      term2 = temp_val_2;
    }
  }
  return std::max(term1,term2);
}


bool SCF_Comp(const Coflow & c1, const Coflow & c2)
{
   if(c1.length==c2.length){
     return c1.coflow_id < c2.coflow_id;
   }
   else{
     return c1.length < c2.length;
   }

}

bool TCF_Comp(const Coflow & c1, const Coflow & c2)
{
  if(c1.size==c2.size){
    return c1.coflow_id < c2.coflow_id;
  }
  else{
    return c1.size < c2.size;
  }
}

bool NCF_Comp(const Coflow & c1, const Coflow & c2)
{
  if(c1.width==c2.width){
    return c1.coflow_id < c2.coflow_id;
  }
  else{
    return c1.width < c2.width;
  }
}

bool SEBF_Comp(const Coflow & c1, const Coflow & c2)
{
  if(gamma(c1) == gamma(c2)){
    return 0;
  }
  else{
    return gamma(c1) < gamma(c2);
  }

}

std::vector<Coflow> Shortest_Coflow_First_Ordering(std::vector<Coflow> C){
  std::sort(C.begin(),C.end(),SCF_Comp);
  return C;
}

std::vector<Coflow> Smallest_Coflow_First_Ordering(std::vector<Coflow> C){
  std::sort(C.begin(),C.end(),TCF_Comp);
  return C;
}

std::vector<Coflow> Narrowest_Coflow_First_Ordering(std::vector<Coflow> C){
  std::sort(C.begin(),C.end(),NCF_Comp);
  return C;
}

std::vector<Coflow> SEBF_Ordering(std::vector<Coflow> C){
  std::stable_sort(C.begin(),C.end(),SEBF_Comp);
  return C;
}
