make clean
make all
bash generate_trace.sh 2000 0.9
./offlinesim workload-generator/traces/2000-0.9-FB-UP.txt 2000-0.9-PD-GREEDY.txt PD GREEDY
# ./offlinesim workload-generator/traces/2000-0.9-FB-UP.txt 2000-0.9-SEBF-MADD.txt SEBF MADD