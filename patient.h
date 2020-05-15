#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define DATE "DD-MM-YYYY"

typedef struct Patient{	
	char* recordID;
	char* state;
	char* firstName;
	char* lastName;
	char* disease;
	int age;
}Patient;

Patient* createPatient(char* recordID,char* state,char* firstName,char* lastName,char* disease,int age);
void deletePatient(Patient* pat);


