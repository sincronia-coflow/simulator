#include "scheduler.h"
#include "flow.h"
#include "coflow.h"
#include "ports.h"
#include "common.h"
#include "heuristics.h"

#include <iostream>
#include <limits.h>
#include <algorithm>
#include <vector>
#include <set>
#include <iomanip>

double EPS = 0.00000000000093132257;
double SIZE_EPS = 0.000000953674316;
// 9.3132257e-13
int check_any_coflow_left(std::vector<Coflow> v){
  for(int i=0;i<v.size();i++){
    for(int j=0;j<v[i].flows.size();j++){
      if(v[i].flows[j].size_left > SIZE_EPS){
        return 1;
      }
    }
  }
  return 0;
}

int any_flow_left(Coflow C){
  for(int i=0;i<C.flows.size();i++){
    if(C.flows[i].size_left > SIZE_EPS){
      return 1;
    }
  }
  return 0;
}

/*---------------------------------------------------------------- 
assign_rates_to_flows_greedy:
input args: time deadline 
schedules the vector_coflows and
returns the time until which scheduled
-------------------------------------------------------------------*/

double assign_rates_to_flows(){

  double time_to_event = (double) INT_MAX;
  for(int i=0;i<vector_coflows.size();i++){
    if(vector_coflows[i].size_left > EPS){        //consider the top coflow which has yet data to be sent
      for(int j=0;j<vector_coflows[i].flows.size();j++){
        if(vector_coflows[i].flows[j].size_left > EPS){
          if(S[vector_coflows[i].flows[j].sender_id].left_bandwidth > EPS & R[vector_coflows[i].flows[j].receiver_id].left_bandwidth > EPS){    //if it is possible to schedule this flow of the coflow
            double allocated_bw = std::min(S[vector_coflows[i].flows[j].sender_id].left_bandwidth,R[vector_coflows[i].flows[j].receiver_id].left_bandwidth);
            vector_coflows[i].flows[j].bw_allocated = allocated_bw;
            vector_coflows[i].flows[j].scheduled = 1;
            S[vector_coflows[i].flows[j].sender_id].left_bandwidth -= allocated_bw;
            R[vector_coflows[i].flows[j].receiver_id].left_bandwidth -= allocated_bw;
            double time_for_this_flow = vector_coflows[i].flows[j].size_left / allocated_bw;
            if(time_for_this_flow < time_to_event ){
              time_to_event = time_for_this_flow;    //rates to all flows have been allocated and time_to_event has been calculated
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
        std::cout << "Coflow " << i+1 << " completed" << '\n';
        vector_coflows[i].completion_time = counter;
      }
    }
  }

  for(int i=0;i<vector_coflows.size();i++){
    for(int j=0;j<vector_coflows[i].flows.size();j++){
      if(vector_coflows[i].flows[j].scheduled){
        vector_coflows[i].flows[j].size_left -= time_to_event*vector_coflows[i].flows[j].bw_allocated;     //updating the flow size
        vector_coflows[i].size_left -= time_to_event*vector_coflows[i].flows[j].bw_allocated;              //updating the coflow size
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

/*---------------------------------------------------------------- 
assign_MADD_rates_to_flows:
input args: time deadline 
schedules the vector_coflows using MADD scheme and
returns the time until which scheduled
-------------------------------------------------------------------*/
double assign_MADD_rates_to_flows(){
  double time_to_event = (double) INT_MAX;

  std::vector<int> skipped_coflow_index;
  skipped_coflow_index.clear();
  for(int i=0;i<vector_coflows.size();i++){
    if(any_flow_left(vector_coflows[i])){
      double gamma_coflow = gamma(vector_coflows[i]);
      //MADD rate allocation
      if(gamma_coflow == -1){
        skipped_coflow_index.push_back(i);
        for (int j=0; j<vector_coflows[i].flows.size(); j++){
          if (vector_coflows[i].flows[j].size_left <= SIZE_EPS)
            if (vector_coflows[i].flows[j].completion_time == -1)
              vector_coflows[i].flows[j].completion_time = counter;
        }
        continue;
      }

      for(int j=0;j<vector_coflows[i].flows.size();j++){
        if(vector_coflows[i].flows[j].size_left > SIZE_EPS & S[vector_coflows[i].flows[j].sender_id].left_bandwidth > EPS & R[vector_coflows[i].flows[j].receiver_id].left_bandwidth > EPS){
          double r_ij = vector_coflows[i].flows[j].size_left / gamma_coflow;
          if(r_ij>S[vector_coflows[i].flows[j].sender_id].left_bandwidth || r_ij>R[vector_coflows[i].flows[j].receiver_id].left_bandwidth){
            r_ij = std::min(S[vector_coflows[i].flows[j].sender_id].left_bandwidth,R[vector_coflows[i].flows[j].receiver_id].left_bandwidth);
          }
          vector_coflows[i].flows[j].bw_allocated = r_ij;
          S[vector_coflows[i].flows[j].sender_id].left_bandwidth -= r_ij;
          R[vector_coflows[i].flows[j].receiver_id].left_bandwidth -= r_ij;
          vector_coflows[i].flows[j].scheduled = 1;
        }
        else{
          if(vector_coflows[i].flows[j].completion_time == -1){
            vector_coflows[i].flows[j].completion_time = counter;
          }
        }
      }
      if(gamma_coflow < time_to_event){
        //time when next coflow finishes
        time_to_event = gamma_coflow;
      }
    }
    else{
      if(vector_coflows[i].completion_time ==-1){
        std::cout << "Coflow " << i+1 << " completed" << '\n';
        vector_coflows[i].completion_time = counter;
      }
    }
  }
    //implementing work conservation now
    std::vector<int> num_flows_at_senders;
    std::vector<int> num_flows_at_receivers;
    num_flows_at_receivers.clear();
    num_flows_at_senders.clear();
    for(int i=0;i<B.no_ports;i++){
      num_flows_at_receivers.push_back(0);
      num_flows_at_senders.push_back(0);
    }
    for(int i=0;i<skipped_coflow_index.size();i++){
      for(int j=0;j<vector_coflows[skipped_coflow_index[i]].flows.size();j++){
        if(S[vector_coflows[skipped_coflow_index[i]].flows[j].sender_id].left_bandwidth > EPS & R[vector_coflows[skipped_coflow_index[i]].flows[j].receiver_id].left_bandwidth > EPS & vector_coflows[skipped_coflow_index[i]].flows[j].size_left > SIZE_EPS){
          num_flows_at_receivers[vector_coflows[skipped_coflow_index[i]].flows[j].receiver_id] += 1;
          num_flows_at_senders[vector_coflows[skipped_coflow_index[i]].flows[j].sender_id] += 1;
        }
      }
    }

    for(int i=0;i<skipped_coflow_index.size();i++){
      for(int j=0;j<vector_coflows[skipped_coflow_index[i]].flows.size();j++){
        if(S[vector_coflows[skipped_coflow_index[i]].flows[j].sender_id].left_bandwidth > EPS & R[vector_coflows[skipped_coflow_index[i]].flows[j].receiver_id].left_bandwidth > EPS & vector_coflows[skipped_coflow_index[i]].flows[j].size_left > SIZE_EPS){
          double allocated_bw = std::min(S[vector_coflows[skipped_coflow_index[i]].flows[j].sender_id].left_bandwidth/num_flows_at_senders[vector_coflows[skipped_coflow_index[i]].flows[j].sender_id],R[vector_coflows[skipped_coflow_index[i]].flows[j].receiver_id].left_bandwidth/num_flows_at_receivers[vector_coflows[skipped_coflow_index[i]].flows[j].receiver_id]);
          vector_coflows[skipped_coflow_index[i]].flows[j].bw_allocated = allocated_bw;
          if(vector_coflows[skipped_coflow_index[i]].flows[j].bw_allocated < EPS){
            vector_coflows[skipped_coflow_index[i]].flows[j].bw_allocated = 0;
          }
          vector_coflows[skipped_coflow_index[i]].flows[j].scheduled = 1;
        }
      }
    }

    //update the left bandwith after allocating to skipped coflows
    for(int i=0;i<skipped_coflow_index.size();i++){
      for(int j=0;j<vector_coflows[skipped_coflow_index[i]].flows.size();j++){
        if(vector_coflows[skipped_coflow_index[i]].flows[j].scheduled){
          S[vector_coflows[skipped_coflow_index[i]].flows[j].sender_id].left_bandwidth -= vector_coflows[skipped_coflow_index[i]].flows[j].bw_allocated;
          R[vector_coflows[skipped_coflow_index[i]].flows[j].receiver_id].left_bandwidth -= vector_coflows[skipped_coflow_index[i]].flows[j].bw_allocated;
        }
      }
    }

    for(int i=0;i<vector_coflows.size();i++){
      if(vector_coflows[i].completion_time != -1){   //coflow already completed
        continue;
      }
      else{
        for(int j=0;j<vector_coflows[i].flows.size();j++){
          if(vector_coflows[i].flows[j].completion_time != -1){
            continue;
          }
          else{
              if(S[vector_coflows[i].flows[j].sender_id].left_bandwidth > EPS & R[vector_coflows[i].flows[j].receiver_id].left_bandwidth > EPS){
              double allocated_bw = std::min(S[vector_coflows[i].flows[j].sender_id].left_bandwidth,R[vector_coflows[i].flows[j].receiver_id].left_bandwidth);
              if(allocated_bw < EPS){
                allocated_bw = 0;
              }
              vector_coflows[i].flows[j].bw_allocated += allocated_bw;
              S[vector_coflows[i].flows[j].sender_id].left_bandwidth -= allocated_bw;
              R[vector_coflows[i].flows[j].receiver_id].left_bandwidth -= allocated_bw;
              vector_coflows[i].flows[j].scheduled = 1;
            }
          }
        }
      }
    }
  //reset the things for the next time to event
  for(int i=0;i<vector_coflows.size();i++){
    for(int j=0;j<vector_coflows[i].flows.size();j++){
      if(vector_coflows[i].flows[j].scheduled){
        vector_coflows[i].flows[j].scheduled = 0;
        double data_flown = std::min(vector_coflows[i].flows[j].size_left,vector_coflows[i].flows[j].bw_allocated*time_to_event);
        vector_coflows[i].flows[j].size_left -= data_flown;
        vector_coflows[i].size_left -= data_flown;
        if(vector_coflows[i].flows[j].size_left < SIZE_EPS){
          vector_coflows[i].flows[j].size_left = 0;
        }
        if(vector_coflows[i].size_left < SIZE_EPS){
          vector_coflows[i].size_left = 0;
        }
        vector_coflows[i].flows[j].bw_allocated = 0;
        if(vector_coflows[i].start_time == -1){
          vector_coflows[i].start_time = counter;
        }
        S[vector_coflows[i].flows[j].sender_id].left_bandwidth = S[vector_coflows[i].flows[j].sender_id].max_avail_bandwidth;
        R[vector_coflows[i].flows[j].receiver_id].left_bandwidth = R[vector_coflows[i].flows[j].receiver_id].max_avail_bandwidth;
      }
    }
  }

  return time_to_event;
}
