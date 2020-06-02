#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashtable.h"
#define err(mess){fprintf(stderr,"033[1;31mERROR: \033[0m: %s\n",mess);exit(1);}


int diseaseFrequency(char* country,char* virus,HashTable* diseaseHashtable,HashTable* countryHashtable,char* FirstDate,char* SecondDate,int entry,int exit);

void countTreePatients(int* count,Treenode * root,char* FirstDate,char* SecondDate,int entry,int exit);

void countFrequency(char* virus,int* count,Treenode * root,char* FirstDate,char* SecondDate,int entry,int exit);

int findRange(char* country,char* disease,HashTable* diseaseHashtable,HashTable* countryHashtable,char* FirstDate,char* SecondDate,int range);

void countRanges(char* disease,int* count,Treenode * root,char* FirstDate,char* SecondDate,int range);