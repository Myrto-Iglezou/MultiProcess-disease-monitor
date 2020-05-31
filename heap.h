#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>



typedef struct heapData{
	int numOfCases;
	char* data;
}heapData;

typedef struct heapNode{
	heapData* info;
	struct heapNode* parent;
	struct heapNode* right;
	struct heapNode* left; 
}heapNode;

typedef struct heap{
	unsigned int numOfnodes;
	heapNode* root;
}heap;

void swap(heapNode* A, heapNode* B);
void createHeap(heap** newheap);
void insertToHeap(int num, char* data,heap* newheap);
void fixInsertion(heap* newheap,heapNode* newNode);
void getTheMax(char data[32], int* max,heap* newheap);
void fixDeletion(heap* newheap,heapNode* tempNode);
void DeleteHeap(heap* newheap,heapNode* tempNode);