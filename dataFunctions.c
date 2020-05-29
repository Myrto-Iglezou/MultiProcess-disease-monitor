#include "dataFunctions.h"

int  diseaseFrequency(char* country,char* virus,HashTable* diseaseHashtable,HashTable* countryHashtable,char* FirstDate,char* SecondDate){
	BucketRecord* record;
	int count=0;
		
	if(!findData(diseaseHashtable,virus,&record)){		// find the virus
		err("No record for this virus");	// if it doesn't exist print error
		return -1;
	}
	if(!strcmp(country,"-")){			// if no country is given
		countTreePatients(&count,record->root,FirstDate,SecondDate);	// count all the tree of the virus
		return count;
	}else{
		if(!findData(countryHashtable,country,&record)){	// find the country
			err("No record for this country");	// if it doesn't exist print error
			return -1;
		}
		countFrequency(virus,&count,record->root,FirstDate,SecondDate);	
		return count;
	}
}

void countTreePatients(int* count,Treenode * root,char* FirstDate,char* SecondDate){	// count the patients in the tree
	Treenode *temp  = root;		
	Patient* pat = (Patient*) temp->data;

	if(!strcmp(FirstDate,"-")){			// if no date is given
		if(temp==guard)
			return;
		countTreePatients(count,temp->left,FirstDate,SecondDate);
		(*count)++;
		countTreePatients(count,temp->right,FirstDate,SecondDate);		
	}else{

		if(temp==guard)
			return;
		if(CompareDates(&(pat->entryDate),&FirstDate)>=0 && CompareDates(&(pat->entryDate),&SecondDate)<=0) // increase the counter between the two dates
			(*count)++;
		
		countTreePatients(count,temp->left,FirstDate,SecondDate);
		countTreePatients(count,temp->right,FirstDate,SecondDate);
	}
}

void countFrequency(char* virus,int* count,Treenode * root,char* FirstDate,char* SecondDate){ // count patients with this varius
	Treenode *temp  = root;
	Patient* pat = (Patient*) temp->data;

	if(!strcmp(FirstDate,"-")){
		if(temp==guard)
			return;
		if(!strcmp(pat->disease,virus))		// if the virus is the wanted increase
			(*count)++;
		countFrequency(virus,count,temp->left,FirstDate,SecondDate);
		countFrequency(virus,count,temp->right,FirstDate,SecondDate);
	}else{			
		if(temp==guard)
			return;

		if(CompareDates(&(pat->entryDate),&FirstDate)>=0 && CompareDates(&(pat->entryDate),&SecondDate)<=0){ // increase the counter between the two dates
			if(!strcmp(pat->disease,virus))
				(*count)++;
		}
		countFrequency(virus,count,temp->left,FirstDate,SecondDate);
		countFrequency(virus,count,temp->right,FirstDate,SecondDate);
	}
}