#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iomanip>
#include <fstream>
//#include <limits.h>
//#include <math>

#include "coflow.h"
#include "flow.h"
#include "bigswitch.h"
#include "scheduler.h"
#include "ports.h"
#include "primal_dual.h"
#include "common.h"
#include "heuristics.h"


#include <set>
#include <vector>

std::vector<Coflow> work_conservation_coflows;
std::vector<Coflow> vector_coflows;
std::vector<Sender> S;
std::vector<Receiver> R;
double counter;
Bigswitch B;

int main(int argc, char const *argv[]) {
  /*------------------------------------------------------------------------------------------------- 
  Input args:
  [1] input coflow trace (check format at https:github.com/sakshamagarwals/coflow_workload_generator)
  [2] output file to store algorithm coflow completion times
  [3] ordering mechanism (PD (primal dual), SEBF (shortest effective bottleneck first), NCF (narrowest coflow first), SCF (shortest coflow first), TCF (smallesT coflow first))
  [4] rate allocation scheme: Greedy (check sincronia paper) or MADD (check Varys paper)
  -------------------------------------------------------------------------------------------------*/

  FILE *F;
  F = fopen(argv[1],"r");
  std::ofstream outputfile;
  outputfile.open (argv[2]);

  /* ------------------------------
  Parse the input coflow trace file
  --------------------------------*/
  int portsize, num_coflows;
  fscanf(F,"%d",&portsize);
  fscanf(F,"%d",&num_coflows);

  std::vector<Coflow> coflows;         //declare set of coflows
  int coflow_id, release_date;
  int no_senders, no_receivers, no_flows, send_id, receive_id;
  int flow_size;
  std::vector<Flow> flows_to_add;

  for(int i=0;i<num_coflows;i++){
    flows_to_add.clear();
    fscanf(F,"%d",&coflow_id);
    fscanf(F,"%d",&release_date);
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
    Coflow c(coflow_id,0,flows_to_add, no_senders,no_receivers);  //define the constructor here
    coflows.push_back(c);
  }

  B.no_ports = portsize;
  B.coflows = coflows;
  B.unscheduled_coflows = coflows;

  for(int i=0;i<B.no_ports;i++){
    Sender s(i,1);
    Receiver r(i,1);
    S.push_back(s);
    R.push_back(r);
  }

  /*-------------------------------------------------------------------------
  Note: regarding time unit conversion
  Simulator is assumed to finish 1 MB of data in 1 (simulator) unit of time
  => 1 simulator time unit =  1 / (1024*ACCCESS_LINK_BANDWDTH/8) seconds , for ACCESS_LINK_BANDWIDTH in Gbps
  => 1 second = 128*ACCESS_LINK_BANDWIDTH simulator units
  output file contains results in simulator units and can be converted, for any given ACCESS_LINK_BANDWIDTH accordingly
  --------------------------------------------------------------------------*/

  counter = 0.0;
  std::vector<Coflow> admissible_coflows = B.unscheduled_coflows;

  /*------------------------ 
  Parse the ordering scheme
  --------------------------*/

  if(strncmp(argv[3],"PD",5)==0){
    vector_coflows = primal_dual_ordering(admissible_coflows, B);
  }
  else if(strncmp(argv[3],"SEBF",5)==0){
    vector_coflows = SEBF_Ordering(admissible_coflows);
  }
  else if(strncmp(argv[3],"SCF",5)==0){
    vector_coflows = Shortest_Coflow_First_Ordering(admissible_coflows);
  }
  else if(strncmp(argv[3],"NCF",5)==0){
    vector_coflows = Narrowest_Coflow_First_Ordering(admissible_coflows);
  }
  else if(strncmp(argv[3],"TCF",5)==0){
    vector_coflows = Smallest_Coflow_First_Ordering(admissible_coflows);
  }

  for(int k=0;k<vector_coflows.size();k++){
    vector_coflows[k].oracle_time = find_oracle_time(vector_coflows[k],B.no_ports);
  }

  /*------------------------------ 
  Parse the rate allocation scheme
  --------------------------------*/
  if(strncmp(argv[4],"GREEDY",5)==0){
    while(check_any_coflow_left(vector_coflows)){
      double time_to_event = assign_rates_to_flows();  
      counter += time_to_event;
    }
  }
  else if(strncmp(argv[4],"MADD",5)==0){
    while(check_any_coflow_left(vector_coflows)){
       double time_to_event = assign_MADD_rates_to_flows();  
      counter += time_to_event;
    }
  }
  for(int k=0;k<vector_coflows.size();k++){
    if(vector_coflows[k].completion_time == -1){
      std::cout << "Coflow " << k+1 << " completed" << '\n';
      vector_coflows[k].completion_time = counter;
    }
  }

  /*------------------------------------------------------------
  Calculate avg coflow completion times and print required stats
  -------------------------------------------------------------*/

  for(int k=0;k<vector_coflows.size();k++){
    std::cout  << std::setprecision(5) << "Coflow ID: " << vector_coflows[k].coflow_id << 
    " Oracle Time: " << vector_coflows[k].oracle_time << 
    " Selected Ordering: " << k+1 <<
    " Scheduled time: " << vector_coflows[k].start_time << 
    " Completion Time: " << vector_coflows[k].completion_time << 
    "\n";
  }

  for(int k=0;k<vector_coflows.size();k++){
    outputfile << vector_coflows[k].coflow_id << " " 
    << vector_coflows[k].oracle_time  << " " 
    << k+1 << " " 
    << vector_coflows[k].start_time << " " 
    << vector_coflows[k].completion_time 
    << "\n";
  }

  float coflow_completion_time_sum = 0.0;
  for(int k=0;k<vector_coflows.size();k++){
    coflow_completion_time_sum += (float) vector_coflows[k].completion_time;
  }

  float coflow_avg_completion_time = coflow_completion_time_sum / vector_coflows.size();
  std::cout << "Average CCT: " << coflow_avg_completion_time << '\n';
  outputfile << coflow_avg_completion_time << "\n";
  outputfile.close();
  return 0;
}
