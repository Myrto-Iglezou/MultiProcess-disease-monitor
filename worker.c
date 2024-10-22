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
	sigset_t set;
	sigfillset(&set);
	sigprocmask(SIG_SETMASK, &set,NULL);

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
		
		if(!strcmp(argv[i],"-rfn"))
			strcpy(readFifo,argv[i+1]);
		if(!strcmp(argv[i],"-wfn"))
			strcpy(writeFifo,argv[i+1]);
		if(!strcmp(argv[i],"-b"))
			bufferSize = atoi(argv[i+1]);
	}

    while(1){

		rfd = open(readFifo, O_RDONLY );
		wfd = open(writeFifo, O_WRONLY );	

		if(rfd > 0 && wfd > 0)
			break;
	}

	if(read(rfd,&maxFolders,sizeof(int))<0)		// read the number of idrectories he has to read
		err("Problem in reading bytes");

	char* subdirectories[maxFolders]; 

	char* countries[maxFolders]; 

	char** filesPerDir[maxFolders];

	int numOfFilesPerDir[maxFolders];

	/*----------------------- Create the two hashtables -----------------------*/

	if(createHashTable(&diseaseHashtable,numOfentries)==FALSE){
		err("there's no space for a record in the disease hashtable\n");
		DeleteHashTable(diseaseHashtable);
		free(guard);
		return 0;
	}
	if(createHashTable(&countryHashtable, maxFolders)==FALSE){
		err("there's no space for a record in the country hashtable\n");
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

		readBytes(rfd,buffer,bufferSize,message_size);		// read the path for the directory

		if((dir = opendir(buffer)) == NULL)	//open directory
			err("Can not open directory");

		subdirectories[numOfFolders] = malloc((strlen(buffer)+1)*sizeof(char));
		strcpy(subdirectories[numOfFolders],buffer);		// save the path in an array

		n=0;
		numOfFiles=0;

		while((dir_info = readdir(dir)) != NULL){		// count the files in the directory
			numOfFiles++;
		}

		numOfFilesPerDir[numOfFolders] = numOfFiles-2;		// remove 2 for . and ..

		filesPerDir[numOfFolders] = malloc(numOfFiles*sizeof(char*));

		char *namelist[numOfFiles];
		rewinddir(dir);		// move pointer in the start of the directory
		while((dir_info = readdir(dir)) != NULL){
			strcpy(path,buffer); 
			if(!strcmp(dir_info->d_name,".") || !strcmp(dir_info->d_name,".."))	continue;
			
			filesPerDir[numOfFolders][n] = malloc((strlen(DATE)+1)*sizeof(char));	 // save the files in an array of every country
			strcpy(filesPerDir[numOfFolders][n],dir_info->d_name);		// save the names of the files(dates) in an array to sort them and read them
			namelist[n] = malloc((strlen(DATE)+1)*sizeof(char)); 
			strcpy(namelist[n],dir_info->d_name);
			n++;
		}
		
		qsort(&namelist[0],n,sizeof(char*),CompareDates);	// sort the names of the files 

		for(int i=0 ; i<n ; i++){

			if(createHashTable(&tableForStatistics,numOfentries)==FALSE){		// create hash table for the diseases to help find statistics
				err("there's no space for a record in the disease hashtable\n");
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
			strcpy(countries[numOfFolders],country);		// save the country to an array
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
				Patient* temppat  = NULL;
				if(tempNode!=guard)
					temppat = (Patient*) tempNode->data;

				if(tempNode==guard && !strcmp(state,"ENTER")){		// if there isn't a patient with this id and this record's state is Enter add him to the rbt
					insertion(&root,pat,ComparePatientsID);			// insert patient to RBT
					insertToHashTable(diseaseHashtable,pat,pat->disease,ComparePatientsDates);
					insertToHashTable(countryHashtable,pat,pat->country,ComparePatientsDates);
					insertToHashTable(tableForStatistics,pat,pat->disease,CompareAges);
				}else if(tempNode!=guard && !strcmp(state,"EXIT") && !strcmp(temppat->exitDate,"-")){		// if there is a patient with this id with no exit date and this record's state is Exit, add him to the rbt
					deletePatient(pat);
					pat = tempNode->data;
					strcpy(date1,pat->entryDate);
					if(CompareDates(&date1,&date2)<=0)
						strcpy(pat->exitDate,date2);
					else 
						printf("ERROR\n");
				}else{					// in every other case print error
					printf("ERROR\n");
					deletePatient(pat);
				}
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
						arrayOfStat[numOfstat] = *stat;		// save all statistics in an array
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

	for (int i = 0; i < numOfstat; i++){	// send all statistics for every country to the parent
		
		sendStat(arrayOfStat[i].date,bufferSize,wfd);		// date
		sendStat(arrayOfStat[i].country,bufferSize,wfd);	// country
		sendStat(arrayOfStat[i].disease,bufferSize,wfd);	// disease
		
		buffer = malloc(sizeof(int));

		for (int k = 0; k < 4; k++){	// ranges
			
			sprintf(buffer,"%d",arrayOfStat[i].ranges[k]);	
			sendStat(buffer,bufferSize,wfd);	
		}

		free(buffer);
	}
	char diseaseCountry[64];
	char* tempbuffer;

	sigprocmask(SIG_UNBLOCK,&set,NULL);

	while(1){

		while(read(rfd,&message_size,sizeof(int))<0){	// while nothing is read, if a signal comes chech signals

			if(SIGUR1Flag){		// if new files have occurred
				statistics * tempstat = malloc(sizeof(statistics));
				int totalFiles = 0;

				int tempFlag = FALSE;

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
				
					if(newFiles == 0)		// if there are no newfiles continue to the other directories
						continue;

					tempFlag = TRUE;		// flag in case no new file has been added

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
							Patient* temppat  = NULL;
							if(tempNode!=guard)
								temppat = (Patient*) tempNode->data;

							if(tempNode==guard && !strcmp(state,"ENTER")){		// if there isn't a patient with this id and this record's state is Enter add him to the rbt
								insertion(&root,pat,ComparePatientsID);			// insert patient to RBT
								insertToHashTable(diseaseHashtable,pat,pat->disease,ComparePatientsDates);
								insertToHashTable(countryHashtable,pat,pat->country,ComparePatientsDates);
								insertToHashTable(tableForStatistics,pat,pat->disease,CompareAges);
							}else if(tempNode!=guard && !strcmp(state,"EXIT") && !strcmp(temppat->exitDate,"-")){		// if there is a patient with this id with no exit date and this record's state is Exit, add him to the rbt
								deletePatient(pat);
								pat = tempNode->data;
								strcpy(date1,pat->entryDate);
								if(CompareDates(&date1,&date2)<=0)
									strcpy(pat->exitDate,date2);
								else 
									printf("ERROR\n");
							}else{					// in every other case print error
								printf("ERROR\n");
								deletePatient(pat);
							}
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

				if(tempFlag){
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
					kill(getppid(),SIGUSR1);	// send signal to the parent to read the statistics
				}
				SIGUR1Flag = FALSE;			
			}

			if(SIGINTFlag || SIGQUITFlag){

				char fileName[64];
				char temp[64];
				char request[64];

				strcpy(fileName,"log_file.");		// create the log file
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

		sigprocmask(SIG_SETMASK,&set,NULL);

		buffer = malloc((message_size+1)*sizeof(char));
		strcpy(buffer,"");
		readBytes(rfd,buffer,bufferSize,message_size);

		if(!strcmp(buffer,"/diseaseFrequency")){ 
			for (int i = 0; i < 4; i++){		// reacieve all the info from the parent

				if(read(rfd,&message_size,sizeof(int))<0)
					err("Problem in reading bytes");

				tempbuffer = malloc((message_size+1)*sizeof(char));
				strcpy(tempbuffer,"");
				readBytes(rfd,tempbuffer,bufferSize,message_size);

				if(i==0)
					strcpy(disease,tempbuffer);
				else if(i==1)
					strcpy(date1,tempbuffer);
				else if(i==2)
					strcpy(date2,tempbuffer);
				else if(i==3)
					strcpy(country,tempbuffer);
				free(tempbuffer);
			}

			count = diseaseFrequency(country,disease,diseaseHashtable,countryHashtable,date1,date2,FALSE,FALSE);	// find the frequency
			sprintf(buffer,"%d",count);

			if(count>0)
				success++;
			else 
				fail++;

			writeBytes(buffer,wfd,bufferSize);		// send the result  
		}else if(!strcmp(buffer,"/topk-AgeRanges")){
			char k[5];
			heap* newheap;
			createHeap(&newheap);	// create the heap

			for (int i = 0; i < 5; i++){		// read the info

				if(read(rfd,&message_size,sizeof(int))<0)
					err("Problem in reading bytes.");

				tempbuffer = malloc((message_size+1)*sizeof(char));
				strcpy(tempbuffer,"");
				readBytes(rfd,tempbuffer,bufferSize,message_size);

				if(i==0)
					strcpy(k,tempbuffer);
				else if(i==1)
					strcpy(country,tempbuffer);
				else if(i==2)
					strcpy(disease,tempbuffer);
				else if(i==3)
					strcpy(date1,tempbuffer);
				else if(i==4)
					strcpy(date2,tempbuffer);
				free(tempbuffer);

			}

			int countRanges[4];
			int total = 0;
			float per = 0.0;
			tempbuffer = malloc(32*sizeof(char));
			int max;
			char data[10];

			for (int i = 0; i < 4; i++){		// find the num of cases of every range
				countRanges[i] = findRange(country,disease,diseaseHashtable,countryHashtable,date1,date2,i);
				sprintf(tempbuffer,"%d",i);
				insertToHeap(countRanges[i],tempbuffer,newheap);
				total += countRanges[i] ;
			}

			if(total<0){
				strcpy(tempbuffer,"-1");
				writeBytes(tempbuffer,wfd,bufferSize);
			}else{
				strcpy(tempbuffer,"1");
				writeBytes(tempbuffer,wfd,bufferSize);
				if(atoi(k)>4)			// if the number requested is greater than the number of the ranges
					strcpy(k,"4");

				for(int i=0;i<atoi(k);i++){
					getTheMax(data,&max,newheap);		// find the max of the heap
					per = (float) ((float) countRanges[atoi(data)] / (float) total);		// find the percentage
					sprintf(tempbuffer,"%lf",per);
					writeBytes(data,wfd,bufferSize);		// send the range
					writeBytes(tempbuffer,wfd,bufferSize);	// send the percentage
				}
			}
			success++;
			DeleteHeap(newheap,newheap->root);
			free(newheap);
			free(tempbuffer);
			
		}else if(!strcmp(buffer,"/searchPatientRecord")){
			if(read(rfd,&message_size,sizeof(int))<0)
					err("Problem in reading bytes");

			tempbuffer = malloc((message_size+1)*sizeof(char));
			strcpy(tempbuffer,"");
			readBytes(rfd,tempbuffer,bufferSize,message_size);	// read the id of the Record
			strcpy(patientFirstName,"-");
			strcpy(patientLastName,"-");
			strcpy(disease,"-");
			strcpy(country,"-");
			strcpy(date1,"-");
			strcpy(date2,"-");
			strcpy(age,"0");
			pat =  createPatient(tempbuffer,patientFirstName,patientLastName,disease,country,date1,date2,atoi(age));		// create a temporary patient to check the tree
			tempNode = FindData(root,pat,ComparePatientsID);	
			deletePatient(pat);		
			if(tempNode!=guard){		// if it found it send 1 and alla the info of the record
				pat = (Patient*) tempNode->data;
				strcpy(tempbuffer,"1");
				writeBytes(tempbuffer,wfd,bufferSize);
				writeBytes(pat->firstName,wfd,bufferSize);
				writeBytes(pat->lastName,wfd,bufferSize);
				sprintf(buffer,"%d",pat->age);
				writeBytes(buffer,wfd,bufferSize);
				writeBytes(pat->disease,wfd,bufferSize);
				writeBytes(pat->country,wfd,bufferSize);
				writeBytes(pat->entryDate,wfd,bufferSize);
				writeBytes(pat->exitDate,wfd,bufferSize);
				success++;
			}else{		// if i didn't find it send 0; 
				strcpy(tempbuffer,"0");
				writeBytes(tempbuffer,wfd,bufferSize);
				fail++;
			}
			free(tempbuffer);
		}else if(!strcmp(buffer,"/numPatientAdmissions")){ 
			for (int i = 0; i < 4; i++){

				if(read(rfd,&message_size,sizeof(int))<0)
					err("Problem in reading bytes");

				tempbuffer = malloc((message_size+1)*sizeof(char));
				strcpy(tempbuffer,"");
				readBytes(rfd,tempbuffer,bufferSize,message_size);

				if(i==0)
					strcpy(disease,tempbuffer);
				else if(i==1)
					strcpy(date1,tempbuffer);
				else if(i==2)
					strcpy(date2,tempbuffer);
				else if(i==3)
					strcpy(country,tempbuffer);
				free(tempbuffer);
			}
			if(!strcmp(country,"-")){
				for (int i = 0; i < maxFolders; i++){
					writeBytes(countries[i],wfd,bufferSize);
					count = diseaseFrequency(countries[i],disease,diseaseHashtable,countryHashtable,date1,date2,TRUE,FALSE); // find the frequency of the admisions
					sprintf(buffer,"%d",count);

					writeBytes(buffer,wfd,bufferSize);	// send the result

					if(count>0)
						success++;
					else 
						fail++;			
				}
			}else{
				count = diseaseFrequency(country,disease,diseaseHashtable,countryHashtable,date1,date2,TRUE,FALSE);
				sprintf(buffer,"%d",count);

				writeBytes(buffer,wfd,bufferSize);

				if(count>0)
					success++;
				else 
					fail++;
			}
		}else if(!strcmp(buffer,"/numPatientDischarges")){ 
			for (int i = 0; i < 4; i++){

				if(read(rfd,&message_size,sizeof(int))<0)
					err("Problem in reading bytes");

				tempbuffer = malloc((message_size+1)*sizeof(char));
				strcpy(tempbuffer,"");
				readBytes(rfd,tempbuffer,bufferSize,message_size);

				if(i==0)
					strcpy(disease,tempbuffer);
				else if(i==1)
					strcpy(date1,tempbuffer);
				else if(i==2)
					strcpy(date2,tempbuffer);
				else if(i==3)
					strcpy(country,tempbuffer);
				free(tempbuffer);
			}
			if(!strcmp(country,"-")){
				for (int i = 0; i < maxFolders; i++){
					writeBytes(countries[i],wfd,bufferSize);
					count = diseaseFrequency(countries[i],disease,diseaseHashtable,countryHashtable,date1,date2,FALSE,TRUE);  // find the frequency of the dicharges
					sprintf(buffer,"%d",count);

					writeBytes(buffer,wfd,bufferSize);		// send result

					if(count>0)
						success++;
					else 
						fail++;		
				}
			}else{
				count = diseaseFrequency(country,disease,diseaseHashtable,countryHashtable,date1,date2,FALSE,TRUE);
				sprintf(buffer,"%d",count);

				writeBytes(buffer,wfd,bufferSize);

				if(count>0)
					success++;
				else 
					fail++;
			}
		}
		free(buffer);
		sigprocmask(SIG_UNBLOCK, &set,NULL);
	}

	free(arrayOfStat);

	free(stat);	

	close(rfd);
	close(wfd);

	DeleteHashTable(diseaseHashtable);
	DeleteHashTable(countryHashtable);

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