#include "doublelinked.h"

struct DList* newList(){
	struct DList *list = (struct DList*) malloc(sizeof(struct DList));
	list->head=NULL;
	list->tail=NULL;
	return list;
}

struct DListNode* newNode(){ //Creation of a new node
	// Allocate memory for a message object
	struct dayObject *day = (struct dayObject*) calloc(1,100);//malloc(sizeof(struct dayObject));
	// Allocate memory for a list node
	struct DListNode *node = (struct DListNode*) malloc(sizeof(struct DListNode));
	
	if (day == NULL) { // If memory cant be allocated
		return NULL;
	} else if(node == NULL){
		free(day);
		return NULL;
	} else { // Else assign data and return pointer
		node->prev = NULL;
		node->next = NULL;
		return node;
	}
}

void append(struct DList *list, struct DListNode *nInsert){
	//Insert a node in the list
	if(list->head!=NULL){ // If there's a head: put in list
		nInsert->prev = list->tail;
		nInsert->next = list->head;
		list->tail->next = nInsert;
		list->tail = nInsert;
	}else{ // If no head: be head
		list->head=nInsert;
		list->tail=nInsert;
	}
}

struct dayObject pop(struct DList *list){ //Delete the first node in the list.
	struct DListNode *tempLink = list->head;
	struct dayObject tempData = *(tempLink->day);
	list->head = list->head->next;
	free(tempLink);
	return tempData;
}