#include "scheduler.h"
#include "flow.h"
#include "coflow.h"
#include "ports.h"
#include "common.h"

#include <iostream>
#include <limits>
#include <climits>
#include <algorithm>
#include <vector>
#include <set>
#include <numeric>
#include <iomanip>
double EPS = 0.0000001;
long long unit_size = 1000000000;
double unit_size_dbl = 1000000000.0;

int check_any_flow_left(Coflow c){
  for(int i=0;i<c.flows.size();i++){
    if(c.flows[i].size_left > EPS){
      return 1;
    }
  }
  return 0;
}

/*---------------------------------------------------------------- 
assign_rates_to_flows_greedy:
input args: time deadline 
schedules the vector_coflows set until a max time of the deadline
returns the time until which scheduled
-------------------------------------------------------------------*/
unsigned long long assign_rates_to_flows_greedy(unsigned long long max_time){
  if(vector_coflows.size()==0){
    return max_time;
  }

  unsigned long long time_to_event = ULLONG_MAX;

  for(int i=0;i<vector_coflows.size();i++){
    if(check_any_flow_left(vector_coflows[i])){
      for(int j=0;j<vector_coflows[i].flows.size();j++){
        if(vector_coflows[i].flows[j].size_left > EPS){
          if(S[vector_coflows[i].flows[j].sender_id].left_bandwidth > EPS & R[vector_coflows[i].flows[j].receiver_id].left_bandwidth > EPS){    
            //if it is possible to schedule this flow of the coflow
            double allocated_bw = std::min(S[vector_coflows[i].flows[j].sender_id].left_bandwidth,R[vector_coflows[i].flows[j].receiver_id].left_bandwidth);
            vector_coflows[i].flows[j].bw_allocated = allocated_bw;
            vector_coflows[i].flows[j].scheduled = 1;
            S[vector_coflows[i].flows[j].sender_id].left_bandwidth -= allocated_bw;
            R[vector_coflows[i].flows[j].receiver_id].left_bandwidth -= allocated_bw;
            double temp = (vector_coflows[i].flows[j].size_left / allocated_bw)*(unit_size_dbl);
            unsigned long long time_for_this_flow = (unsigned long long) temp;
            if(time_for_this_flow < time_to_event ){
              time_to_event = time_for_this_flow;    
              //rates to all flows have been allocated and time_to_event has been calculated
            }
          }
        }
        else{
          if(vector_coflows[i].flows[j].completion_time == -1){
            vector_coflows[i].flows[j].completion_time = counter;
          }
        }
      }
    }
    else{
      if(vector_coflows[i].completion_time == -1){
        std::cout << "Coflow " << vector_coflows[i].coflow_id << " completed" << '\n';
        vector_coflows[i].completion_time = counter;
      }
    }
    time_to_event = std::min(time_to_event,max_time);
  }

  for(int i=0;i<vector_coflows.size();i++){
    for(int j=0;j<vector_coflows[i].flows.size();j++){
      if(vector_coflows[i].flows[j].scheduled){
        vector_coflows[i].flows[j].size_left -= (((double)time_to_event)/unit_size)*vector_coflows[i].flows[j].bw_allocated;     
        //updating the flow size
        vector_coflows[i].size_left -= (((double)time_to_event)/unit_size)*vector_coflows[i].flows[j].bw_allocated;
        //updating the coflow size
        if(vector_coflows[i].flows[j].size_left < EPS){
          vector_coflows[i].flows[j].size_left = 0;
        }
        if(vector_coflows[i].start_time == -1){
          vector_coflows[i].start_time = counter;
        }
        S[vector_coflows[i].flows[j].sender_id].left_bandwidth = S[vector_coflows[i].flows[j].sender_id].max_avail_bandwidth;
        R[vector_coflows[i].flows[j].receiver_id].left_bandwidth = R[vector_coflows[i].flows[j].receiver_id].max_avail_bandwidth;
        S[vector_coflows[i].flows[j].sender_id].data_sent += time_to_event*vector_coflows[i].flows[j].bw_allocated;
        S[vector_coflows[i].flows[j].sender_id].completion_time = counter + time_to_event;
        R[vector_coflows[i].flows[j].receiver_id].data_sent += time_to_event*vector_coflows[i].flows[j].bw_allocated;
        R[vector_coflows[i].flows[j].receiver_id].completion_time = counter + time_to_event;
        vector_coflows[i].flows[j].scheduled = 0;
      }
    }
  }
  //now all flows are again unscheduled and we will repeat the whole procedure on the next event
  return time_to_event;
}

/*-------------------------------------------------------- 
find_oracle_time:
input args: a coflow and number of ports for the bigswitch
returns the oracle completion time for the coflow
----------------------------------------------------------*/
double find_oracle_time(Coflow coflow, int no_ports){
  std::vector<double> s, r;
  for(int i=0;i<no_ports;i++){
    s.push_back(0);
    r.push_back(0);
  }
  for(int i=0;i<coflow.flows.size();i++){
    s[coflow.flows[i].sender_id] += (coflow.flows[i].size/(S[coflow.flows[i].sender_id].max_avail_bandwidth));
    r[coflow.flows[i].receiver_id] += (coflow.flows[i].size/(R[coflow.flows[i].receiver_id].max_avail_bandwidth));
  }
  double max_send_time = -1;
  double max_receive_time = -1;
  for(int i=0;i<no_ports;i++){
    if(s[i]>max_send_time){
      max_send_time = s[i];
    }
    if(r[i]>max_receive_time){
      max_receive_time = r[i];
    }
  }
  double max_time = max_send_time;
  if(max_receive_time > max_send_time){
    max_time = max_receive_time;
  }
  return max_time;
}