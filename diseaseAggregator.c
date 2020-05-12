#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#define BUFFER 100

int main(int argc, char const *argv[]){
	
	int numWorkers, bufferSize;
	char input_dir[30];
	pid_t pid;

	/*---------------------------- Read from the input -------------------------------*/

	for(int i=0; i<argc;i++){
		if(!strcmp(argv[i],"-w"))
			numWorkers = atoi(argv[i+1]);
		if(!strcmp(argv[i],"-i"))
			strcpy(input_dir,argv[i+1]);
		if(!strcmp(argv[i],"-b"))
			bufferSize = atoi(argv[i+1]);
	}
	printf("%d %d %s \n",numWorkers,bufferSize,input_dir );

	/*------------------------------- Create Workers -----------------------------------*/

	for(int i=0; i<numWorkers ;i++){
		if(pipe(arrayOfpipes[numOfcoaches-1].fd) == -1){
               perror("pipe");
               exit(1);
        }
        pid = fork();
        
        if(pid == -1){
           perror ("---fork failed---" );
           exit (1) ;
        }
        if(pid != 0){
           printf ("I am the coordinator with process id  %d\n",getpid());
        }else{
           sprintf(nums,"%d",numOfrecords);
           sprintf(numOfcoach,"%d",numOfcoaches-1);
           close(arrayOfpipes[numOfcoaches-1].fd[READ]);
           sprintf(Hcolumnid,"%d",id); 
           sprintf(temp,"%d",arrayOfpipes[numOfcoaches-1].fd[WRITE]);
           execv("worker","worker",NULL);
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
	

	while(strcmp(buff,"/exit")){
		printf("\033[1;36mREQUEST:  \033[0m");
		scanf("%s",buff);

		if(!strcmp(buff,"/listCountries")){
		
		}if(!strcmp(buff,"/diseaseFrequency")){
			scanf("%s %s %s",VirusName,date1,date2);
			ch = getchar();
			if(ch != '\n')
				scanf("%s",diseaseCountry);
			if(!CheckDate(date1,date2)){
				printf("\033[1;31mERROR: \033[0mSomething went wrong with your date input\n");
			}else
				diseaseFrequency(diseaseCountry,VirusName,diseaseHashtable,countryHashtable,date1,date2);		
		}else if(!strcmp(buff,"/topk-AgeRanges")){
			scanf("%s %s",k,country,VirusName,date1,date2);
		
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
		}else if(strcmp(buff,"/exit"))
			printf("Wrong input\n");
	}


	return 0;
}