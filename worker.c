#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <dirent.h>
#include <fcntl.h>
#include "utils.h"
#define TRUE 1
#define FALSE 0
#define err(mess){fprintf(stderr,"ERROR: %s\n",mess);exit(1);}

int main(int argc, char const *argv[]){

	int rfd,wfd,bufferSize;
	char readFifo[32];
	char writeFifo[32];
	int num,k=0;
	DIR * dir;
	FILE * fp;
	struct dirent * dir_info;
	int n=0,count=0;
	char path[100];
	char country[40],recordID[10],state[10],patientFirstName[30],patientLastName[30],disease[20],age[5];
	Patient* pat;
	guard = createGuard();
	Treenode * root = guard;
	Treenode * tempNode;
	char* date1 = malloc((strlen(DATE)+1)*sizeof(char));
	char* date2 = malloc((strlen(DATE)+1)*sizeof(char));
	char* buffer;
	char readBuffer[256];
	int message_size;
	int countBytes=0;
	int numOfFiles=0;
	int maxFiles=0;
	char* token;
	char tempstr[256];
	HashTable* diseaseHashtable = NULL;
	HashTable* countryHashtable = NULL;
	HashTable* tableForStatistics = NULL;
	int numOfentries = 20;
	char* strPointer;
	char message[256];
	statistics * stat;
	statistics * arrayOfStat;
	arrayOfStat = malloc(sizeof(statistics));

	/*---------------------------- Read from the input -------------------------------*/

	for(int i=0; i<argc;i++){
		
		if(!strcmp(argv[i],"-rfd"))
			rfd = atoi(argv[i+1]);
		if(!strcmp(argv[i],"-wfd"))
			wfd = atoi(argv[i+1]);
		if(!strcmp(argv[i],"-b"))
			bufferSize = atoi(argv[i+1]);
	}

	if(read(rfd,&maxFiles,sizeof(int))<0)
		err("Problem in reading bytes");

		/*----------------------- Create the two hashtables -----------------------*/

	if(createHashTable(&diseaseHashtable,numOfentries)==FALSE){
		printf("error\n");
		//printf("\033[1;31mERROR: \033[0m: there's no space for a record in the disease hashtable\n");
		DeleteHashTable(diseaseHashtable);
		free(guard);
		return 0;
	}
	if(createHashTable(&countryHashtable, maxFiles)==FALSE){
		printf("error\n");
		//printf("\033[1;31mERROR: \033[0m: there's no space for a record in the country hashtable\n");
		DeleteHashTable(diseaseHashtable);
		DeleteHashTable(countryHashtable);
		free(guard);
		return 0;
	}

	while(numOfFiles<maxFiles){

		if(read(rfd,&message_size,sizeof(int))<0)
			err("Problem in reading bytes");

		buffer = malloc((message_size+1)*sizeof(char));
		strcpy(buffer,"");
		countBytes=0;
		while(countBytes<message_size){			
			if((num = read(rfd,readBuffer,bufferSize))<0)
				err("Problem in reading!");
			strncat(buffer,readBuffer,num);
			countBytes = strlen(buffer);
		}

		if((dir = opendir(buffer)) == NULL)	//open directory
			err("Can not open directory");
		n=0;
		count=0;
		while((dir_info = readdir(dir)) != NULL){
			count++;
		}
		char *namelist[count];
		rewinddir(dir);
		while((dir_info = readdir(dir)) != NULL){
			strcpy(path,buffer); 
			if(!strcmp(dir_info->d_name,".") || !strcmp(dir_info->d_name,".."))	continue;
			namelist[n] = malloc((strlen(DATE)+1)*sizeof(char)); 
			strcpy(namelist[n],dir_info->d_name);
			n++;
		}
		
		qsort(&namelist[0],n-1,sizeof(char*),CompareDates);	

		for(int i=0 ; i<n-1 ; i++){

			if(createHashTable(&tableForStatistics,numOfentries)==FALSE){
				printf("error\n");
				//printf("\033[1;31mERROR: \033[0m: there's no space for a record in the disease hashtable\n");
				DeleteHashTable(tableForStatistics);
				free(guard);
				return 0;
			}
			strcpy(path,buffer); 
			strcat(path,"/");

			strcpy(tempstr,path);
			token = strtok(tempstr,"/");
			while(token !=NULL){
				strcpy(country,token);
				token = strtok(NULL,"/");
			}
			
			strcat(path,namelist[i]);
			
			if((fp = fopen(path,"r"))== NULL)	//open file
				err("Can not open file");

			while(!feof(fp)){
				fscanf(fp,"%s %s %s %s %s %s \n",recordID,state,patientFirstName,patientLastName,disease,age);
				if(!strcmp(state,"ENTER")){
					strcpy(date1,namelist[i]);
					strcpy(date2,"-");
				}else if(!strcmp(state,"EXIT")){
					strcpy(date2,namelist[i]);
					strcpy(date1,"-");
				}
				
				pat =  createPatient(recordID,patientFirstName,patientLastName,disease,country,date1,date2,atoi(age));
				tempNode = FindData(root,pat,ComparePatientsID);
				if(tempNode==guard && !strcmp(state,"ENTER")){
					insertion(&root,pat,ComparePatientsID);
					insertToHashTable(diseaseHashtable,pat,pat->disease,ComparePatientsDates);
					insertToHashTable(countryHashtable,pat,pat->country,ComparePatientsDates);
					insertToHashTable(tableForStatistics,pat,pat->disease,CompareAges);
				}else if(tempNode!=guard && !strcmp(state,"EXIT")){
					deletePatient(pat);
					pat = tempNode->data;
					strcpy(date1,pat->entryDate);
					if(CompareDates(&date1,&date2)<=0)
						strcpy(pat->exitDate,date2);
				}else	
					deletePatient(pat);
			}
			
			fclose(fp);
			stat = malloc(sizeof(statistics));
			strcpy(stat->date,namelist[i]);
			strcpy(stat->country,country);
			for(int i=0 ; i<4 ; i++){
				stat->ranges[i] = 0;
			}
			Bucket* curBucket;
			BucketRecord* record;
			count=0;

			for(int i=0; i<tableForStatistics->NumOfEntries; i++){			// for every disease in the hashtable
				curBucket = tableForStatistics->listOfBuckets[i];
				while(curBucket!=NULL){
					for(int j=0 ; j<curBucket->currentRecords ; j++){
						for(int i=0 ; i<4 ; i++)
							stat->ranges[i] = 0;
						count=0;
						record = curBucket->records[j];
						strcpy(stat->disease,record->data);
						findRanges(&stat,record);
						arrayOfStat = realloc(arrayOfStat,(k+1)*sizeof(statistics));
						memset(&arrayOfStat[k],0,sizeof(statistics));
						arrayOfStat[k] = *stat;
						k++;
					}
					curBucket = curBucket->next;
				}
			}		
			DeleteHashTable(tableForStatistics);
			free(stat);
		}
		for (int i = 0; i < n; i++){
			free(namelist[i]);
		}		
		free(buffer);
		closedir(dir);
		numOfFiles++;
	}

	/*--------------------------- Send Statistics -------------------------------------*/ 

	statistics * tempstat;
	message_size = k;
	if(write(wfd,&message_size,sizeof(int))<0)
		err("Problem in writing");

	for (int i = 0; i < k; i++){
		tempstat = calloc(1,sizeof(statistics));
		stat = tempstat;
		count=0;
		message_size = sizeof(statistics);

		// memcpy(tempstat,&arrayOfStat[i],sizeof(statistics));	

		tempstat = &arrayOfStat[i];

		while(count < message_size){
			
			tempstat+=count;

			if(write(wfd,tempstat,bufferSize)<0)
				err("Problem in writing");
			count+=bufferSize;
		}
		tempstat = stat;
		free(tempstat);
	}

	/*--------------------------- Clean the memory -------------------------------------*/ 

	// for (int i = 0; i < k-1; ++i){
	// 	free(&arrayOfStat[i]);
	// }
	free(arrayOfStat);

	DeleteHashTable(diseaseHashtable);
	DeleteHashTable(countryHashtable);

	// printTree(root,PrintPatient);
	free(guard);
	deleteTree(root,deletePatient);
	free(date1);free(date2);
	return 0;
}