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
#define TRUE 1
#define FALSE 0

#define err(mess){fprintf(stderr,"ERROR: %s\n",mess);exit(1);}




int CompareDates(const void* FDate,const void* SDate){
	const char* FirstDate  = *(char**) FDate;
	const char* SecondDate = *(char**) SDate;
	
	char* temp = malloc((strlen(DATE)+1)*sizeof(char)); ;
	char day1[3],day2[3],month1[3],month2[3],year1[5],year2[5];
	const char s[2] = "-";
   	char *token;

    strcpy(temp,FirstDate);
    token = strtok(temp, s);

	for(int i=0;i<3;i++){		// split the first date
	    	
    	if(i==0)
    		strcpy(day1,token);
    	if(i==1)
    		strcpy(month1,token);
    	if(i==2)
    		strcpy(year1,token);
   	
      	token = strtok(NULL, s);
    }

   strcpy(temp,SecondDate);
   token = strtok(temp, s);
   
   for(int i=0;i<3;i++){		// split the second date
    	
    	if(i==0)
    		strcpy(day2,token);
    	if(i==1)
    		strcpy(month2,token);
    	if(i==2)
    		strcpy(year2,token);
   	
      	token = strtok(NULL, s);
   }

   free(temp);
   
   	if(atoi(year2)<atoi(year1))
		return 1;
	else if(atoi(year2)>atoi(year1))
		return -1;
	else if(atoi(year2)==atoi(year1) && atoi(month2)<atoi(month1))
		return 1;
	else if(atoi(year2)==atoi(year1) && atoi(month2)>atoi(month1))
		return -1;
	else if(atoi(year2)==atoi(year1) && atoi(month2)==atoi(month1) && atoi(day2)<atoi(day1))
		return 1;
	else if(atoi(year2)==atoi(year1) && atoi(month2)==atoi(month1) && atoi(day2)>atoi(day1))
		return -1;
	else if(atoi(year2)==atoi(year1) && atoi(month2)==atoi(month1) && atoi(day2)==atoi(day1))
		return 0;
}



int main(int argc, char const *argv[]){

	int fd,bufferSize;
	char fifo[32];
	int num;
	DIR * dir;
	FILE * fp;
	struct dirent * dir_info;
	int n=0,count=0;
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
				strcat(path,namelist[i]);
				printf("%s\n",path );
				if((fp = fopen(path,"r"))== NULL)	//open file
					err("Can not open file");

				while(!feof(fp)){
					fscanf(fp,"%s %s %s %s %s %s \n",recordID,state,patientFirstName,patientLastName,disease,age);
					pat =  createPatient(recordID,state,patientFirstName,patientLastName,disease,atoi(age));
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

	
	return 0;
}