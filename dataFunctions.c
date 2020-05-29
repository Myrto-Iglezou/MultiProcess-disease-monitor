#include "dataFunctions.h"

void diseaseFrequency(char* country,char* virus,HashTable* diseaseHashtable,HashTable* countryHashtable,char FirstDate[strlen(DATE)+1],char SecondDate[strlen(DATE)+1]){
	BucketRecord* record;
	int count=0;
		
	if(!findData(diseaseHashtable,virus,&record)){		// find the virus
		printf("error\n");
		//printf("\n\033[1;31mERROR: \033[0m:No record for %s! \n",virus);	// if it doesn't exist print error
		return;
	}
	if(!strcmp(country,"-")){			// if no country is given
		//printf("\n\033[1;32m%s\033[0m: ",record->data);	
		printf("%s ",record->data);
		countTreePatients(&count,record->root,FirstDate,SecondDate);	// count all the tree of the virus
		printf("%d\n",count);
		return;
	}else{
		if(!findData(countryHashtable,country,&record)){	// find the country
			printf("error\n");
			//printf("\n\033[1;31mERROR: \033[0m:No record for %s! \n",country);	// if it doesn't exist print error
			return;
		}
		
		//printf("\n\033[1;32m%s\033[0m: ",record->data);		// count the patients in this country, of the tree of this virus
		printf("%s ",virus);
		countFrequency(virus,&count,record->root,FirstDate,SecondDate);	
		printf("%d\n",count);
		return;
	}
}

void countTreePatients(int* count,Treenode * root,char FirstDate[strlen(DATE)+1],char SecondDate[strlen(DATE)+1]){
	Treenode *temp  = root;		

	if(!strcmp(FirstDate,"-")){			// if no date is given
		if(temp==guard)
			return;
		countTreePatients(count,temp->left,FirstDate,SecondDate);
		(*count)++;
		countTreePatients(count,temp->right,FirstDate,SecondDate);		
	}else{

		if(temp==guard)
			return;
		if(CompareDates(temp->date,FirstDate)>=0 && CompareDates(temp->date,SecondDate)<=0) // increase the counter between the two dates
			(*count)++;
		
		countTreePatients(count,temp->left,FirstDate,SecondDate);
		countTreePatients(count,temp->right,FirstDate,SecondDate);
	}
}

void countFrequency(char* virus,int* count,Treenode * root,char FirstDate[strlen(DATE)+1],char SecondDate[strlen(DATE)+1]){

	Treenode *temp  = root;
	if(!strcmp(FirstDate,"-")){
		if(temp==guard)
			return;
		if(!strcmp(temp->patient->patient->diseaseID,virus))		// if the virus is the wanted increase
			(*count)++;
		countFrequency(virus,count,temp->left,FirstDate,SecondDate);
		countFrequency(virus,count,temp->right,FirstDate,SecondDate);
	}else{			
		if(temp==guard)
			return;
		if(CompareDates(temp->date,FirstDate)>=0 && CompareDates(temp->date,SecondDate)<=0){
			if(!strcmp(temp->patient->patient->diseaseID,virus))
				(*count)++;
		}
		countFrequency(virus,count,temp->left,FirstDate,SecondDate);
		countFrequency(virus,count,temp->right,FirstDate,SecondDate);
	}
}