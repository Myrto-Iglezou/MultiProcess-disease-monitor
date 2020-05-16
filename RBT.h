#include <stdio.h>
#include <stdlib.h>

typedef struct Treenode{
    void* data;
    char color;
    struct Treenode* left;
    struct Treenode* right;
    struct Treenode* parent;
    
}Treenode;

Treenode * guard;

void LeftRotate(Treenode **root,Treenode* x);

void RightRotate(Treenode **root,Treenode* y);

void insertion(Treenode **root,void* data,int (*comparator)(const void*,const void*));

void Fixedinsertion(Treenode **root,Treenode* z);

void deleteTree(Treenode * root,void (*deletion)(const void*));

Treenode* FindData(Treenode *root,void* data,int (*comparator)(const void*,const void*));

void DeleteData(Treenode **root,void* data,int (*comparator)(const void*,const void*));

void FixedDelete(Treenode **root,Treenode* x);

void transplant(Treenode **root,Treenode * x,Treenode* y);

void printTree(Treenode * root,void (*printData)(const void*));

Treenode* createGuard();


