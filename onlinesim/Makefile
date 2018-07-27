OBJS = main.o coflow.o flow.o scheduler.o ports.o primal_dual.o onlinescheduler.o
CC = g++
CFLAGS = -c $(DEBUG)
# LFLAGS = -L/home/gurobi702/linux64/lib/ -lgurobi_g++5.2 -lgurobi70 #Gurobi 7.02 installed on Linux in home folder
LFLAGS = -L/Library/gurobi702/mac64/lib/ -lgurobi_c++ -lgurobi70 #Gurobi 7.02 installed on MacOS using Gurobi installer in default location

# Refer to [https://www.gurobi.com/documentation/7.0/quickstart_linux/cpp_building_and_running_t.html] for finding the required files to link

all : $(OBJS)
	$(CC) -o onlinesim $(OBJS) $(LFLAGS)

clean:
	\rm *.o onlinesim
