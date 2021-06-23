//21800691 Operating Systems HW5
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

// //Node structure
typedef struct Node Node;
struct Node
{
  char *value;
  Node *next;
};

//Queue structure
typedef struct Queue Queue;
struct Queue
{
    Node *headNode, *tailNode;
    pthread_mutex_t headLock, tailLock;
    int queCount;

} que;

//executed right after que creation
void initializeQue(Queue *que)
{
    Node *tempNode = malloc(sizeof(Node));
    tempNode->next = NULL;
    
    que->headNode = tempNode;
    que->tailNode = tempNode;

    pthread_mutex_init(&que->headLock, NULL);
    pthread_mutex_init(&que->tailLock, NULL);
    que->queCount = 0;
}

void enque(Queue *que, char *value)
{
    Node *tempNode = malloc(sizeof(Node));
    tempNode->value = value;
    tempNode->next = NULL;

    pthread_mutex_lock(&que->tailLock);//protect critical section
    //add a new tail
    que->tailNode->next = tempNode;
    que->tailNode = tempNode;
    que->queCount++; //increment current queCount

    pthread_mutex_unlock(&que->tailLock); //end of critical section
}

char * deque(Queue *que)
{
    pthread_mutex_lock(&que->headLock);//protect critical section
    
    //swap next node with current node
    Node *tempNode = que->headNode;
    Node *newHeadNode = tempNode->next;
    char *value = newHeadNode->value;
    que->headNode = newHeadNode;
    que->queCount--; //decrement current queCount
    
    pthread_mutex_unlock(&que->headLock); //end of critical section
    free(tempNode);
    return value;
}

int empty(Queue *queue)
{
    //empty if count is zero
    if (queue->queCount == 0) return 1;
}