# Sincronia Online Simulator

Flow level simulator to check the performance of online coflow scheduling algorithm in paper "Sincronia: Near-Optimal Scheduling for Coflows". User should have Gurobi solver to run the simulator. Make the changes to gurobi include flags according to the installed location and source OS in the Makefile [https://www.gurobi.com/documentation/7.0/quickstart_linux/cpp_building_and_running_t.html].

Requires Gurobi version >= 7.0.2. If using version different from 7.0.2, replace the files "gurobi_c++.h" and "gurobi_c.h" with the corresponding ones found in the include folder of the install directory. Gurobi provides free academic licenses [https://user.gurobi.com/download/licenses/free-academic].

To run the simulator, compile by 

#### make all

#### ./onlinesim [path to input coflow trace] [path to output log] [maximum #epochs before reset]

To generate tracefiles, (runs without errors with numy 1.14.1, scipy 1.0.0 and matplotlib 2.1.2)

#### bash generate_trace.sh [#Coflows] [Network Load]

Input trace is generated with format at https://github.com/sincronia-coflow/workload-generator. Check the respective README to generate traces with other varying characteristics.

##### Simply run "bash 2000-0.9.sh" to generate and simulate for 2000 coflow trace with 0.9 network load, and stores the output log as output-2000-0.9.txt. 
