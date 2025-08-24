

#include "Queue.h"

void get_Request(Node temp, Request * r_1){
    if (temp== NULL){
       return;
    }


    *r_1=temp->r;
}
int get_fd(Request r){
    if (r!=NULL){
        return r->fd;

    }
    return -1;
}

int create_node(Request r_1,  Node *n_1){

    *n_1=(Node)malloc(sizeof(**n_1));
     Node n=*n_1;
    
    if(!n){
        return -1;
    }
    n->r=r_1;
    n->next= NULL;
    n->prev=NULL;
    return 1;


}

int init_queue(Queue *temp,int n){
     *temp= (Queue)malloc(sizeof(**temp));
Queue q=*temp;
     
    if(q==NULL){
        return -1;
     }

    q->last=NULL;
    q->root=NULL;
    q->m_size=n;
    q->members_num=0;
    q->r_num=0;
   


    return 1;
}

int enqueue(Queue q,Request r_1){
    if (q==NULL){
        return -1;
    }
    Node n=NULL;
    int t=create_node(r_1,&n);
    if (t==-1){
        return -1;
    }
   if (q->last== NULL){
       q->root=n;
       q->last=n;
       q->members_num++;
        
       return 1;
   }
   n->prev=q->last;
   q->last->next=n;
   q->last=n;

    q->members_num++;

    return 1;

}



int createRequest(int fd,struct timeval  time,Request *req_1){
    *req_1=(Request) malloc(sizeof(**req_1));
     Request req=*req_1;
    if(req==NULL){
        return -1;
    }
    req->fd=fd;
    req->ArrivalTime=time;
    // not picked up by a thread yet so no dispatch time
    req->dispachTime=time;
    return 1;

}

Node dequeue(Queue q)
{
    if (q==NULL || q->members_num==0){
        return NULL;
    }

    Node temp=q->root->next;
    Node removed_node=(q->root);
    if (temp != NULL){
        temp->prev=NULL;
        q->members_num--;
        q->root=temp;
        return removed_node;
    }
    q->root=NULL;
    q->last=NULL;
    q->members_num--;
    return removed_node;

}
int dequeue_fd(Queue q,int fd){
    if (q==NULL){
        return -1;
    }
    Node temp=q->root;
    while(temp != NULL){
        if (temp->r->fd==fd){
           break;
          }
          
        temp=temp->next;
    }
    if(temp==NULL){return -1;}
      if(temp->prev)
        temp->prev->next = temp->next;
    if(temp->next)
        temp->next->prev = temp->prev;
    if(q->root == temp)
        q->root = temp->next;
    if(q->last == temp)
        q->last = temp->prev;

      q->members_num--;
return 1;

}

Node dequeueLast(Queue q) {
    if (q == NULL || q->members_num == 0) {
        return NULL;
    }

    Node removed_node = q->last;
    if (removed_node->prev != NULL) {
        q->last = removed_node->prev;
        q->last->next = NULL;
    } else {
        q->root = NULL;
        q->last = NULL;
    }
    q->members_num--;

    return removed_node;
}
