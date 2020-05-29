#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashtable.h"


void diseaseFrequency(char* country,char* virus,HashTable* diseaseHashtable,HashTable* countryHashtable,char FirstDate[strlen(DATE)+1],char SecondDate[strlen(DATE)+1]);

void countTreePatients(int* count,Treenode * root,char FirstDate[strlen(DATE)+1],char SecondDate[strlen(DATE)+1]);

void countFrequency(char* virus,int* count,Treenode * root,char FirstDate[strlen(DATE)+1],char SecondDate[strlen(DATE)+1]);