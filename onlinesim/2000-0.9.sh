make clean
make all
bash generate_trace.sh 2000 0.9
./onlinesim workload-generator/traces/2000-0.9-FB-UP.txt 2000-0.9.txt 8 > 2000-0.9.log