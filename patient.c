#include "patient.h"

Patient* createPatient(char* recordID,char* state,char* firstName,char* lastName,char* disease,int age){
	Patient* pat = malloc(sizeof(Patient));
	pat->recordID = malloc((strlen(recordID)+1)*sizeof(char));
	pat->firstName = malloc((strlen(firstName)+1)*sizeof(char));
	pat->lastName = malloc((strlen(lastName)+1)*sizeof(char));
	pat->disease = malloc((strlen(disease)+1)*sizeof(char));
	pat->age = age;
	strcpy(pat->recordID,recordID);
	strcpy(pat->firstName,firstName);
	strcpy(pat->lastName,lastName);
	strcpy(pat->disease,disease);
	return pat;
}

void deletePatient(Patient* pat){
	free(pat->recordID);
	free(pat->firstName);
	free(pat->lastName);
	free(pat->disease);
	free(pat);
}