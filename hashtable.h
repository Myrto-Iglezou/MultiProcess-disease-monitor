#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "RBT.h"
#include "comparators.h" 
#define TRUE 1
#define FALSE 0

typedef struct BucketRecord{
	char* data;
	Treenode* root;
}BucketRecord;

typedef struct Bucket{
	struct Bucket* next;
	int currentRecords;
	BucketRecord** records;
}Bucket;

typedef struct table{
	Bucket** listOfBuckets;
	int maxNumOfRecords;
	int NumOfEntries;
}HashTable;

unsigned int hashFunction(const char* str);
int createHashTable(HashTable** start,int bucketSize);
void insertToHashTable(HashTable* start,Patient* pat,char* info,int (*comparator)(const void*,const void*));
void CreateRecord(BucketRecord** record,char* info,Patient* pat,int (*comparator)(const void*,const void*));
void CreateBucket(HashTable* hashtable,Bucket** newBucket,Patient* pat,char* info,int (*comparator)(const void*,const void*));
BucketRecord* CheckHashTable(HashTable* hashtable,char* info,int hash);
void DeleteHashTable(HashTable* hashtable);
int findData(HashTable* hashtable,char* data,BucketRecord** record);