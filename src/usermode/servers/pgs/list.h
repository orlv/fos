#ifndef __LIST_H
#define __LIST_H

typedef struct ns {
  void *data;
  struct ns *next;
  struct ns *prev;
} node;

void insertFront(void *data);
void insertBack(void *data);

void removeFront();
void removeBack();

void insertBefore(void *data, node * nodeB);
void insertAfter(void *data, node * nodeA);

void removeBefore(node * nodeB);
void removeAfter(node * nodeA);
void removeNode(node * newNode);

/*void printDListFront();
void printDListBack();
*/

#endif
