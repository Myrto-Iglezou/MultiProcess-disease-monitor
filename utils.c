#include "utils.h"
#define err(mess){fprintf(stderr,"ERROR: %s\n",mess);exit(1);}

void findRanges(statistics** stat,BucketRecord* record){
	searchTree(record->root,stat);
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