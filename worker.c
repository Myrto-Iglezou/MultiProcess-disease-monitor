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

	int fd,bufferSize;
	char fifo[32];


	for(int i=0; i<argc;i++){
		
		// if(!strcmp(argv[i],"-fd"))
		// 	fd = atoi(argv[i+1]);
		if(!strcmp(argv[i],"-fn"))
			strcpy(fifo,argv[i+1]);
		if(!strcmp(argv[i],"-b"))
			bufferSize = atoi(argv[i+1]);
	}

	if((fd=open(fifo, O_RDONLY)) < 0)
		err("Could not open fifo");

	char buffer[bufferSize];

	if(read(fd,buffer,bufferSize+1)<0)
		err("Problem in reading");
	close(fd);
	
	printf("%s\n",buffer );
	return 0;
}