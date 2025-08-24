

#ifndef HW3_OS_QUEUE_H
#define HW3_OS_QUEUE_H
#include "segel.h"

typedef struct request_t{
    int fd;
    struct timeval ArrivalTime;
    struct timeval dispachTime;
}* Request;
typedef struct node{
    Request r;
    struct node* prev;
    struct node* next;
}* Node;


typedef struct queue_t{
    Node  root;
    Node  last;
    int m_size;
    int members_num;
    int r_num;
   
 

}*Queue;




int init_queue(Queue *temp, int n);
int enqueue(Queue q,Request r_1);
Node dequeue(Queue q);
Node dequeueLast(Queue q);
int dequeue_fd(Queue q,int fd);
int r_enqueue(Queue q,Node n);
int createRequest(int fd,struct timeval  time,Request * req_1);
void set_time(struct timeval * t,Request n);
void get_Request(Node temp, Request *r_1);
int get_fd(Request r);
void incrase_r(Queue q);
void decrase_r(Queue q);

#endif //HW3_OS_QUEUE_H

