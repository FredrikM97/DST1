#ifndef doublelinked_h
#define doublelinked_h

#include "stdio.h"
#include "stdlib.h"

struct DListNode {
	struct dayObject *day;
	struct DListNode *prev;
	struct DListNode *next;
};

struct dayObject {
	double avg;
	float min;
	float max;
	float data[1440]; // 1440 minutes of data per day
};

struct DList {
	struct DListNode *head;
	struct DListNode *tail;
};

struct DList* newList();
struct DListNode* newNode();
void printList(struct DList *list);
void append(struct DList *list, struct DListNode *nInsert);
struct dayObject pop(struct DList *list);

#endif
