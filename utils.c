#include "utils.h"
#define err(mess){fprintf(stderr,"ERROR: %s\n",mess);exit(1);}

void findRanges(statistics** stat,BucketRecord* record){
	searchTree(record->root,stat);
}

int findNum(int id,workerInfo ** array,int numWorkers){
	for(int i=0; i<numWorkers ;i++){
		if(array[i]->pid == id)
			return i;
	}
}

int findWorkerFromCountry(char* country, workerInfo ** array,int numWorkers,int *counter){
	for(int i=0; i<numWorkers ;i++){
		for (int j = 0; j < counter[i]; j++){
			if(!strcmp(country,array[i]->countries[j]))
				return i;
		}
	}
}

int findWorkerFromfd(int fd, workerInfo ** array,int numWorkers){
	for(int i=0; i<numWorkers ;i++){
		if(fd == array[i]->readFd)
			return i;
	}
}

void readBytes(int rfd,char* buffer,int bufferSize,int message_size){
	int num;
	char readBuffer[256];

	int countBytes=0;
	while(countBytes<message_size){			
		if((num = read(rfd,readBuffer,bufferSize))<0)
			err("Problem in reading!");
		strncat(buffer,readBuffer,num);
		countBytes = strlen(buffer);
	}
}

void writeBytes(char * data,int wfd, int bufferSize){
	char *strPointer = &data[0];
	int size = bufferSize;
	int count = 0;
	int message_size = strlen(data);
	char tempstr[256];

	if(write(wfd,&message_size,sizeof(int))<0)
		err("Problem in writing");

	while(count < (strlen(data))){

		strPointer = &data[0];
		strPointer+=count;
		
		if(((strlen(data)+1)-count)<size){
			size = (strlen(data)+1)-count;					
		}
		strncpy(tempstr,strPointer,size);
		if(write(wfd,tempstr,size)<0)
			err("Problem in writing");
		count+=size;
	}
}

void sendStat(char* data,int bufferSize,int wfd){
	int count=0;
	char tempstr[256];
	char* strPointer = data;
	int size = bufferSize;

	int message_size = strlen(data);

	if(write(wfd,&message_size,sizeof(int))<0)
		printf("Problem in writing");

	while(count < message_size){
		
		if(((strlen(data)+1)-count)<size){
			size = (strlen(data)+1)-count;					
		}
		strncpy(tempstr,strPointer,size);
		// printf("%s\n",tempstr );
		if(write(wfd,tempstr,size)<0)
			printf("err\n");
			// err("Problem in writing");
		count+=size;
		strPointer+=size;
	}
}

void savestat(int readFd,int bufferSize,char* data,int sizeOfdata){
	int count=0;
	int message_size;
	int num;
	char readBuffer[256];

	char* buffer = malloc(sizeOfdata);
	strcpy(buffer,"");

	if(read(readFd,&message_size,sizeof(int))<0)
		err("Problem in writing");

	while(count < message_size){

		if((num = read(readFd,readBuffer,bufferSize))<0)
			err("Problem in reading!");
		strncat(buffer,readBuffer,num);
		count += bufferSize;
	}
	strcpy(data,buffer);

	free(buffer);
}

void searchTree(Treenode* root,statistics** stat){
	if(root==guard)
		return;
	searchTree(root->left,stat);
	searchTree(root->right,stat);
	Patient* pat = (Patient*) root->data;
	int age = pat->age;
	if(age>=0 && age<= 20)
		(*stat)->ranges[0]++;
	else if(age>=21 && age<= 40)
		(*stat)->ranges[1]++;
	else if(age>=41 && age<= 60)
		(*stat)->ranges[2]++;
	else if(age>=60)
		(*stat)->ranges[3]++;
}

void printStat(statistics * stat){
	printf("\n%s\n",stat->date);
	printf("%s\n",stat->country);
	printf("%s\n",stat->disease );
	for (int i = 0; i < 4; i++){
		switch(i){
			case 0:
				printf("Age range 0-20 years: ");
				break;
			case 1:
				printf("Age range 21-40 years: ");	
				break;
			case 2:
				printf("Age range 41-60 years: ");
				break;
			case 3: 
				printf("Age range 60+ years: ");
		}
		printf("%d cases\n",stat->ranges[i]);
	}
}

