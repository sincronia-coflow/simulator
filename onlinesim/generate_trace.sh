if [ -d "$workload-generator" ]; then
cd workload-generator
python trace_producer.py $1 FB-UP $2
cd ..
else
git clone http://github.com/sincronia-coflow/workload-generator
cd workload-generator
python trace_producer.py $1 FB-UP $2
cd ..
fi