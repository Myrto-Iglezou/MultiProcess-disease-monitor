#include "utils.h"

void findRanges(statistics** stat,BucketRecord* record){
	searchTree(record->root,stat);
}


void searchTree(Treenode* root,statistics** stat){
	if(root==guard)
		return;
	searchTree(root->left,stat);
	searchTree(root->right,stat);
	Patient* pat = (Patient*) root->data;
	int age = pat->age;
	if(age>=0 && age<= 20)
		(*stat)->ranges[0]++;
	else if(age>=21 && age<= 40)
		(*stat)->ranges[1]++;
	else if(age>=41 && age<= 60)
		(*stat)->ranges[2]++;
	else if(age>=60)
		(*stat)->ranges[3]++;
}

void printStat(statistics * stat){
	printf("\n%s\n",stat->date);
	printf("%s\n",stat->country);
	printf("%s\n",stat->disease );
	for (int i = 0; i < 4; i++){
		switch(i){
			case 0:
				printf("Age range 0-20 years: ");
				break;
			case 1:
				printf("Age range 21-40 years: ");	
				break;
			case 2:
				printf("Age range 41-60 years: ");
				break;
			case 3: 
				printf("Age range 60+ years: ");
		}
		printf("%d cases\n",stat->ranges[i]);
	}

}