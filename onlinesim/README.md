# Sincronia Online Simulator

Flow level simulator to check the performance of online coflow scheduling algorithm in paper [Sincronia: Near-Optimal Network Design for Coflows](https://dl.acm.org/citation.cfm?id=3230569).  

User should have Gurobi solver to run the simulator. Make the changes to gurobi include flags according to the installed location and source OS in the Makefile [https://www.gurobi.com/documentation/7.0/quickstart_linux/cpp_building_and_running_t.html].

Requires Gurobi version >= 7.0.2. If using version different from 7.0.2, replace the files "gurobi_c++.h" and "gurobi_c.h" with the corresponding ones found in the include folder of the install directory. Gurobi provides free academic licenses [https://user.gurobi.com/download/licenses/free-academic].

The main input parameter to the simulator is the number of epochs needed before time horizon gets reset to the smallest value (check the details in the paper).

### Running the Simulator
To compile, run 
```
make all
```

Use the binary **onlinesim** to run the simulator. The simulator requires also path to the input coflow trace and a log file (to generate the output). 
```
./onlinesim [path to coflow-trace] [path to output-log] [number of epochs before reset]
```
For e.g., if we want to simulate for the workload present in file ~/coflow-trace.txt and store the results in ~/output.log and 8 epochs before resetting to smallest size, run
```
./offlinesim ~/coflow-trace.txt ~/output.log 8
```

### Input Trace Format and Generation
Input coflow traces can be generated using this [coflow workload generator](https://github.com/sincronia-coflow/workload-generator). The input format of the traces looks like follows
```
Line1: <NUM_INP_PORTS> <NUM_COFLOWS>
Line2: <Coflow 1ID> <Arrival Time (in millisec)> <Number of Flows in Coflow 1> <Number of Sources in Coflow 1> <Number of Destinations in Coflow 1> <Flow 1 Source ID> <Flow 1 Destination ID> <Flow 1 Size (in MB)> ... <Flow N Source ID> <Flow N1 Destination ID> <Flow N1 size (in MB)>
...
...
Line i+1:  <Coflow iID> <Arrival Time (in millisec)> <Number of Flows in Coflow i> <Number of Sources in Coflow i> <Number of Destinations in Coflow i> <Flow 1 Source ID> <Flow 1 Destination ID> <Flow 1 Size (in MB)> ... <Flow N Source ID> <Flow Ni Destination ID> <Flow Ni size (in MB)>
...
```

#### Output Format
The generated output log has the following format
```
Line i: <Coflow ID> <Oracle Time> <Coflow Ordering> <Coflow Release Date> <Coflow Start Time> <Coflow Completion Time>
```

To quickly generate a coflow trace with similar properties as the [Facebook trace](https://github.com/coflow/coflow-benchmark), but varying the number of coflows and network load run
```
bash generate_trace.sh [Number of Coflows] [Network Load]
```

Check the respective README [here](https://github.com/sincronia-coflow/workload-generator) to generate traces with other varying characteristics.

Simply run "bash 2000-0.9.sh" to generate and simulate for 2000 coflow trace with 0.9 network load and 8 epochs before reset, and stores the output log as output-2000-0.9.txt. 
