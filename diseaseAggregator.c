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

int main(int argc, char const *argv[]){
	
	int numWorkers, bufferSize;
	char input_dir[30];
	pid_t pid;
	DIR * dir;
	struct dirent * dir_info;
	int numOfdir=0;
	char filed[10];
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

	char *fifo[numWorkers];
	int fd[numWorkers];
	char temp[5];

	// if(read(fd,buffer,bufferSize+1)<0)
	// 	err("Problem in reading")


	for(int i=0; i<numWorkers ;i++){
		sprintf(temp,"%d",i);
           	
		fifo[i] = temp;
		
		if(mkfifo(fifo[i],0666) == -1)
			err("No fifo was created");

		pid = fork();
        
        if(pid == -1){
           	perror ("---fork failed---" );
           	exit (1) ;
        }
        if(pid != 0){
           	printf ("I am the parent with process id  %d\n",getpid());  	
           	// close(fd[i]);
        }else{
        	printf ("I am the child with process id  %d\n",getpid());
           	
        	sprintf(fifoBuffer,"%d",bufferSize);
           	// sprintf(filed,"%d",fd[i]);
           	execlp("./worker","worker","-fn",fifo[i],"-b",fifoBuffer,NULL);
        }
       
	}

	/*----------------------- Distribute the subcategories -----------------------------*/

	if((dir = opendir(input_dir)) == NULL){		//open directory
		err("Can not open directory");
		return 1;
	}
	int w=0;
	char path[bufferSize];
	
	while((dir_info=readdir(dir)) != NULL){
		strcpy(path,input_dir); 
		if(!strcmp(dir_info->d_name,".") || !strcmp(dir_info->d_name,".."))	continue;

		if(w==numWorkers-1)
			w=0;
		strcat(path,"/");
		strcat(path,dir_info->d_name);

		if((fd[w] = open(fifo[w], O_WRONLY)) < 0)
			err("Could not open fifo");
		
		write(fd[w],path,bufferSize+1);
		close(fd[w]);

		w++;     
	}

	for (int i = 0; i < numWorkers; ++i){
		wait(0);
		// close(fd[i]);
		unlink(fifo[i]);
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

	closedir(dir);
	return 0;
}