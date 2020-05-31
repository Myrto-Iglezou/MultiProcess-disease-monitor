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
#define stdinFd 0;

static volatile sig_atomic_t signalPid = -1;

static volatile sig_atomic_t signalPid2 = -1;

static volatile sig_atomic_t SIGUR1Flag = FALSE;

static volatile sig_atomic_t SIGINTFlag = FALSE;

static volatile sig_atomic_t SIGQUITFlag = FALSE;

static volatile sig_atomic_t SIGCHLDFlag = FALSE;

void get_pid(int sig, siginfo_t *info, void* context){
	signalPid = info->si_pid;
	SIGUR1Flag = TRUE;
}

void SIGCHLD_pid(int sig, siginfo_t *info, void* context){
	signalPid2 = info->si_pid;
	SIGCHLDFlag = TRUE;
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
	FILE * fp;
	struct dirent * dir_info;
	int numOfdir=0;
	char pidfdr[10];
	char pidfdw[10];
	char fifoBuffer[10];
	int num;
	char readBuffer[256];
	int total=0, success=0, fail=0;

	/*--------------------------- Handle Signals -------------------------------------*/ 

	static struct sigaction SIGUSR1act,SIGINTact,SIGQUITact,SIGCHLDact;

	sigset_t set;
	sigfillset(&set);
	sigprocmask(SIG_SETMASK, &set,NULL);

	SIGINTact.sa_handler = SIGINTHandler;
	sigfillset(&(SIGINTact.sa_mask));
	SIGQUITact.sa_handler = SIGQUITHandler;
	sigfillset(&(SIGQUITact.sa_mask));
	
	SIGCHLDact.sa_flags = SA_SIGINFO;
	SIGCHLDact.sa_sigaction = SIGCHLD_pid;

	SIGUSR1act.sa_flags = SA_SIGINFO;
	SIGUSR1act.sa_sigaction = get_pid;

	if(sigaction(SIGUSR1,&SIGUSR1act,NULL) == -1)
		err("sigaction error");
	
	if(sigaction(SIGINT,&SIGINTact,NULL) == -1)
		err("sigaction error");
	
	if(sigaction(SIGQUIT,&SIGQUITact,NULL) == -1)
		err("sigaction error");

	if(sigaction(SIGCHLD,&SIGCHLDact,NULL) == -1)
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

	workerInfo* workerArray[numWorkers];
	for(int i=0; i<numWorkers ;i++){
		workerArray[i] = malloc(sizeof(workerInfo));
	}

	char rfifo[5];
	char wfifo[5];

	int workerWfd,workerRfd;

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
	}

	for(int i=0; i<numWorkers ;i++){

		pid = fork();

        if(pid == -1){
           	err("fork failed" );
        }
        else if(pid == 0){
        	sprintf(fifoBuffer,"%d",bufferSize);
           	execlp("./worker","worker","-wfn",workerArray[i]->readFifo,"-rfn",workerArray[i]->writeFifo,"-b",fifoBuffer,NULL);
        }
        else{
        	workerArray[i]->pid = pid;
        	workerArray[i]->num = i;
        }  

	}

	for(int i=0; i<numWorkers ;i++){
		
		while(1){
			if((workerArray[i]->writeFd = open(workerArray[i]->writeFifo, O_WRONLY)) < 0)
				printf("Could not open fifo..");

			if((workerArray[i]->readFd = open(workerArray[i]->readFifo, O_RDONLY)) < 0)
				printf("Could not open fifo++");

			if(workerArray[i]->writeFd > 0 && workerArray[i]->readFd > 0)
				break;
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
	int workerNum, tempFlag;
	int boolArray[numWorkers];
	for (int i = 0; i < numWorkers; i++){
		 boolArray[i] = 0;
	}

	for (int i = 0; i < numWorkers; i++){

		if(read(workerArray[i]->readFd,&numOfloops,sizeof(int))<0)		// numOfloops --> how many statistics are
				err("Problem in reading bytes");

		boolArray[i] = 1;
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
			printStat(&arrayOfStat[i][j]);
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
	struct pollfd *tempfd;
	tempfd = calloc(1,sizeof(struct pollfd));
	tempfd[0].fd = stdinFd;
	tempfd[0].events = POLLIN;

	sigprocmask(SIG_UNBLOCK, &set,NULL);

	printf("\033[1;36mREQUEST:  \033[0m");
	fflush(stdout);

	while(flag){

		strcpy(buff,"-");

		rc = poll(tempfd,1,1);

		if(SIGUR1Flag){

			int workerNum,id;
			id = signalPid;

			workerNum  = findNum(id,workerArray,numWorkers);

			if(read(workerArray[workerNum]->readFd,&numOfloops,sizeof(int))<0)		// numOfloops --> how many statistics are
				err("Problem in reading bytes");

			numOfstat[workerNum]+=numOfloops;
			arrayOfStat[workerNum] = realloc(arrayOfStat[workerNum], numOfstat[workerNum]*sizeof(statistics));

			for (int j = numOfstat[workerNum]-numOfloops-1; j < numOfstat[workerNum]-1; j++){

				savestat(workerArray[workerNum]->readFd,bufferSize,arrayOfStat[workerNum][j].date,sizeof(stat->date));
				savestat(workerArray[workerNum]->readFd,bufferSize,arrayOfStat[workerNum][j].country,sizeof(stat->country));
				savestat(workerArray[workerNum]->readFd,bufferSize,arrayOfStat[workerNum][j].disease,sizeof(stat->disease));

				for (int k = 0; k < 4; k++){
					count=0;

					buffer = malloc(sizeof(int));
					strcpy(buffer,"");

					if(read(workerArray[workerNum]->readFd,&message_size,sizeof(int))<0)
						err("Problem in reading");

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
			SIGUR1Flag = FALSE;
		}
		if(SIGCHLDFlag){

			int workerNum,id;
			id = signalPid2;

			workerNum  = findNum(id,workerArray,numWorkers);
			
			pid = fork();

	        if(pid == -1){
	           	err("fork failed" );
	        }
	        else if(pid == 0){
	        	sprintf(fifoBuffer,"%d",bufferSize);
	           	sprintf(pidfdr,"%d",workerArray[workerNum]->writeFd);
	           	sprintf(pidfdw,"%d",workerArray[workerNum]->readFd);
	           	execlp("./worker","worker","-wfd",pidfdw,"-rfd",pidfdr,"-b",fifoBuffer,NULL);
	        }
	        else{
	        	workerArray[workerNum]->pid = pid;
	        } 

			if(write(workerArray[workerNum]->writeFd,&countriesCounter[workerNum],sizeof(int))<0)
				err("Problem in writing");

	        for (int i = 0; i < countriesCounter[workerNum]; i++){
	       		
	       		strcpy(path,cwd);
				strcat(path,"/");
				strcat(path,input_dir); 
				strcat(path,"/");
				strcat(path,workerArray[workerNum]->countries[i]);

				strPointer = &path[0];
				size = bufferSize;
				message_size = strlen(path);
				if(write(workerArray[workerNum]->writeFd,&message_size,sizeof(int))<0)
					err("Problem in writing");
				count = 0;
				printf("--  %s\n",path );
				while(count < strlen(path)){

					strPointer = &path[0];
					strPointer+=count;
					
					if(((strlen(path)+1)-count)<size){
						size = (strlen(path)+1)-count;					
					}
					strncpy(tempStr,strPointer,size);
					if(write(workerArray[workerNum]->writeFd,tempStr,size)<0)
						err("Problem in writing");
					count+=size;
				}  	
	        }

	       	free(arrayOfStat[workerNum]);

			numOfloops = 0;

			if(read(workerArray[workerNum]->readFd,&numOfloops,sizeof(int))<0)		// numOfloops --> how many statistics are
					err("Problem in reading bytes");

			arrayOfStat[workerNum] = malloc(numOfloops*sizeof(statistics));
			numOfstat[workerNum] = numOfloops;

			for (int j = 0; j < numOfloops; j++){

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
			
			SIGCHLDFlag = FALSE;
		}
		if(SIGINTFlag || SIGQUITFlag){

			char fileName[64];
			char temp[64];
			char request[64];

			for (int i = 0; i < numWorkers; i++){
				kill(workerArray[i]->pid,SIGKILL);
			}	
			while(wait(NULL)>0);

			strcpy(fileName,"log_file.");
			sprintf(temp,"%d",getpid());
			strcat(fileName,temp);

			fp = fopen(fileName,"w+");

			for (int i = 0; i < numWorkers; i++){
				for (int j = 0; j < countriesCounter[i] ; j++){
					fprintf(fp,"%s\n",workerArray[i]->countries[j]);
				}
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
			break;				
		}

		sigprocmask(SIG_SETMASK, &set,NULL);	

		if(rc > 0){
			if((tempfd[0].revents & POLLIN)){
				scanf("%s",buff);

				if(!strcmp(buff,"/listCountries")){
					for (int i = 0; i < numWorkers; i++){
						for (int j = 0; j < countriesCounter[i] ; j++){
							printf("%s %d\n",workerArray[i]->countries[j],workerArray[i]->pid);
						}
					}
				
				}if(!strcmp(buff,"/diseaseFrequency")){
					scanf("%s %s %s",VirusName,date1,date2);
					strcpy(diseaseCountry,"-");
					ch = getchar();
					if(ch != '\n')
						scanf("%s",diseaseCountry);
					if(!CheckDate(date1,date2)){
						printf("Something went wrong with your date input\n");
					}else{
						if(!strcmp(diseaseCountry,"-")){
							int total = 0;
							for (int i = 0; i < numWorkers; i++){
								writeBytes(buff,workerArray[i]->writeFd, bufferSize);
								writeBytes(VirusName,workerArray[i]->writeFd, bufferSize);
								writeBytes(date1,workerArray[i]->writeFd, bufferSize);
								writeBytes(date2,workerArray[i]->writeFd, bufferSize);
								writeBytes(diseaseCountry,workerArray[i]->writeFd, bufferSize);	
							}
							for (int i = 0; i < numWorkers; i++){
								if(read(workerArray[i]->readFd,&message_size,sizeof(int))<0)
									err("Problem in reading bytes.");

								buffer = malloc((message_size+1)*sizeof(char));
								strcpy(buffer,"");
								readBytes(workerArray[i]->readFd,buffer,bufferSize,message_size);
								total += atoi(buffer);		
								free(buffer);
							}
							if(total<0)
								printf("No record for this disease\n");
							else 
								printf("%d\n",total );

						}else{
							workerNum = findWorkerFromCountry(diseaseCountry, workerArray,numWorkers,countriesCounter);
							if(workerNum==-1){
								printf("No such country in the directory\n");
							}
							else{
								writeBytes(buff,workerArray[workerNum]->writeFd, bufferSize);
								writeBytes(VirusName,workerArray[workerNum]->writeFd, bufferSize);
								writeBytes(date1,workerArray[workerNum]->writeFd, bufferSize);
								writeBytes(date2,workerArray[workerNum]->writeFd, bufferSize);
								writeBytes(diseaseCountry,workerArray[workerNum]->writeFd, bufferSize);	

								if(read(workerArray[workerNum]->readFd,&message_size,sizeof(int))<0)
									err("Problem in reading bytes.");

								buffer = malloc((message_size+1)*sizeof(char));
								strcpy(buffer,"");
								readBytes(workerArray[workerNum]->readFd,buffer,bufferSize,message_size);
								if(!strcmp(buffer,"-1"))
									printf("No record for this disease\n");
								else 
									printf("%s\n",buffer );
								free(buffer);	
							}								
						}		
					}
				}else if(!strcmp(buff,"/topk-AgeRanges")){
					scanf("%s %s %s %s %s",k,diseaseCountry,VirusName,date1,date2);

					workerNum = findWorkerFromCountry(diseaseCountry, workerArray,numWorkers,countriesCounter);
					if(workerNum==-1){
						printf("No such country in the directory\n");
					}else if(!CheckDate(date1,date2)){
						printf("Something went wrong with your date input\n");
					}else{
						writeBytes(buff,workerArray[workerNum]->writeFd, bufferSize);
						writeBytes(k,workerArray[workerNum]->writeFd, bufferSize);	
						writeBytes(diseaseCountry,workerArray[workerNum]->writeFd, bufferSize);	
						writeBytes(VirusName,workerArray[workerNum]->writeFd, bufferSize);
						writeBytes(date1,workerArray[workerNum]->writeFd, bufferSize);
						writeBytes(date2,workerArray[workerNum]->writeFd, bufferSize);

						if(read(workerArray[workerNum]->readFd,&message_size,sizeof(int))<0)
							err("Problem in reading bytes.");

						buffer = malloc((message_size+1)*sizeof(char));
						strcpy(buffer,"");
						readBytes(workerArray[workerNum]->readFd,buffer,bufferSize,message_size);

						if(!strcmp(buffer,"-1")){
							printf("No record for this disease\n");
							free(buffer);
						}else{
							free(buffer);
							if(atoi(k)>4)			// if the number requested is greater than the number of the ranges
								strcpy(k,"4");
							for (int i = 0; i < atoi(k); i++){

								if(read(workerArray[workerNum]->readFd,&message_size,sizeof(int))<0)
									err("Problem in reading bytes.");

								buffer = malloc((message_size+1)*sizeof(char));
								strcpy(buffer,"");
								readBytes(workerArray[workerNum]->readFd,buffer,bufferSize,message_size);
								if(!strcmp(buffer,"0"))
									printf("0-20: ");
								else if(!strcmp(buffer,"1"))
									printf("21-40: ");
								else if(!strcmp(buffer,"2"))
									printf("41-60: ");
								else if(!strcmp(buffer,"3"))
									printf("60+: ");

								free(buffer);
								if(read(workerArray[workerNum]->readFd,&message_size,sizeof(int))<0)
									err("Problem in reading bytes.");

								buffer = malloc((message_size+1)*sizeof(char));
								strcpy(buffer,"");
								readBytes(workerArray[workerNum]->readFd,buffer,bufferSize,message_size);
								float temp = atof(buffer)*100;
								printf("%0.2f %% \n",temp );
								free(buffer);
							}
						}
					}
						
				}else if(!strcmp(buff,"/searchPatientRecord")){
					char recordID[32] = "-";
					scanf("%s",recordID);
					tempFlag = FALSE;
					for (int i = 0; i < numWorkers; i++){
						writeBytes(buff,workerArray[i]->writeFd,bufferSize);
						writeBytes(recordID,workerArray[i]->writeFd,bufferSize);
					}
					for (int i = 0; i < numWorkers; i++){
						if(read(workerArray[i]->readFd,&message_size,sizeof(int))<0)
							err("Problem in reading bytes");

						buffer = malloc((message_size+1)*sizeof(char));
						strcpy(buffer,"");
						readBytes(workerArray[i]->readFd,buffer,bufferSize,message_size);
						if(!strcmp(buffer,"1")){
							free(buffer);
							char country[40],patientFirstName[30],patientLastName[30],disease[20],age[5];
							
							for (int k = 0; k < 7; k++){
								if(read(workerArray[i]->readFd,&message_size,sizeof(int))<0)
								err("Problem in reading bytes");

								buffer = malloc((message_size+1)*sizeof(char));
								strcpy(buffer,"");
								readBytes(workerArray[i]->readFd,buffer,bufferSize,message_size);
								if(k==0)
									strcpy(patientFirstName,buffer);
								else if(k==1)
									strcpy(patientLastName,buffer);
								else if(k==2)
									strcpy(age,buffer);
								else if(k==3)
									strcpy(disease,buffer);
								else if(k==4)
									strcpy(country,buffer);
								else if(k==5)
									strcpy(date1,buffer);
								else if(k==6)
									strcpy(date2,buffer);
								free(buffer);
							}
							printf("\n%s %s %s %s %s %s %s %s\n\n",recordID,patientFirstName,patientLastName,age,disease,country,date1,date2 );
							tempFlag = TRUE;
							
						}else
							free(buffer);
					}
					if(!tempFlag)
						printf("\nNo record with id %s\n\n",recordID );

				}else if(!strcmp(buff,"/numPatientAdmissions") || !strcmp(buff,"/numPatientDischarges")){ // /numPatientAdmissions H1N1 12-2-2000 12-2-2009
					scanf("%s %s %s",VirusName,date1,date2);
					strcpy(diseaseCountry,"-");
					ch = getchar();
					if(ch != '\n')
						scanf("%s",diseaseCountry);
					if(!CheckDate(date1,date2)){
						printf("Something went wrong with your date input\n");
					}else if(!strcmp(diseaseCountry,"-")){
						for (int i = 0; i < numWorkers; i++){
							writeBytes(buff,workerArray[i]->writeFd, bufferSize);
							writeBytes(VirusName,workerArray[i]->writeFd, bufferSize);
							writeBytes(date1,workerArray[i]->writeFd, bufferSize);
							writeBytes(date2,workerArray[i]->writeFd, bufferSize);
							writeBytes(diseaseCountry,workerArray[i]->writeFd, bufferSize);		
						}
						for (int i = 0; i < numWorkers; i++){
							for (int k = 0; k < countriesCounter[i]; k++){
								for (int j = 0; j < 2; j++){
									if(read(workerArray[i]->readFd,&message_size,sizeof(int))<0)
									err("Problem in reading bytes.");

									buffer = malloc((message_size+1)*sizeof(char));
									strcpy(buffer,"");
									readBytes(workerArray[i]->readFd,buffer,bufferSize,message_size);
									if(!strcmp(buffer,"-1"))
										printf("0\n");
									else 
										printf("%s ",buffer );
									free(buffer);	
								}
								printf("\n");
							}
						}
					}else{
						workerNum = findWorkerFromCountry(diseaseCountry, workerArray,numWorkers,countriesCounter);
						if(workerNum==-1){
							printf("No such country in the directory\n");
						}
						else{
							writeBytes(buff,workerArray[workerNum]->writeFd, bufferSize);
							writeBytes(VirusName,workerArray[workerNum]->writeFd, bufferSize);
							writeBytes(date1,workerArray[workerNum]->writeFd, bufferSize);
							writeBytes(date2,workerArray[workerNum]->writeFd, bufferSize);
							writeBytes(diseaseCountry,workerArray[workerNum]->writeFd, bufferSize);	

							if(read(workerArray[workerNum]->readFd,&message_size,sizeof(int))<0)
								err("Problem in reading bytes.");

							buffer = malloc((message_size+1)*sizeof(char));
							strcpy(buffer,"");
							readBytes(workerArray[workerNum]->readFd,buffer,bufferSize,message_size);
							if(!strcmp(buffer,"-1"))
								printf("No record for this disease\n");
							else 
								printf("%s %s\n",diseaseCountry,buffer );
							free(buffer);	
						}								
					}		
				}else if(!strcmp(buff,"/exit")){
					flag = FALSE;
					for (int i = 0; i < numWorkers; i++){
						kill(workerArray[i]->pid,SIGKILL);
					}
				}
				else{
					ch = getchar();
					while(ch != '\n'){
						scanf("%s",diseaseCountry);
						ch = getchar();
					}
					printf("Wrong input\n");
				}
				if(strcmp(buff,"/exit")){
					printf("\033[1;36mREQUEST:  \033[0m");
					fflush(stdout);
				}
			}
		}				
		sigprocmask(SIG_UNBLOCK, &set,NULL);
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
	free(tempfd);
	return 0;
}