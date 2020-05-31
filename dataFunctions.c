#include "dataFunctions.h"

int  diseaseFrequency(char* country,char* virus,HashTable* diseaseHashtable,HashTable* countryHashtable,char* FirstDate,char* SecondDate,int entry,int exit){
	BucketRecord* record;
	int count=0;
		
	if(!findData(diseaseHashtable,virus,&record)){		// find the virus
		return -1;
	}
	if(!strcmp(country,"-")){			// if no country is given
		countTreePatients(&count,record->root,FirstDate,SecondDate,entry,exit);	// count all the tree of the virus
		return count;
	}else{
		if(!findData(countryHashtable,country,&record)){	// find the country
			return -1;
		}
		countFrequency(virus,&count,record->root,FirstDate,SecondDate,entry,exit);	
		return count;
	}
}

void countTreePatients(int* count,Treenode * root,char* FirstDate,char* SecondDate,int entry,int exit){	// count the patients in the tree
	Treenode *temp  = root;		
	Patient* pat = (Patient*) temp->data;

	if(entry){

		if(temp==guard)
			return;
		if(CompareDates(&(pat->entryDate),&FirstDate)>=0 && CompareDates(&(pat->entryDate),&SecondDate)<=0 && !strcmp(pat->exitDate,"-")) // increase the counter between the two dates
			(*count)++;
		
		countTreePatients(count,temp->left,FirstDate,SecondDate,entry,exit);
		countTreePatients(count,temp->right,FirstDate,SecondDate,entry,exit);			
				
	}else if(exit){

		if(temp==guard)
			return;
		if(CompareDates(&(pat->entryDate),&FirstDate)>=0 && CompareDates(&(pat->entryDate),&SecondDate)<=0 && strcmp(pat->exitDate,"-")) // increase the counter between the two dates
			(*count)++;
		
		countTreePatients(count,temp->left,FirstDate,SecondDate,entry,exit);
		countTreePatients(count,temp->right,FirstDate,SecondDate,entry,exit);	

	}else{

		if(temp==guard)
			return;
		if(CompareDates(&(pat->entryDate),&FirstDate)>=0 && CompareDates(&(pat->entryDate),&SecondDate)<=0) // increase the counter between the two dates
			(*count)++;
		
		countTreePatients(count,temp->left,FirstDate,SecondDate,entry,exit);
		countTreePatients(count,temp->right,FirstDate,SecondDate,entry,exit);
	}
}

void countFrequency(char* virus,int* count,Treenode * root,char* FirstDate,char* SecondDate,int entry,int exit){ // count patients with this varius
	Treenode *temp  = root;
	Patient* pat = (Patient*) temp->data;

	if(entry){

		if(temp==guard)
			return;

		if(CompareDates(&(pat->entryDate),&FirstDate)>=0 && CompareDates(&(pat->entryDate),&SecondDate)<=0 && !strcmp(pat->exitDate,"-")){ // increase the counter between the two dates
			if(!strcmp(pat->disease,virus))
				(*count)++;
		}
		countFrequency(virus,count,temp->left,FirstDate,SecondDate,entry,exit);
		countFrequency(virus,count,temp->right,FirstDate,SecondDate,entry,exit);

	}else if(exit){

		if(temp==guard)
			return;

		if(CompareDates(&(pat->entryDate),&FirstDate)>=0 && CompareDates(&(pat->entryDate),&SecondDate)<=0 && strcmp(pat->exitDate,"-")){ // increase the counter between the two dates
			if(!strcmp(pat->disease,virus))
			(*count)++;
		}
		
		countFrequency(virus,count,temp->left,FirstDate,SecondDate,entry,exit);
		countFrequency(virus,count,temp->right,FirstDate,SecondDate,entry,exit);

	}else{			
		if(temp==guard)
			return;

		if(CompareDates(&(pat->entryDate),&FirstDate)>=0 && CompareDates(&(pat->entryDate),&SecondDate)<=0){ // increase the counter between the two dates
			if(!strcmp(pat->disease,virus))
				(*count)++;
		}
		countFrequency(virus,count,temp->left,FirstDate,SecondDate,entry,exit);
		countFrequency(virus,count,temp->right,FirstDate,SecondDate,entry,exit);
	}
}

int  findRange(char* country,char* disease,HashTable* diseaseHashtable,HashTable* countryHashtable,char* FirstDate,char* SecondDate,int range){

	BucketRecord* record;
	int count=0;
		
	if(!findData(diseaseHashtable,disease,&record)){		// find the virus
		return -1;
	}

	if(!findData(countryHashtable,country,&record)){	// find the country
		return -1;
	}
	countRanges(disease,&count,record->root,FirstDate,SecondDate,range);	
	return count;
}

void countRanges(char* disease,int* count,Treenode * root,char* FirstDate,char* SecondDate,int range){
	Treenode *temp  = root;
	Patient* pat = (Patient*) temp->data;

	if(temp==guard)
		return;

	if(range == 0){
		if(pat->age>=0 && pat->age<= 20){
			if(CompareDates(&(pat->entryDate),&FirstDate)>=0 && CompareDates(&(pat->entryDate),&SecondDate)<=0){ // increase the counter between the two dates
				if(!strcmp(pat->disease,disease))
					(*count)++;
			}
		}
	}else if(range == 1){
		if(pat->age>=21 && pat->age<= 40){
			if(CompareDates(&(pat->entryDate),&FirstDate)>=0 && CompareDates(&(pat->entryDate),&SecondDate)<=0){ // increase the counter between the two dates
				if(!strcmp(pat->disease,disease))
					(*count)++;
			}
		}
	}else if(range == 2){
		if(pat->age>=41 && pat->age<= 60){
			if(CompareDates(&(pat->entryDate),&FirstDate)>=0 && CompareDates(&(pat->entryDate),&SecondDate)<=0){ // increase the counter between the two dates
				if(!strcmp(pat->disease,disease))
					(*count)++;
			}
		}
	}else if(range == 3){
		if(pat->age>=60){
			if(CompareDates(&(pat->entryDate),&FirstDate)>=0 && CompareDates(&(pat->entryDate),&SecondDate)<=0){ // increase the counter between the two dates
				if(!strcmp(pat->disease,disease))
					(*count)++;
			}
		}
	}

	countRanges(disease,count,temp->left,FirstDate,SecondDate,range);
	countRanges(disease,count,temp->right,FirstDate,SecondDate,range);
}