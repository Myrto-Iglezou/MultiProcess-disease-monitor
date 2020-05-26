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
#include <poll.h>
#include "utils.h"
#define err(mess){fprintf(stderr,"ERROR: %s\n",mess);exit(1);}

typedef struct workerInfo{
	char* writeFifo;
	char* readFifo;
	char** countries;
	int writeFd;
	int readFd; 
	pid_t pid;
	int num;
}workerInfo;

int findNum(int id,workerInfo ** array,int numWorkers){
	for(int i=0; i<numWorkers ;i++){
		if(array[i]->pid == id)
			return i;
	}
}

static volatile sig_atomic_t signalPid = -1;

static volatile sig_atomic_t SIGUR1Flag = FALSE;

static volatile sig_atomic_t SIGINTFlag = FALSE;

static volatile sig_atomic_t SIGQUITFlag = FALSE;

void get_pid(int sig, siginfo_t *info, void* context){
	signalPid = info->si_pid;
	SIGUR1Flag = TRUE;
}

void SIGINTHandler(int sig_num){
	SIGINTFlag = TRUE;
}

void SIGQUITHandler(int sig_num){
	SIGQUITFlag = TRUE;
}

int main(int argc, char const *argv[]){
	
	int numWorkers, bufferSize;
	char input_dir[30];
	pid_t pid;
	DIR * dir;
	struct dirent * dir_info;
	int numOfdir=0;
	char pidfdr[10];
	char pidfdw[10];
	char fifoBuffer[10];
	int num;
	char readBuffer[256];

	/*--------------------------- Handle Signals -------------------------------------*/ 

	static struct sigaction SIGUSR1act,SIGINTact,SIGQUITact;

	SIGINTact.sa_handler = SIGINTHandler;
	sigfillset(&(SIGINTact.sa_mask));
	SIGQUITact.sa_handler = SIGQUITHandler;
	sigfillset(&(SIGQUITact.sa_mask));

	SIGUSR1act.sa_flags = SA_SIGINFO;
	SIGUSR1act.sa_sigaction = get_pid;

	// sigfillset(&(SIGUSR1act.sa_mask));

	if(sigaction(SIGUSR1,&SIGUSR1act,NULL) == -1)
		err("sigaction error");
	
	if(sigaction(SIGINT,&SIGINTact,NULL) == -1)
		err("sigaction error");
	
	if(sigaction(SIGQUIT,&SIGQUITact,NULL) == -1)
		err("sigaction error");
	

	/*---------------------------- Read from the input -------------------------------*/

	for(int i=0; i<argc;i++){
		if(!strcmp(argv[i],"-w"))
			numWorkers = atoi(argv[i+1]);
		if(!strcmp(argv[i],"-i"))
			strcpy(input_dir,argv[i+1]);
		if(!strcmp(argv[i],"-b"))
			bufferSize = atoi(argv[i+1]);
	}
	if(bufferSize<0 || numWorkers<0){
		err("Wrong input");
	}

	/*------------------------------- Create Workers -----------------------------------*/

	struct pollfd fdarray[numWorkers];

	workerInfo* workerArray[numWorkers];
	for(int i=0; i<numWorkers ;i++){
		workerArray[i] = malloc(sizeof(workerInfo));
	}

	char rfifo[5];
	char wfifo[5];

	for(int i=0; i<numWorkers ;i++){
		sprintf(rfifo,"%d",i);
		strcat(rfifo,"r");
		sprintf(wfifo,"%d",i);
		strcat(wfifo,"w");

        workerArray[i]->writeFifo = malloc((strlen(rfifo)+1)*sizeof(char));
		strcpy( workerArray[i]->writeFifo,rfifo);

		workerArray[i]->readFifo = malloc((strlen(wfifo)+1)*sizeof(char));
		strcpy( workerArray[i]->readFifo,wfifo);
		
		if(mkfifo(workerArray[i]->writeFifo ,0666) == -1)
			err("No fifo was created");

		if(mkfifo(workerArray[i]->readFifo,0666) == -1)
			err("No fifo was created");

		if((workerArray[i]->writeFd = open(workerArray[i]->writeFifo, O_RDWR | O_NONBLOCK)) < 0)
			err("Could not open fifo");

		if((workerArray[i]->readFd = open(workerArray[i]->readFifo, O_RDWR )) < 0)
			err("Could not open fifo");
	}

	for(int i=0; i<numWorkers ;i++){

		pid = fork();

        if(pid == -1){
           	err("fork failed" );
        }
        else if(pid == 0){
        	sprintf(fifoBuffer,"%d",bufferSize);
           	sprintf(pidfdr,"%d",workerArray[i]->writeFd);
           	sprintf(pidfdw,"%d",workerArray[i]->readFd);
           	fdarray[i].fd  =  workerArray[i]->readFd;

           	execlp("./worker","worker","-wfd",pidfdw,"-rfd",pidfdr,"-b",fifoBuffer,NULL);
        }
        else{
        	workerArray[i]->pid = pid;
        	workerArray[i]->num = i;
        }  

	}
	
	/*----------------------- Distribute the subdirectories -----------------------------*/

	if((dir = opendir(input_dir)) == NULL)	//open directory
		err("Can not open directory");
	
	int w=0;
	char path[256];
	memset(path,0,bufferSize);
	char* strPointer;
	char tempStr[256];
	int size;
	int message_size;
	memset(tempStr,0,sizeof(tempStr));


	int count=0;
	char cwd[256];
	getcwd(cwd,sizeof(cwd));
	int numOffiles = 0;

	while((dir_info = readdir(dir)) != NULL){
		if(!strcmp(dir_info->d_name,".") || !strcmp(dir_info->d_name,".."))	continue;
		numOffiles++;
	}
	rewinddir(dir);
	int filesPerWorker = numOffiles/numWorkers;
	int extraFiles = numOffiles%numWorkers;
	int totalFiles = 0;

	for(int i = 0; i < numWorkers; i++){
		
		totalFiles = filesPerWorker;
		
		if(extraFiles>i)
			totalFiles+=1;

		workerArray[i]->countries = malloc(totalFiles*sizeof(char*));

		for (int j = 0; j < totalFiles; j++){
			workerArray[i]->countries[j] = malloc(sizeof(char));
		}

		if(write(workerArray[i]->writeFd,&totalFiles,sizeof(int))<0)
			err("Problem in writing");
	}

	int countriesCounter[numWorkers];
	for (int i = 0; i < numWorkers; i++){
		countriesCounter[i] = 0;
	}
	while((dir_info=readdir(dir)) != NULL){

		if(!strcmp(dir_info->d_name,".") || !strcmp(dir_info->d_name,".."))	continue;

		count=0;

		if(w == numWorkers)
			w=0;

		workerArray[w]->countries[countriesCounter[w]] = realloc(workerArray[w]->countries[countriesCounter[w]], (strlen(dir_info->d_name)+1)*sizeof(char));
		strcpy(workerArray[w]->countries[countriesCounter[w]],dir_info->d_name);

		countriesCounter[w]++;

		strcpy(path,cwd);
		strcat(path,"/");
		strcat(path,input_dir); 
		strcat(path,"/");
		strcat(path,dir_info->d_name);

		strPointer = &path[0];
		size = bufferSize;
		message_size = strlen(path);
		if(write(workerArray[w]->writeFd,&message_size,sizeof(int))<0)
			err("Problem in writing");

		while(count < (strlen(path))){

			strPointer = &path[0];
			strPointer+=count;
			
			if(((strlen(path)+1)-count)<size){
				size = (strlen(path)+1)-count;					
			}
			strncpy(tempStr,strPointer,size);
			if(write(workerArray[w]->writeFd,tempStr,size)<0)
				err("Problem in writing");
			count+=size;
		}
		w++;     
	}

	/*--------------------------- Receive Statistics -------------------------------------*/ 

	statistics* stat = malloc(sizeof(statistics));
	statistics* arrayOfStat[numWorkers];
	int numOfstat[numWorkers];
	int numOfloops = 0;
	char* buffer;


	for (int i = 0; i < numWorkers; i++){

		if(read(workerArray[i]->readFd,&numOfloops,sizeof(int))<0)		// numOfloops --> how many statistics are
				err("Problem in reading bytes");


		arrayOfStat[i] = malloc(numOfloops*sizeof(statistics));
		numOfstat[i] = numOfloops;


		for (int j = 0; j < numOfloops; j++){

			savestat(workerArray[i]->readFd,bufferSize,arrayOfStat[i][j].date,sizeof(stat->date));
			savestat(workerArray[i]->readFd,bufferSize,arrayOfStat[i][j].country,sizeof(stat->country));
			savestat(workerArray[i]->readFd,bufferSize,arrayOfStat[i][j].disease,sizeof(stat->disease));

			for (int k = 0; k < 4; k++){
				count=0;

				buffer = malloc(sizeof(int));
				strcpy(buffer,"");

				if(read(workerArray[i]->readFd,&message_size,sizeof(int))<0)
					err("Problem in writing");

				while(count < message_size){

					if((num = read(workerArray[i]->readFd,readBuffer,bufferSize))<0)
						err("Problem in reading!");
					strncat(buffer,readBuffer,num);
					count += bufferSize;
				}
				
				arrayOfStat[i][j].ranges[k] = atoi(buffer);
							
				free(buffer);			
			}
			// printStat(&arrayOfStat[i][j]);
		}
	}

	/*----------------------------------------------------------------------------------*/

	char buff[32] = "-";
	char date1[11] = "-";
	char date2[11] = "-";
	char diseaseCountry[32] = "-";
	char ch;
	char VirusName[32] = "-";
	char k[10];

	/*--------------------- Read the commands till the "/exit" -------------------------*/
	
	int flag = TRUE;
	int rc;

	while(flag){

		if(SIGUR1Flag){

			strcpy(buff,"-");

			int workerNum,id;
			id = signalPid;

			workerNum  = findNum(id,workerArray,numWorkers);

			if(read(workerArray[workerNum]->readFd,&numOfloops,sizeof(int))<0)		// numOfloops --> how many statistics are
				err("Problem in reading bytes");

			numOfstat[workerNum]+=numOfloops;
			arrayOfStat[workerNum] = realloc(arrayOfStat[workerNum], numOfstat[workerNum]*sizeof(statistics));

			for (int j = numOfstat[workerNum]-numOfloops-1; j < numOfstat[workerNum]; j++){

				savestat(workerArray[workerNum]->readFd,bufferSize,arrayOfStat[workerNum][j].date,sizeof(stat->date));
				savestat(workerArray[workerNum]->readFd,bufferSize,arrayOfStat[workerNum][j].country,sizeof(stat->country));
				savestat(workerArray[workerNum]->readFd,bufferSize,arrayOfStat[workerNum][j].disease,sizeof(stat->disease));

				for (int k = 0; k < 4; k++){
					count=0;

					buffer = malloc(sizeof(int));
					strcpy(buffer,"");

					if(read(workerArray[workerNum]->readFd,&message_size,sizeof(int))<0)
						err("Problem in writing");

					while(count < message_size){

						if((num = read(workerArray[workerNum]->readFd,readBuffer,bufferSize))<0)
							err("Problem in reading!");
						strncat(buffer,readBuffer,num);
						count += bufferSize;
					}
					
					arrayOfStat[workerNum][j].ranges[k] = atoi(buffer);
								
					free(buffer);			
				}
				printStat(&arrayOfStat[workerNum][j]);
			}
		}

		printf("\033[1;36mREQUEST:  \033[0m");
		scanf("%s",buff);

		if(!strcmp(buff,"/listCountries")){
			for (int i = 0; i < numWorkers; i++){
				for (int j = 0; j < countriesCounter[i] ; j++){
					printf("%s %d\n",workerArray[i]->countries[j],workerArray[i]->pid);
				}
			}
		
		}if(!strcmp(buff,"/diseaseFrequency")){
			scanf("%s %s %s",VirusName,date1,date2);
			ch = getchar();
			if(ch != '\n')
				scanf("%s",diseaseCountry);
			// if(!CheckDate(date1,date2)){
			// 	printf("\033[1;31mERROR: \033[0mSomething went wrong with your date input\n");
			// }else
			// 	diseaseFrequency(diseaseCountry,VirusName,diseaseHashtable,countryHashtable,date1,date2);		
		}else if(!strcmp(buff,"/topk-AgeRanges")){
			scanf("%s %s %s %s %s",k,diseaseCountry,VirusName,date1,date2);
		
		}else if(!strcmp(buff,"/searchPatientRecord")){
			char recordID[32] = "-";
			scanf("%s",recordID);
		}else if(!strcmp(buff,"/numPatientAdmissions")){
			scanf("%s %s %s",VirusName,date1,date2);
			ch = getchar();
			if(ch != '\n')
				scanf("%s",diseaseCountry);
		}else if(!strcmp(buff,"/numPatientDischarges")){
			scanf("%s %s %s",VirusName,date1,date2);
			ch = getchar();
			if(ch != '\n')
				scanf("%s",diseaseCountry);
		}else if(!strcmp(buff,"/exit")){
			flag = FALSE;
			for (int i = 0; i < numWorkers; i++){
				kill(workerArray[i]->pid,SIGKILL);
			}
		}
		// else{
		// 	printf("Wrong input\n");
		// }
	}

	while(wait(NULL)>0);

	/*--------------------------- Clean the memory -------------------------------------*/ 

	for(int i=0; i<numWorkers ;i++){
		free(arrayOfStat[i]);
		close(workerArray[i]->writeFd);
		close(workerArray[i]->readFd);
		unlink(workerArray[i]->writeFifo);
		unlink(workerArray[i]->readFifo);
	}

	closedir(dir);
	for(int i=0; i<numWorkers ;i++){

		for (int j = 0; j < countriesCounter[i] ; j++){
			free(workerArray[i]->countries[j]);
		}

		free(workerArray[i]->countries); 
		free(workerArray[i]->writeFifo);
		free(workerArray[i]->readFifo);
		free(workerArray[i]); 
	}
	free(stat);
	return 0;
}