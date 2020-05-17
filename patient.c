#include "patient.h"

Patient* createPatient(char* recordID,char* firstName,char* lastName,char* disease,char* country,char* entryDate,char* exitDate,int age){
	Patient* pat = malloc(sizeof(Patient));
	pat->recordID = malloc((strlen(recordID)+1)*sizeof(char));
	pat->firstName = malloc((strlen(firstName)+1)*sizeof(char));
	pat->lastName = malloc((strlen(lastName)+1)*sizeof(char));
	pat->disease = malloc((strlen(disease)+1)*sizeof(char));
	pat->country = malloc((strlen(country)+1)*sizeof(char));
	pat->entryDate = malloc((strlen(DATE)+1)*sizeof(char));
	pat->exitDate = malloc((strlen(DATE)+1)*sizeof(char));
	pat->age = age;
	strcpy(pat->recordID,recordID);
	strcpy(pat->firstName,firstName);
	strcpy(pat->lastName,lastName);
	strcpy(pat->disease,disease);
	strcpy(pat->country,country);
	strcpy(pat->entryDate,entryDate);
	strcpy(pat->exitDate,exitDate);
	return pat;
}

void deletePatient(const void* patient){
	Patient* pat = (Patient*) patient;
	free(pat->recordID);
	free(pat->firstName);
	free(pat->lastName);
	free(pat->disease);
	free(pat->country);
	free(pat->entryDate);
	free(pat->exitDate);
	free(pat);
}

void PrintPatient(const void* patient){
	Patient* pat = (Patient*) patient;
	printf("%s %s %s %s %d %s %s\n",pat->recordID,pat->firstName,pat->lastName,pat->disease,pat->age,pat->entryDate,pat->exitDate);
}