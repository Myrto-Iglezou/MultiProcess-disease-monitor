#include "hashtable.h"

unsigned int hashFunction(const char* str){

   int length  = strlen(str);
   unsigned int hash = 22;

   for(unsigned int i = 0; i < length; str++, i++){
      hash = (*str) + (hash << 6) + (hash << 16) - hash;
   }
   return hash;
}
int createHashTable(HashTable** hashtable,int NumOfEntries){
	int bucketSize = 512;
	*hashtable = malloc(sizeof(HashTable));
	(*hashtable)->listOfBuckets = malloc(NumOfEntries*sizeof(Bucket*));
	for(int i=0; i<NumOfEntries; i++){
		(*hashtable)->listOfBuckets[i] = NULL;
	}

	int RecordSize = bucketSize - sizeof(int) - sizeof(Bucket*) - sizeof(BucketRecord**);		// find the total size for the records 
	(*hashtable)->maxNumOfRecords = RecordSize/sizeof(BucketRecord);	// find how many records can be
	(*hashtable)->NumOfEntries = NumOfEntries;
	if((*hashtable)->maxNumOfRecords<=0)
		return FALSE;
}

void insertToHashTable(HashTable* hashtable,Patient* pat,char* info,int (*comparator)(const void*,const void*)){
	unsigned int hash = hashFunction(info);
	int index = hash%(hashtable->NumOfEntries);		// find the index in the hashtable
	Bucket* curBucket = hashtable->listOfBuckets[index];		// find the right bucket
	BucketRecord* record = CheckHashTable(hashtable,info,index);	// check if the data already exists
	
	if(record!=NULL){		// if it does exists just insert record by the entry date to RBT
		insertion(&(record->root),pat,comparator);
	}else{
		if(curBucket == NULL){		// if the right bucket is empty
			CreateBucket(hashtable,&(hashtable->listOfBuckets[index]),pat,info,comparator);	// create a new one with all the info
		}
		else{
			while(curBucket->next!=NULL)
				curBucket = curBucket->next;

			if((curBucket->currentRecords) < (hashtable->maxNumOfRecords)){			// if there is space in the bucket
				BucketRecord* record = curBucket->records[curBucket->currentRecords];
				CreateRecord(&record,info,pat,comparator);							// create the record
				curBucket->records[curBucket->currentRecords] = record;				// store it in the current bucket
				curBucket->currentRecords = curBucket->currentRecords + 1;			// increase the number of records
			}else{		// if there's no 
				Bucket* newBucket;		
				CreateBucket(hashtable,&newBucket,pat,info,comparator);	// create a new bucket
				curBucket->next = newBucket;					// connected to the old one
			}
		}
	}	
}

void CreateRecord(BucketRecord** record,char* info,Patient* pat,int (*comparator)(const void*,const void*)){
	*record = malloc(sizeof(BucketRecord));				// create a new record
	(*record)->data = malloc((strlen(info)+1)*sizeof(char));
	strcpy((*record)->data,info);				// save the data (disease or country)
	(*record)->root = guard;
	insertion(&((*record)->root),pat,comparator);		// insert the patient's dates into the RBT and save the root to the record
}

void CreateBucket(HashTable* hashtable,Bucket** newBucket,Patient* pat,char* info,int (*comparator)(const void*,const void*)){
	*newBucket = malloc(sizeof(Bucket));		// create a new bucket
	(*newBucket)->next = NULL;
	(*newBucket)->currentRecords = 1;
	(*newBucket)->records = malloc((hashtable->maxNumOfRecords)*sizeof(BucketRecord*));
	BucketRecord* record =  (*newBucket)->records[0];
	CreateRecord(&record,info,pat,comparator);		// create the record
	(*newBucket)->records[0] = record;
}

BucketRecord* CheckHashTable(HashTable* hashtable,char* info,int index){
	Bucket* curBucket = hashtable->listOfBuckets[index];
	BucketRecord* record;
	if(curBucket == NULL)
		return NULL;

	while(curBucket!=NULL){		// check the bucket list for the record data asked
		for(int i=0 ; i<curBucket->currentRecords ; i++){
			record = curBucket->records[i];
			if(!strcmp(record->data,info))
				return record;
		}
		curBucket = curBucket->next;
	}
	return NULL;
}

void DeleteHashTable(HashTable* hashtable){
	Bucket* curBucket,*tempBucket;
	BucketRecord* record;

	for(int i=0; i<hashtable->NumOfEntries; i++){
		curBucket = hashtable->listOfBuckets[i];
	
		while(curBucket!=NULL){
			for(int j=0 ; j<curBucket->currentRecords ; j++){
				record = curBucket->records[j];
				free(record->data);
				deleteTreeNoData(record->root);
				free(record);
			}
			tempBucket = curBucket;
			curBucket = curBucket->next;
			free(tempBucket->records);
			free(tempBucket);
		}
	}
	free(hashtable->listOfBuckets);
	free(hashtable);
}

int findData(HashTable* hashtable,char* data,BucketRecord** record){		// find the record with this data 
	Bucket* curBucket,*tempBucket;
	unsigned int hash = hashFunction(data);
	int index = hash%(hashtable->NumOfEntries);
	int flag = FALSE;
	curBucket =  hashtable->listOfBuckets[index];
	
	tempBucket = curBucket;
	while(tempBucket!=NULL){
		for(int j=0 ; j<tempBucket->currentRecords ; j++){		// find the virus
			*record = tempBucket->records[j];
			if(!strcmp((*record)->data,data)){
				return TRUE;
			}
		}
		curBucket = tempBucket;
		tempBucket = tempBucket->next;
	}
	return FALSE;
}