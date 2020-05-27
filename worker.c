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

int main(int argc, char const *argv[]){

	int rfd,wfd,bufferSize;
	char readFifo[32];
	char writeFifo[32];
	int num,numOfstat=0;
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
	int numOfFolders=0;
	int maxFolders=0;
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
	int numOfFiles;
	int total=0, success=0, fail=0;

	/*--------------------------- Handle Signals -------------------------------------*/ 

	static struct sigaction SIGUSR1act,SIGINTact,SIGQUITact,SIGKILLact;

	SIGINTact.sa_handler = SIGINTHandler;
	sigfillset(&(SIGINTact.sa_mask));
	SIGQUITact.sa_handler = SIGQUITHandler;
	sigfillset(&(SIGQUITact.sa_mask));
	SIGUSR1act.sa_handler = SIGUR1Handler;
	sigfillset(&(SIGUSR1act.sa_mask));


	if(sigaction(SIGUSR1,&SIGUSR1act,NULL) == -1)
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

	if(read(rfd,&maxFolders,sizeof(int))<0)
		err("Problem in reading bytes");

	char* subdirectories[maxFolders]; 

	char* countries[maxFolders]; 

	char** filesPerDir[maxFolders];

	int numOfFilesPerDir[maxFolders];

	/*----------------------- Create the two hashtables -----------------------*/

	if(createHashTable(&diseaseHashtable,numOfentries)==FALSE){
		printf("error\n");
		//printf("\033[1;31mERROR: \033[0m: there's no space for a record in the disease hashtable\n");
		DeleteHashTable(diseaseHashtable);
		free(guard);
		return 0;
	}
	if(createHashTable(&countryHashtable, maxFolders)==FALSE){
		printf("error\n");
		//printf("\033[1;31mERROR: \033[0m: there's no space for a record in the country hashtable\n");
		DeleteHashTable(diseaseHashtable);
		DeleteHashTable(countryHashtable);
		free(guard);
		return 0;
	}


	/*----------------------- Read the subdirectories ---------------------------*/

	while(numOfFolders<maxFolders){

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

		subdirectories[numOfFolders] = malloc((strlen(buffer)+1)*sizeof(char));
		strcpy(subdirectories[numOfFolders],buffer);

		n=0;
		numOfFiles=0;

		while((dir_info = readdir(dir)) != NULL){
			numOfFiles++;
		}

		numOfFilesPerDir[numOfFolders] = numOfFiles-2;

		filesPerDir[numOfFolders] = malloc(numOfFiles*sizeof(char*));

		char *namelist[numOfFiles];
		rewinddir(dir);
		while((dir_info = readdir(dir)) != NULL){
			strcpy(path,buffer); 
			if(!strcmp(dir_info->d_name,".") || !strcmp(dir_info->d_name,".."))	continue;
			
			filesPerDir[numOfFolders][n] = malloc((strlen(DATE)+1)*sizeof(char)); 
			strcpy(filesPerDir[numOfFolders][n],dir_info->d_name);
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
			countries[numOfFolders] = malloc((strlen(country)+1)*sizeof(char));
			strcpy(countries[numOfFolders],country);
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
						arrayOfStat = (statistics*) realloc(arrayOfStat,(numOfstat+1)*sizeof(statistics));
						memset(&arrayOfStat[numOfstat],0,sizeof(statistics));
						arrayOfStat[numOfstat] = *stat;
						numOfstat++;
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
		numOfFolders++;
	}

	/*--------------------------- Send Statistics -------------------------------------*/ 

	message_size = numOfstat;

	if(write(wfd,&message_size,sizeof(int))<0)
		err("Problem in writing");

	for (int i = 0; i < numOfstat; i++){
		
		sendStat(arrayOfStat[i].date,bufferSize,wfd);
		sendStat(arrayOfStat[i].country,bufferSize,wfd);
		sendStat(arrayOfStat[i].disease,bufferSize,wfd);
		
		buffer = malloc(sizeof(int));

		for (int k = 0; k < 4; k++){
			
			sprintf(buffer,"%d",arrayOfStat[i].ranges[k]);	
			sendStat(buffer,bufferSize,wfd);	
		}

		free(buffer);
	}

	while(1){

		if(SIGUR1Flag){

			kill(getppid(),SIGUSR1);

			statistics * tempstat = malloc(sizeof(statistics));
			int totalFiles = 0;

			for (int i = 0; i < maxFolders; i++){
				if((dir = opendir(subdirectories[i])) == NULL)	//open directory
					err("Can not open directory");

				count=0;
				int flag = FALSE; 

				while((dir_info = readdir(dir)) != NULL){
					count++;
				}
				count-=2;
				int newFiles = count - numOfFilesPerDir[i];
				if(newFiles == 0)
					continue;

				char *newnamelist[newFiles];
				rewinddir(dir);
				n=0;
				
				while((dir_info = readdir(dir)) != NULL){
					flag = FALSE;
					strcpy(path,subdirectories[i]); 
					if(!strcmp(dir_info->d_name,".") || !strcmp(dir_info->d_name,".."))	continue;

					for (int k = 0; k < numOfFilesPerDir[i]; k++){
						// printf("%s - %s\n",dir_info->d_name, filesPerDir[i][k] );
						if(!strcmp(dir_info->d_name,filesPerDir[i][k]))
							flag = TRUE;
						if(flag)
							break;	
					}
					if(flag == FALSE){
						newnamelist[n] = malloc((strlen(DATE)+1)*sizeof(char)); 
						strcpy(newnamelist[n],dir_info->d_name);
						filesPerDir[i] = realloc(filesPerDir[i], (numOfFilesPerDir[i]+1)*sizeof(char*));
						filesPerDir[i][numOfFilesPerDir[i]] = malloc((strlen(DATE)+1)*sizeof(char)); 
						strcpy(filesPerDir[i][numOfFilesPerDir[i]],dir_info->d_name);
						numOfFilesPerDir[i]++;
						n++;
					}
				}

				qsort(&newnamelist[0],n-1,sizeof(char*),CompareDates);	

				for(int j=0 ; j<newFiles ; j++){

					if(createHashTable(&tableForStatistics,numOfentries)==FALSE){
						err("error in creating hashtable\n");
						DeleteHashTable(tableForStatistics);
						free(guard);
					}

					strcpy(path,subdirectories[i]); 
					strcat(path,"/");

					strcpy(tempstr,path);
					token = strtok(tempstr,"/");
					while(token !=NULL){
						strcpy(country,token);
						token = strtok(NULL,"/");
					}
					
					strcat(path,newnamelist[j]);
					
					if((fp = fopen(path,"r"))== NULL)	//open file
						err("Can not open file");

					while(!feof(fp)){
						fscanf(fp,"%s %s %s %s %s %s \n",recordID,state,patientFirstName,patientLastName,disease,age);
						if(!strcmp(state,"ENTER")){
							strcpy(date1,newnamelist[j]);
							strcpy(date2,"-");
						}else if(!strcmp(state,"EXIT")){
							strcpy(date2,newnamelist[j]);
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
					strcpy(stat->date,newnamelist[j]);
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
								arrayOfStat = (statistics*) realloc(arrayOfStat,(numOfstat+1)*sizeof(statistics));
								tempstat = (statistics*) realloc(tempstat,(totalFiles+1)*sizeof(statistics));
								memset(&arrayOfStat[numOfstat],0,sizeof(statistics));
								arrayOfStat[numOfstat] = *stat;
								tempstat[totalFiles] = *stat;
								numOfstat++;
								totalFiles++;
							}
							curBucket = curBucket->next;
						}
					}	
					DeleteHashTable(tableForStatistics);
					
				}
				for (int i = 0; i < newFiles; i++){
						free(newnamelist[i]);
					}
			}
			message_size = totalFiles;

			if(write(wfd,&message_size,sizeof(int))<0)
				err("Problem in writing");

			for (int i = 0; i < totalFiles; i++){
				
				sendStat(tempstat[i].date,bufferSize,wfd);
				sendStat(tempstat[i].country,bufferSize,wfd);
				sendStat(tempstat[i].disease,bufferSize,wfd);
				
				buffer = malloc(sizeof(int));

				for (int k = 0; k < 4; k++){
					
					sprintf(buffer,"%d",tempstat[i].ranges[k]);	
					sendStat(buffer,bufferSize,wfd);	
				}

				free(buffer);
			}	
			SIGUR1Flag = FALSE;
		}

		if(SIGINTFlag || SIGQUITFlag){

			char fileName[64];
			char temp[64];
			char request[64];

			strcpy(fileName,"log_file.");
			sprintf(temp,"%d",getpid());
			strcat(fileName,temp);

			fp = fopen(fileName,"w+");

			for (int i = 0; i < maxFolders; i++){
				fprintf(fp, "%s\n", countries[i]);				
			}
			strcpy(request,"TOTAL ");
			total = success+fail;
			sprintf(temp,"%d",total);
			strcat(request,temp);
			fprintf(fp, "%s\n", request);				

			strcpy(request,"SUCCESS ");
			sprintf(temp,"%d",success);
			strcat(request,temp);
			fprintf(fp, "%s\n", request);				

			strcpy(request,"FAIL ");
			sprintf(temp,"%d",fail);
			strcat(request,temp);
			fprintf(fp, "%s\n", request);				

			fclose(fp);
			if(SIGQUITFlag){
				SIGQUITFlag = FALSE;
			}else if(SIGINTFlag){
				SIGINTFlag = FALSE;
			}
		}
	}

	free(arrayOfStat);

	free(stat);	

	DeleteHashTable(diseaseHashtable);
	DeleteHashTable(countryHashtable);

	// printTree(root,PrintPatient);
	free(guard);
	free(stat);
	deleteTree(root,deletePatient);
	free(date1);free(date2);
	for (int i = 0; i < maxFolders; i++){
		free(subdirectories[i]);
		free(countries[i]);
	}

	for (int i = 0; i < maxFolders; ++i){
		for (int j = 0; j < n; j++){
			free(filesPerDir[i][j]);
		}
		free(filesPerDir[i]);
	}
	return 0;
}