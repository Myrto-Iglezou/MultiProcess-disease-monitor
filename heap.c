#include "heap.h"

void swap(heapNode* A, heapNode* B){	// exchange the data between two nodes
	heapData* temp;
	temp = A->info;
	A->info = B->info;
	B->info = temp;
}

void createHeap(heap** newheap){
	*newheap = malloc(sizeof(heap));
	(*newheap)->root = NULL;
	(*newheap)->numOfnodes = 0;
}

void insertToHeap(int num, char* data,heap* newheap){
	heapNode* newNode = malloc(sizeof(heapNode));		// create the new node
	newNode->right = NULL;
	newNode->left = NULL;
	newNode->parent = NULL;
	newNode->info = malloc(sizeof(heapData));
	newNode->info->data = malloc((strlen(data)+1)*sizeof(char));
	strcpy(newNode->info->data,data);
	newNode->info->numOfCases = num;

	if(newheap->root == NULL){		// if the heap is empty 
		newheap->root = newNode;	// make the new node the root
		newheap->numOfnodes = 1;
		return;
	}else{
		(newheap->numOfnodes)++;
		int height = (int) log2(newheap->numOfnodes);	// find thh height
		int wantedNumber = newheap->numOfnodes;
		int number = wantedNumber<<(32-height);			// keep the number of bits(height number) we are interested in the left
		int temp;
		heapNode* tempNode = newheap->root;

		for(int i=0; i<(height-1); i++){	// height-1 because we want the parent of the new node
			temp = number;
			temp = temp>>31;		// take one bit at a time
			number = number<<1;
			if(temp)				// if bit is 1 go right
				tempNode = tempNode->right;
			else					// if bit is 0 go left
				tempNode = tempNode->left;
		}
		newNode->parent = tempNode;
		if(tempNode->left == NULL)		// if left child is NULL make the new node the left child
			tempNode->left = newNode;
		else
			tempNode->right = newNode;

		fixInsertion(newheap,newNode);
	}
}

void fixInsertion(heap* newheap,heapNode* newNode){
	if(newNode->parent==NULL)
		return;
	
	if(newNode->info->numOfCases > newNode->parent->info->numOfCases)	// if the newnode if greater than  the parent swap
		swap(newNode,newNode->parent);
	fixInsertion(newheap,newNode->parent);

}
void getTheMax(char data[32], int* max,heap* newheap){
	*max = newheap->root->info->numOfCases;
	strcpy(data,newheap->root->info->data);
	if(newheap->root->right == NULL && newheap->root->left == NULL)
		return;
	
	int height = (int) log2(newheap->numOfnodes);	// find the parent of the last node
	int wantedNumber = newheap->numOfnodes;
	int number = wantedNumber<<(32-height);
	int temp;
	heapNode* tempNode = newheap->root;

	for(int i=0; i<height-1; i++){
		temp = number;
		temp = temp>>31;
		number = number<<1;
		if(temp)
			tempNode = tempNode->right;
		else
			tempNode = tempNode->left;
	}

	(newheap->numOfnodes)--;

	if(tempNode->right != NULL){		// if the right child is not null swap it with the root
		swap(newheap->root,tempNode->right);
		free(tempNode->right->info->data);
		free(tempNode->right->info);
		free(tempNode->right);
		tempNode->right = NULL;
	}else{								// if the right child is null swap the left child with the root
		swap(newheap->root,tempNode->left);
		free(tempNode->left->info->data);
		free(tempNode->left->info);
		free(tempNode->left);
		tempNode->left = NULL;
	}
	fixDeletion(newheap,newheap->root);
}
void fixDeletion(heap* newheap,heapNode* tempNode){

	heapNode* maxChild = NULL;
	heapNode* leftChild = tempNode->left;
	heapNode* rightChild = tempNode->right;

	if(rightChild == NULL && leftChild == NULL)
		return;

	/*---------- find the max value of the children ---------*/
	if(rightChild==NULL)
		maxChild = leftChild;
	else if(leftChild==NULL)
		maxChild = rightChild;
	else if(leftChild->info->numOfCases == tempNode->info->numOfCases && rightChild->info->numOfCases == tempNode->info->numOfCases){ // parent and both children have the same value	
		fixDeletion(newheap,leftChild);		// go one leve down
		fixDeletion(newheap,rightChild);
	}
	if(rightChild==NULL)
		maxChild = leftChild;
	else if(leftChild==NULL)
		maxChild = rightChild;
	else if(rightChild->info->numOfCases>leftChild->info->numOfCases)
		maxChild = rightChild;
	else if(leftChild->info->numOfCases>=rightChild->info->numOfCases)
		maxChild = leftChild;

	if(tempNode->info->numOfCases <= maxChild->info->numOfCases){		// if the parent's value is small than the max of the children
		swap(tempNode,maxChild);										// swap that child with the parent
		fixDeletion(newheap,maxChild);									// continue with that child as parent
	}else return;
	
}

void DeleteHeap(heap* newheap,heapNode* tempNode){
	
	if(tempNode->right == NULL && tempNode->left==NULL){
		free(tempNode->info->data);
		free(tempNode->info);
		free(tempNode);
		return;
	}
	if(tempNode->left != NULL)
		DeleteHeap(newheap,tempNode->left);
	if(tempNode->right != NULL)
		DeleteHeap(newheap,tempNode->right);

	free(tempNode->info->data);
	free(tempNode->info);
	free(tempNode);
}