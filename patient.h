#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define DATE "DD-MM-YYYY"

typedef struct Patient{	
	char* recordID;
	char* firstName;
	char* lastName;
	char* disease;
	char* country;
	char* entryDate;
	char* exitDate;
	int age;
}Patient;

Patient* createPatient(char* recordID,char* firstName,char* lastName,char* disease,char* country,char* entryDate,char* exitDate,int age);
void deletePatient(Patient* pat);


