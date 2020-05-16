# in order to execute this "Makefile" just type "make"

OBJS   = diseaseAggregator.o worker.o patient.o RBT.o comparators.o
SOURCE = diseaseAggregator.c worker.c patient.c RBT.c comparators.c
HEADER = patient.h RBT.h comparators.h
OUT    = diseaseAggregator 
CC     = gcc
FLAGS  = -g -c

all: diseaseAggregator worker

diseaseAggregator: diseaseAggregator.o 
	$(CC) -g diseaseAggregator.o  -o diseaseAggregator 

# create/compile the individual files >>seperetaly<<

worker: worker.o patient.o RBT.o comparators.o
	$(CC) -g worker.o patient.o RBT.o comparators.o -o worker 


diseaseAggregator.o: diseaseAggregator.c 
	$(CC) $(FLAGS) diseaseAggregator.c 

worker.o: worker.c
	$(CC) $(FLAGS) worker.c 

patient.o: patient.c
	$(CC) $(FLAGS) patient.c

RBT.o: RBT.c
	$(CC) $(FLAGS) RBT.c

comparators.o: comparators.c
	$(CC) $(FLAGS) comparators.c

# clean house
clean:
	rm -f $(OBJS) $(OUT)

# do a bit of accounting
count:
	wc $(SOURCE) $(HEADER)