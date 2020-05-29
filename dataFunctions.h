#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashtable.h"
#define err(mess){fprintf(stderr,"ERROR: %s\n",mess);exit(1);}


int diseaseFrequency(char* country,char* virus,HashTable* diseaseHashtable,HashTable* countryHashtable,char* FirstDate,char* SecondDate);

void countTreePatients(int* count,Treenode * root,char* FirstDate,char* SecondDate);

void countFrequency(char* virus,int* count,Treenode * root,char* FirstDate,char* SecondDate);