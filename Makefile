# in order to execute this "Makefile" just type "make"

OBJS   = diseaseAggregator.o worker.o
SOURCE = diseaseAggregator.c worker.c
HEADER = 
OUT    = diseaseAggregator 
CC     = gcc
FLAGS  = -g -c

all: diseaseAggregator worker

diseaseAggregator: diseaseAggregator.o 
	$(CC) -g diseaseAggregator.o  -o diseaseAggregator 

# create/compile the individual files >>seperetaly<<

worker: worker.o 
	$(CC) -g worker.o  -o worker 


diseaseAggregator.o: diseaseAggregator.c 
	$(CC) $(FLAGS) diseaseAggregator.c 

worker.o: worker.c
	$(CC) $(FLAGS) worker.c 

# clean house
clean:
	rm -f $(OBJS) $(OUT)

# do a bit of accounting
count:
	wc $(SOURCE) $(HEADER)