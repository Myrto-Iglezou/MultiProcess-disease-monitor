# in order to execute this "Makefile" just type "make"

OBJS   = diseaseAggregator.o worker.o patient.o RBT.o comparators.o hashtable.o utils.o dataFunctions.o heap.o
SOURCE = diseaseAggregator.c worker.c patient.c RBT.c comparators.c hashtable.c utils.c dataFunctions.c heap.c
HEADER = patient.h RBT.h comparators.h hashtable.h utils.h dataFunctions.h heap.h
OUT    = diseaseAggregator 
CC     = gcc
FLAGS  = -g -c

all: diseaseAggregator worker

diseaseAggregator: diseaseAggregator.o utils.o
	$(CC) -g diseaseAggregator.o utils.o  -o diseaseAggregator 

# create/compile the individual files >>seperetaly<<

worker: worker.o patient.o RBT.o comparators.o dataFunctions.o hashtable.o utils.o heap.o
	$(CC) -g worker.o patient.o RBT.o comparators.o utils.o  dataFunctions.o hashtable.o heap.o -o worker -lm


diseaseAggregator.o: diseaseAggregator.c 
	$(CC) $(FLAGS) diseaseAggregator.c 

worker.o: worker.c
	$(CC) $(FLAGS) worker.c 

utils.o: utils.c
	$(CC) $(FLAGS) utils.c 


patient.o: patient.c
	$(CC) $(FLAGS) patient.c

RBT.o: RBT.c
	$(CC) $(FLAGS) RBT.c

comparators.o: comparators.c
	$(CC) $(FLAGS) comparators.c

hashtable.o: hashtable.c
	$(CC) $(FLAGS) hashtable.c

dataFunctions.o: dataFunctions.c
	$(CC) $(FLAGS) dataFunctions.c

heap.o: heap.c
	$(CC) $(FLAGS) heap.c 

# clean house
clean:
	rm -f $(OBJS) $(OUT)

# do a bit of accounting
count:
	wc $(SOURCE) $(HEADER)