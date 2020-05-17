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
#define err(mess){fprintf(stderr,"ERROR: %s\n",mess);exit(1);}

typedef struct workerInfo{
	char* writeFifo;
	char* readFifo;
	int writeFd;
	int readFd; 
	pid_t pid;
}workerInfo;

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

		pid = fork();
        
        if(pid == -1)
           	err("---fork failed---" );

        if(pid == 0){
        	workerArray[i]->pid = getpid();
        	sprintf(fifoBuffer,"%d",bufferSize);
           	sprintf(pidfdr,"%d",workerArray[i]->writeFd);
           	sprintf(pidfdw,"%d",workerArray[i]->readFd );
           	execlp("./worker","worker","-wfd",pidfdw,"-rfd",pidfdr,"-b",fifoBuffer,NULL);
        }      
	}

	/*----------------------- Distribute the subdirectories -----------------------------*/

	if((dir = opendir(input_dir)) == NULL)	//open directory
		err("Can not open directory");
	
	int w=0;
	char path[bufferSize];
	memset(path,0,bufferSize);

	int t=0;
	
	while((dir_info=readdir(dir)) != NULL){
		strcpy(path,input_dir); 
		if(!strcmp(dir_info->d_name,".") || !strcmp(dir_info->d_name,".."))	continue;

		if(w == numWorkers)
			w=0;
		strcat(path,"/");
		strcat(path,dir_info->d_name);

		if(write(workerArray[w]->writeFd,path,bufferSize)<0)
			err("Problem in writing");
		w++;     
	}

	for(int i=0; i<numWorkers ;i++){
		if(write(workerArray[i]->writeFd,"stop",bufferSize)<0)
			err("Problem in writing");
	}

	for(int i=0; i<numWorkers ;i++){
		close(workerArray[i]->writeFd);
		close(workerArray[i]->readFd);
		unlink(workerArray[i]->writeFifo);
		unlink(workerArray[i]->readFifo);
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
	

	// while(strcmp(buff,"/exit")){
	// 	printf("\033[1;36mREQUEST:  \033[0m");
	// 	scanf("%s",buff);

	// 	if(!strcmp(buff,"/listCountries")){
		
	// 	}if(!strcmp(buff,"/diseaseFrequency")){
	// 		scanf("%s %s %s",VirusName,date1,date2);
	// 		ch = getchar();
	// 		if(ch != '\n')
	// 			scanf("%s",diseaseCountry);
	// 		if(!CheckDate(date1,date2)){
	// 			printf("\033[1;31mERROR: \033[0mSomething went wrong with your date input\n");
	// 		}else
	// 			diseaseFrequency(diseaseCountry,VirusName,diseaseHashtable,countryHashtable,date1,date2);		
	// 	}else if(!strcmp(buff,"/topk-AgeRanges")){
	// 		scanf("%s %s",k,country,VirusName,date1,date2);
		
	// 	}else if(!strcmp(buff,"/searchPatientRecord")){
	// 		char recordID[32] = "-";
	// 		scanf("%s",recordID);
	// 	}else if(!strcmp(buff,"/numPatientAdmissions")){
	// 		scanf("%s %s %s",VirusName,date1,date2);
	// 		ch = getchar();
	// 		if(ch != '\n')
	// 			scanf("%s",diseaseCountry);
	// 	}else if(!strcmp(buff,"/numPatientDischarges")){
	// 		scanf("%s %s %s",VirusName,date1,date2);
	// 		ch = getchar();
	// 		if(ch != '\n')
	// 			scanf("%s",diseaseCountry);
	// 	}else if(strcmp(buff,"/exit"))
	// 		printf("Wrong input\n");
	// }
	while(wait(NULL)>0);
	closedir(dir);
	for(int i=0; i<numWorkers ;i++){
		free(workerArray[i]->writeFifo);
		free(workerArray[i]->readFifo);
		free(workerArray[i]); 
	}
	return 0;
}