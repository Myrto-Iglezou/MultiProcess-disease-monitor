#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "patient.h"

int CompareDates(const void* FDate,const void* SDate);
int ComparePatientsID(const void* pat1,const void* pat2);
int ComparePatientsDates(const void* pat1,const void* pat2);
int CompareAges(const void* pat1,const void* pat2);