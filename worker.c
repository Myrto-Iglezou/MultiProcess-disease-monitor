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
#include "comparators.h" 
#include "RBT.h"
#define TRUE 1
#define FALSE 0
#define err(mess){fprintf(stderr,"ERROR: %s\n",mess);exit(1);}

int main(int argc, char const *argv[]){

	int fd,bufferSize;
	char fifo[32];
	int num;
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
	char* token;
	char tempstr[40];

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
			n=0;
			count=0;
			while((dir_info = readdir(dir)) != NULL){
				count++;
			}
			closedir(dir);
			char *namelist[count];
			if((dir = opendir(buffer)) == NULL)	//open directory
				err("Can not open directory");
			while((dir_info = readdir(dir)) != NULL){
				strcpy(path,buffer); 
				if(!strcmp(dir_info->d_name,".") || !strcmp(dir_info->d_name,".."))	continue;
				namelist[n] = malloc((strlen(DATE)+1)*sizeof(char)); 
				strcpy(namelist[n],dir_info->d_name);
				n++;
			}
			
			qsort(&namelist[0],n-1,sizeof(char*),CompareDates);	
	
			for(int i=0 ; i<n-1 ; i++){
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
					if(tempNode==guard && !strcmp(state,"ENTER"))
						insertion(&root,pat,ComparePatientsID);
					else if(tempNode!=guard && !strcmp(state,"EXIT")){
						deletePatient(pat);
						pat = tempNode->data;
						strcpy(date1,pat->entryDate);
						if(CompareDates(&date1,&date2)<=0)
							strcpy(pat->exitDate,date2);
					}else	
						deletePatient(pat);
				}
				fclose(fp);
			}
			for (int i = 0; i < n; i++){
				free(namelist[i]);
			}
				
			closedir(dir);
		}

	}while(strcmp(buffer,"stop"));
	// printTree(root,PrintPatient);
	free(guard);
	deleteTree(root,deletePatient);
	free(date1);free(date2);
	return 0;
}