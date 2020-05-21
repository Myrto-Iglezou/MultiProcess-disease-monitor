#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <dirent.h>
#include <fcntl.h>
#include "hashtable.h"

typedef struct statistics{
	char date[11];
	char country[32];
	char disease[32];
	int ranges[4];
}statistics;

void findRanges(statistics** stat,BucketRecord* record);

void searchTree(Treenode* root,statistics** stat);

void printStat(statistics * stat);