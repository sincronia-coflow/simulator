# Sincronia Offline Simulator

Flow level simulator to check the performance of offline coflow scheduling algorithm in paper [Sincronia: Near-Optimal Network Design for Coflows](https://dl.acm.org/citation.cfm?id=3230569). 

The simulator takes in an input coflow workload (input format discussed below), and simulates coflow scheduling using various heuristics including ones mentioned in [Varys](https://dl.acm.org/citation.cfm?id=2626315) (SEBF coflow ordering and MADD rate allocation) or using the BSSI coflow ordering and Greedy rate allocation scheme as mentioned in [Sincronia](https://dl.acm.org/citation.cfm?id=3230569). 

#### Input Parameters

The simulator only requires two main parameters -- a coflow ordering and a per-flow rate allocation mechanism.

1. Coflow ordering: The simulator supports the following ordering mechanisms shown along with the required identifier for input.

Coflow Ordering Scheme | Description | Input Identifier
:----------------------:|:----------:|:---------------:
Bottleneck-Select-Scale-Iterate | A combinatorial primal-dual based algorithm used in Sincronia for coflow ordering | **PD**
Smallest Effective Bottleneck First | Orders coflow with smallest bottleneck port fisrt, used in Varys | **SEBF**
Narrowest Coflow First | Orders coflow with smallest width first | **NCF**
Shortest Coflow First | Orders coflow with smallest length first | **SCF**
SmallesT Coflow First | Orders coflow with smallest total size first | **TCF**

2. Rate allocation mechanism: The simulator supports two schemes -- MADD (Minimum Allocation for Desired Duration) as mentioned in Varys and Greedy rate allocation scheme in Sincronia. The identifiers for each are **MADD** and **GREEDY** respectively.

#### Running the Simulator

To compile, run 
```
make all
```

Use the binary **offlinesim** to run the simulator. The simulator requires also path to the input coflow trace and a log file (to generate the output). 
```
./offlinesim [path to coflow-trace] [path to output-log] [Coflow ordering scheme] [Rate allocation scheme]
```
For e.g., if we want to simulate for the workload present in file ~/coflow-trace.txt and store the results in ~/output.log, using BSSI algorithm and Greedy rate allocation, then run simulator using
```
./offlinesim ~/coflow-trace.txt ~/output.log PD GREEDY
```

#### Input Trace Format and Generation
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
Line i: <Coflow ID> <Oracle Time> <Coflow Ordering> <Coflow Start Time> <Coflow Completion Time>
```

To quickly generate a coflow trace with similar properties as the [Facebook trace](https://github.com/coflow/coflow-benchmark), but varying the number of coflows and network load run
```
bash generate_trace.sh [Number of Coflows] [Network Load]
```

Check the respective README [here](https://github.com/sincronia-coflow/workload-generator) to generate traces with other varying characteristics.

Simply run `bash 2000-0.9.sh` to generate and simulate for 2000 coflow trace with 0.9 network load, BSSI algorithm and Greedy rate allocation, and store the output log as output-2000-0.9-PD-GREEDY.txt. 
