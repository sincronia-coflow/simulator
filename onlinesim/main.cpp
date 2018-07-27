#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <numeric>
#include <limits>
#include <climits>
#include <math.h>
#include <cstdlib>

#include "coflow.h"
#include "flow.h"
#include "bigswitch.h"
#include "scheduler.h"
#include "ports.h"
#include "primal_dual.h"
#include "common.h"
#include "onlinescheduler.h"


#include <set>
#include <vector>

std::vector<Coflow> vector_coflows;
std::vector<Sender> S;
std::vector<Receiver> R;
std::vector<Coflow> unscheduled_ordering;
std::vector<Coflow> work_conservation_coflows;
std::vector<Coflow> inadmissible_coflows;
unsigned long long counter;
Bigswitch B;

double x;
int MAX_RESET_COUNTER;
long long unit_time_int = 1000000000;
double unit_time_dbl = 1000000000.0;

/*---------------------------------------------------------------------------------------------
heuristic_comp: work conservation heuristic comparator, 
input args: coflows c1 and c2 
returns whether coflow c1 should be scheduled ahead of c2 using the work conservation heuristic
----------------------------------------------------------------------------------------------*/
bool heuristic_comp(const Coflow & c1, const Coflow & c2)
{
  if(((((double)counter/unit_time_dbl)+x - ((double)c1.release_date/unit_time_dbl))/c1.oracle_time) == ((((double)counter/unit_time_dbl) + x- ((double)c2.release_date/unit_time_dbl))/c2.oracle_time)){
    return c1.release_date < c2.release_date;
  }
  else{
    return (c1.oracle_time/(((double)counter/unit_time_dbl) + x - ((double)c1.release_date/unit_time_dbl))) < (c2.oracle_time/(((double)counter/unit_time_dbl) +x - ((double)c2.release_date/unit_time_dbl)));
  }

}

