#include <stdlib.h>

#include "list.h"

node *front = NULL;
node *back = NULL;

void insertBefore(void *data, node *nodeB) {
	node *newNode = malloc(sizeof(node));
	newNode->prev = nodeB->prev;
	newNode->next = nodeB;
	newNode->data = data;
	if(nodeB->prev == NULL) {
		front = newNode;
	}
	nodeB->prev = newNode;
	removeNode(newNode);
}

void insertFront(void *data) {
	node *newNode;
	if(front == NULL) {
		newNode = malloc(sizeof(node));
		front = newNode;
		back = newNode;
		newNode->prev = NULL;
		newNode->next = NULL;
		newNode->data = data;
	} else
		insertBefore(data, front);
}

void insertAfter(void *data, node *nodeB) {
	node *newNode = malloc(sizeof(node));
	newNode->next = nodeB->next;
	newNode->prev = nodeB;
	newNode->data = data;

	if(nodeB->next == NULL)
		back = newNode;
	nodeB->next = newNode;
}
void insertBack(void *data) {
	if(back == NULL)
		insertFront(data);
	else
		insertAfter(data, back);
}

void removeFront() {
	removeNode(front);
}

void removeBack() {
	removeNode(back);
}
void removeBefore(node *nodeB) {
	if(nodeB->prev == front) {
		front = nodeB;
		front->prev = NULL;
	} else
		removeNode(nodeB->prev);
}
void removeAfter(node *nodeA) {
	if(nodeA->next == back) {
		back = nodeA;
		back->next = NULL;
	} else
		removeNode(nodeA->next);
}

void removeNode(node *nodeToRemove) {
	if(nodeToRemove == front) {
		front = front->next;
		front->prev = NULL;
	} else if(nodeToRemove == back) {
		back = back ->prev;
		back->next = NULL;
	} else {
		nodeToRemove->prev->next = nodeToRemove->next;
		nodeToRemove->next->prev = nodeToRemove->prev;
	}
}

