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
#include "patient.h"

#define err(mess){fprintf(stderr,"ERROR: %s\n",mess);exit(1);}



int main(int argc, char const *argv[]){

	int fd,bufferSize;
	char fifo[32];
	int num;
	DIR * dir;
	FILE * fp;
	struct dirent * dir_info;
	char path[100];
	char recordID[10],state[10],patientFirstName[30],patientLastName[30],disease[20],age[5];
	Patient* pat;

	for(int i=0; i<argc;i++){
		
		if(!strcmp(argv[i],"-fd"))
			fd = atoi(argv[i+1]);
		if(!strcmp(argv[i],"-b"))
			bufferSize = atoi(argv[i+1]);
	}

	char buffer[bufferSize];

	do{
		if((read(fd,buffer,bufferSize))<0){
			err("Problem in reading");
		}else if(strcmp(buffer,"stop")){
			if((dir = opendir(buffer)) == NULL)	//open directory
				err("Can not open directory");
			while((dir_info=readdir(dir)) != NULL){
				strcpy(path,buffer); 
				if(!strcmp(dir_info->d_name,".") || !strcmp(dir_info->d_name,".."))	continue;
					
				strcat(path,"/");
				strcat(path,dir_info->d_name);
				if((fp = fopen(path,"r"))== NULL)	//open file
					err("Can not open file");
				while(!feof(fp)){
					fscanf(fp,"%s %s %s %s %s %s \n",recordID,state,patientFirstName,patientLastName,disease,age);
					pat =  createPatient(recordID,state,patientFirstName,patientLastName,disease,atoi(age));
					deletePatient(pat);
				}
				
				fclose(fp);
			}
			closedir(dir);
		}

	}while(strcmp(buffer,"stop"));
	
	return 0;
}