int main(int argc, char const *argv[]) {
/*------------------------------------------------------------------------------------------------- 
Input args:
[1] input coflow trace (check format at https:github.com/sakshamagarwals/coflow_workload_generator)
[2] output file to store algorithm coflow completion times
[3] max number of epochs before each reset
  -------------------------------------------------------------------------------------------------*/

FILE *F;
F = fopen(argv[1],"r");
std::ofstream outputfile;
outputfile.open (argv[2]);
MAX_RESET_COUNTER = atoi(argv[3]);
x = pow(2,MAX_RESET_COUNTER-1);
int ACCESS_LINK_BANDWIDTH = 2; //Bandwidth in Gbps

/* ------------------------------
Parse the input coflow trace file
--------------------------------*/

int portsize, num_coflows;
fscanf(F,"%d",&portsize);
fscanf(F,"%d",&num_coflows);
std::vector<Coflow> coflows;  
int coflow_id;
unsigned long long release_date;
unsigned long long rel;
int no_senders, no_receivers, no_flows, send_id, receive_id;
int flow_size;
std::vector<Flow> flows_to_add;

for(int i=0;i<num_coflows;i++){
  flows_to_add.clear();
  fscanf(F,"%d",&coflow_id);
  fscanf(F,"%lld",&rel); //input release date in milliseconds

  /*-------------------------------------------------------------------------
  Normalize release date from microseconds to the unit of simulator:
  Simulator is assumed to finish 1 MB of data in 1 (simulator) unit of time
  => 1 simulator time unit =  1 / (1024*ACCCESS_LINK_BANDWDTH/8) seconds 
  => 1 second = 128*ACCESS_LINK_BANDWIDTH simulator units
  => 1 ms = 128*ACCESS_LINK_BANDWIDTH * 1e6 simulator nano units
  --------------------------------------------------------------------------*/
  unsigned long long release_date_2 = ((rel)*128*ACCESS_LINK_BANDWIDTH*1000000);
  fscanf(F,"%d",&no_flows);
  fscanf(F,"%d",&no_senders);
  fscanf(F,"%d",&no_receivers);
  for(int j=0;j<no_flows;j++){
    fscanf(F,"%d",&send_id);
    fscanf(F,"%d",&receive_id);
    fscanf(F,"%d",&flow_size);
    Flow f(j,coflow_id,flow_size,send_id,receive_id);
    flows_to_add.push_back(f);
  }
  Coflow c(coflow_id,release_date_2,flows_to_add, no_senders,no_receivers);  //define the constructor here
  coflows.push_back(c);
}

/*---------------------------------- 
Create Bigswitch used for simulation
 ----------------------------------*/
B.no_ports = portsize;
B.coflows = coflows;
B.unscheduled_coflows = coflows;
//Create sender and receiver ports for the bigswitch
for(int i=0;i<B.no_ports;i++){
  Sender s(i,1);   //max_avail_bw = 1MBPS
  Receiver r(i,1);
  S.push_back(s);
  R.push_back(r);
}


/*------------------  
Main Simulator Logic 
-------------------*/
counter = 0;
int j=0;
int alpha = 2;
int reset = 0;
int big_counter = 0;
unsigned long long tau_k = 0;
unsigned long long counter_at_reset = 0;
unsigned long long tau_k_at_reset = 0;

// Find coflow oracle time, used to reference the computed performance of the algorithm
for(int k=0;k<B.unscheduled_coflows.size();k++){
  B.unscheduled_coflows[k].oracle_time = find_oracle_time(B.unscheduled_coflows[k],B.no_ports);
  // std::cout << std::setprecision(20) << "Coflow id: " << B.unscheduled_coflows[k].coflow_id << " arrival time: " << B.unscheduled_coflows[k].release_date << '\n';
}


do{
  counter = counter_at_reset + alpha*tau_k;
  if(reset){     
    //online algorithm resets epoch size to smallest after maximum number of allowed epochs before reset
    counter_at_reset = counter;
    tau_k_at_reset = tau_k_at_reset + tau_k;
    tau_k = 0;
    reset = 0;
  }

  std::cout << "Starting counter: "<< counter << '\n';
  std::vector<Coflow> R_k = find_candidate_coflow_set(B.unscheduled_coflows,tau_k_at_reset + tau_k);
  //coflows which are not candidates form the unscheduled coflow set

  std::vector<Coflow> admissible_coflows = generate_online_admissible_set_coflows(R_k,pow(2,(j)+1-1)*unit_time_int-tau_k);
  j++;
  //solve the 2-approximation LP to get the coflows scheduled within the epoch
  tau_k = pow(2,(j)-1)*unit_time_int;

  for(int k=0;k<admissible_coflows.size();k++){
    std::cout << "admitted coflow id: " << admissible_coflows[k].coflow_id  << "size left: " << admissible_coflows[k].size_left << '\n';
  }

  int chosen_size = 0;
  vector_coflows.clear();
  if(admissible_coflows.size()){
    vector_coflows = primal_dual_ordering(admissible_coflows, B);
    chosen_size = vector_coflows.size();
  }
  for(int i=0;i<inadmissible_coflows.size();i++){ //inadmissible coflows by the LP are added back to the set of unscheduled coflows
    B.unscheduled_coflows.push_back(inadmissible_coflows[i]);
  }

  std::vector<Coflow> work_cons;
  unsigned long long epoch_time = counter_at_reset + alpha*tau_k;
  while((counter < epoch_time)){
    work_cons.clear();
    for(int i=chosen_size;i<vector_coflows.size();i++){
      work_cons.push_back(vector_coflows[i]);
    }
    std::vector<Coflow> temp_vector;
    temp_vector.clear();
    for(int i=0;i<chosen_size;i++){
      temp_vector.push_back(vector_coflows[i]);
    }


    int any_new_work_cons_coflow_added = 0;
    for(int i=B.unscheduled_coflows.size()-1;i>=0;i--){
      if(B.unscheduled_coflows[i].release_date <= counter){
        work_cons.push_back(B.unscheduled_coflows[i]);
        any_new_work_cons_coflow_added = 1;
        B.unscheduled_coflows.erase(B.unscheduled_coflows.begin()+i);
      }
    }
    if(any_new_work_cons_coflow_added){
      vector_coflows = temp_vector;
      std::stable_sort(work_cons.begin(),work_cons.end(),heuristic_comp);
      for(int i=0;i<work_cons.size();i++){
        vector_coflows.push_back(work_cons[i]);
      }
    }

    unsigned long long next_deadline = ULLONG_MAX;
    for(int i=0;i<B.unscheduled_coflows.size();i++){
      if(B.unscheduled_coflows[i].release_date < next_deadline){
        next_deadline = B.unscheduled_coflows[i].release_date;
      }
    }

    unsigned long long next_time_max = std::min(epoch_time-counter,next_deadline-counter);
    unsigned long long time_to_event = assign_rates_to_flows_greedy(next_time_max); 
    counter += time_to_event;

  }
  for(int k=0;k<vector_coflows.size();k++){
    if(vector_coflows[k].completion_time == -1 & check_any_flow_left(vector_coflows[k])==0){
      vector_coflows[k].completion_time = counter;
      std::cout << "Coflow " << vector_coflows[k].coflow_id << " completed" << '\n';
    }
  }

  for(int k=0;k<vector_coflows.size();k++){
    if(vector_coflows[k].completion_time != -1){
      B.scheduled_coflows.push_back(vector_coflows[k]);
    }
    else{

      B.unscheduled_coflows.push_back(vector_coflows[k]); 
      //putting back the coflows which are not completed into unscheduled coflows
    }
  }

  if(j%MAX_RESET_COUNTER==0){
    reset = 1;
    j=0;
    big_counter = big_counter + 1;
  }
}while(B.unscheduled_coflows.size());


/*------------------------------------------------------------
Calculate avg coflow completion times and print required stats
-------------------------------------------------------------*/
double coflow_completion_time_sum = 0.0;
for(int k=0;k<B.scheduled_coflows.size();k++){
  coflow_completion_time_sum += (double) B.scheduled_coflows[k].completion_time/(unit_time_dbl);
}

for(int k=0;k<B.scheduled_coflows.size();k++){
  // std::cout  << std::setprecision(20) << "Coflow ID: " << B.scheduled_coflows[k].coflow_id << " Oracle Time: " << B.scheduled_coflows[k].oracle_time << " Selected Ordering: " << k+1 << " Arrival Time: " << B.scheduled_coflows[k].release_date << " Scheduled time: " << B.scheduled_coflows[k].start_time << " Completion Time: " <<  B.scheduled_coflows[k].completion_time << "\n";

  //output times in ms
  outputfile  << std::setprecision(20) << B.scheduled_coflows[k].coflow_id 
  << " "<< B.scheduled_coflows[k].oracle_time/(128.0*ACCESS_LINK_BANDWIDTH)*1000
  << " " << k+1 << " " << ((double)B.scheduled_coflows[k].release_date/unit_time_dbl)/(128.0*ACCESS_LINK_BANDWIDTH)*1000
  << " " << ((double)B.scheduled_coflows[k].start_time/unit_time_dbl)/(128.0*ACCESS_LINK_BANDWIDTH)*1000
  << " "<< ((double)B.scheduled_coflows[k].completion_time/unit_time_dbl)/(128.0*ACCESS_LINK_BANDWIDTH)*1000 << "\n";
}

float coflow_avg_completion_time = coflow_completion_time_sum / B.scheduled_coflows.size() /(128.0*ACCESS_LINK_BANDWIDTH) * 1000;

std::cout << "Average CCT: " << coflow_avg_completion_time << '\n';

outputfile.close();
return 0;
}
