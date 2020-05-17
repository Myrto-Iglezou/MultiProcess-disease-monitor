#include "comparators.h" 

int CompareDates(const void* FDate,const void* SDate){
	const char* FirstDate  = *(char**) FDate;
	const char* SecondDate = *(char**) SDate;
	
	char* temp = malloc((strlen(DATE)+1)*sizeof(char)); ;
	char day1[3],day2[3],month1[3],month2[3],year1[5],year2[5];
	const char s[2] = "-";
   	char *token;
    strcpy(temp,FirstDate);
    token = strtok(temp, s);

	for(int i=0;i<3;i++){		// split the first date
	    	
    	if(i==0)
    		strcpy(day1,token);
    	if(i==1)
    		strcpy(month1,token);
    	if(i==2)
    		strcpy(year1,token);
   	
      	token = strtok(NULL, s);
    }

   strcpy(temp,SecondDate);
   token = strtok(temp, s);
   
   for(int i=0;i<3;i++){		// split the second date
    	
    	if(i==0)
    		strcpy(day2,token);
    	if(i==1)
    		strcpy(month2,token);
    	if(i==2)
    		strcpy(year2,token);
   	
      	token = strtok(NULL, s);
   }

   free(temp);
   
   	if(atoi(year2)<atoi(year1))
		return 1;
	else if(atoi(year2)>atoi(year1))
		return -1;
	else if(atoi(year2)==atoi(year1) && atoi(month2)<atoi(month1))
		return 1;
	else if(atoi(year2)==atoi(year1) && atoi(month2)>atoi(month1))
		return -1;
	else if(atoi(year2)==atoi(year1) && atoi(month2)==atoi(month1) && atoi(day2)<atoi(day1))
		return 1;
	else if(atoi(year2)==atoi(year1) && atoi(month2)==atoi(month1) && atoi(day2)>atoi(day1))
		return -1;
	else if(atoi(year2)==atoi(year1) && atoi(month2)==atoi(month1) && atoi(day2)==atoi(day1))
		return 0;
}

int ComparePatientsID(const void* pat1,const void* pat2){
	int id1 = atoi(((Patient*) pat1)->recordID);
	int id2 = atoi(((Patient*) pat2)->recordID);
	if(id1>id2)
		return 1;
	else if(id1<id2)
		return -1;
	else return 0;
}