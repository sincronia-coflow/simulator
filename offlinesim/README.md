# Sincronia Offline Simulator

Flow level simulator to check the performance of offline coflow scheduling algorithm in paper "Sincronia: Near-Optimal Scheduling for Coflows". 

To run the simulator, compile by 

#### make all

#### ./offlinesim [path to input coflow trace] [path to output log] [Coflow ordering scheme (PD/SEBF/NCF/SCF/TCF)] [Rate allocation scheme (GREEDY/MADD)]

To generate tracefiles, (runs without errors with numpy 1.14.1, scipy 1.0.0 and matplotlib 2.1.2)

#### bash generate_trace.sh [#Coflows] [Network Load]

Input trace is generated with format at https://github.com/sincronia-coflow/workload-generator. Check the respective README to generate traces with other varying characteristics.

##### Simply run "bash 2000-0.9.sh" to generate and simulate for 2000 coflow trace with 0.9 network load, and stores the output log as output-2000-0.9-PD-GREEDY.txt. 
