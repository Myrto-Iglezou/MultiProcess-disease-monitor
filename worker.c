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

static volatile sig_atomic_t SIGUR1Flag = FALSE;

static volatile sig_atomic_t SIGINTFlag = FALSE;

static volatile sig_atomic_t SIGQUITFlag = FALSE;

static volatile sig_atomic_t SIGKILLFlag = FALSE;


void SIGUR1Handler(int sig_num){
	SIGUR1Flag = TRUE;
}

void SIGINTHandler(int sig_num){
	SIGINTFlag = TRUE;
}

void SIGQUITHandler(int sig_num){
	SIGQUITFlag = TRUE;
}

void SIGKILLHandler(int sig_num){
	SIGKILLFlag = TRUE;
}

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
	stat = malloc(sizeof(statistics));

	/*--------------------------- Handle Signals -------------------------------------*/ 

	struct sigaction SIGUSR1act,SIGINTact,SIGQUITact,SIGKILLact;
	SIGKILLact.sa_handler = SIGKILLHandler;
	SIGINTact.sa_handler = SIGINTHandler;
	SIGQUITact.sa_handler = SIGQUITHandler;
	SIGUSR1act.sa_handler = SIGUR1Handler;

	if(sigaction(SIGUSR1,&SIGUSR1act,NULL) == -1)
		err("sigaction error");
	
	if(sigaction(SIGKILL,&SIGKILLact,NULL) == -1)
		err("sigaction error");
	
	if(sigaction(SIGINT,&SIGINTact,NULL) == -1)
		err("sigaction error");
	
	if(sigaction(SIGQUIT,&SIGQUITact,NULL) == -1)
		err("sigaction error");
	

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
						arrayOfStat = (statistics*) realloc(arrayOfStat,(k+1)*sizeof(statistics));
						memset(&arrayOfStat[k],0,sizeof(statistics));
						arrayOfStat[k] = *stat;
						k++;
					}
					curBucket = curBucket->next;
				}
			}		
			DeleteHashTable(tableForStatistics);
		}
		for (int i = 0; i < n; i++){
			free(namelist[i]);
		}		
		free(buffer);
		closedir(dir);
		numOfFiles++;
	}

	/*--------------------------- Send Statistics -------------------------------------*/ 

	printf("-- Send statistics id: %d \n",getpid());
	message_size = k;
	int size;
	memset(tempstr,0,sizeof(tempstr));

	if(write(wfd,&message_size,sizeof(int))<0)
		err("Problem in writing");

	for (int i = 0; i < k; i++){
		
		count=0;
		strPointer = arrayOfStat[i].date;
		size = bufferSize;

		message_size = strlen(arrayOfStat[i].date);

		if(write(wfd,&message_size,sizeof(int))<0)
			err("Problem in writing");

		while(count < message_size){
			
			if(((strlen(arrayOfStat[i].date)+1)-count)<size){
				size = (strlen(arrayOfStat[i].date)+1)-count;					
			}
			strncpy(tempstr,strPointer,size);
			// printf("%s\n",tempstr );
			if(write(wfd,tempstr,size)<0)
				err("Problem in writing");
			count+=size;
			strPointer+=size;
		}

		count=0;
		strPointer = arrayOfStat[i].country;
		size = bufferSize;

		message_size = strlen(arrayOfStat[i].country);

		if(write(wfd,&message_size,sizeof(int))<0)
			err("Problem in writing");

		while(count < message_size){
			
			if(((strlen(arrayOfStat[i].country)+1)-count)<size){
				size = (strlen(arrayOfStat[i].country)+1)-count;					
			}
			strncpy(tempstr,strPointer,size);
			// printf("%s\n",tempstr );
			if(write(wfd,tempstr,size)<0)
				err("Problem in writing");
			count+=size;
			strPointer+=size;
		}

		count=0;
		strPointer = arrayOfStat[i].disease;
		size = bufferSize;

		message_size = strlen(arrayOfStat[i].disease);

		if(write(wfd,&message_size,sizeof(int))<0)
			err("Problem in writing");

		while(count < message_size){
			
			if(((strlen(arrayOfStat[i].disease)+1)-count)<size){
				size = (strlen(arrayOfStat[i].disease)+1)-count;					
			}
			strncpy(tempstr,strPointer,size);
			// printf("%s\n",tempstr );
			if(write(wfd,tempstr,size)<0)
				err("Problem in writing");
			count+=size;
			strPointer+=size;
		}

		buffer = malloc(sizeof(int));

		for (int k = 0; k < 4; k++){

			count=0;
			sprintf(buffer,"%d",arrayOfStat[i].ranges[k]);
			strPointer = buffer;
			size = bufferSize;

			message_size = strlen(buffer);

			if(write(wfd,&message_size,sizeof(int))<0)
				err("Problem in writing");

			while(count < message_size){
				
				strncpy(tempstr,strPointer,size);
				// printf("%s\n",tempstr );
				if(write(wfd,tempstr,size)<0)
					err("Problem in writing");
				count+=size;
				strPointer+=size;
			}
		}
		free(buffer);
	}

	free(stat);

	

	while(1){

		if(SIGUR1Flag){

		}

		if(SIGINTFlag){

		}

		if(SIGQUITFlag){
			
		}

		if(SIGKILLFlag){
			free(arrayOfStat);

			DeleteHashTable(diseaseHashtable);
			DeleteHashTable(countryHashtable);

			// printTree(root,PrintPatient);
			free(guard);
			deleteTree(root,deletePatient);
			free(date1);free(date2);
			return 0;

		}

	}
	
}