OBJS = main.o coflow.o flow.o scheduler.o ports.o primal_dual.o heuristics.o
CC = g++
CFLAGS = -c $(DEBUG)

all : $(OBJS)
	$(CC) -o offlinesim $(OBJS) $(LFLAGS)

clean:
	\rm *.o offlinesim
