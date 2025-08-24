#include "segel.h"
#include "request.h"
#include "Queue.h"
#include <unistd.h>
//
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

// HW3: Parse the new arguments too
pthread_cond_t empty_queue;
pthread_cond_t full_queue;
pthread_mutex_t waiting_q_lock;

void* execute(void* args);
typedef struct args{
    Queue run_queue;
    Queue  wait_queue;
    Thread thread;
}* Args;
int createThread(int id, Thread *t,Queue r_q,Queue w_q){
    if(r_q==NULL || w_q==NULL){
        return -1;
    }
    *t=(Thread) malloc(sizeof(**t));
    Thread thread=*t;

    if(thread==NULL){
        return  -1;
    }
    thread->m_id=id;
    thread->m_count=0;
    thread->m_dynamic=0;
    thread->m_static=0;
    Args x=(Args) malloc(sizeof(*x));
    x->thread=thread;
    x->wait_queue=w_q;
    x->run_queue=r_q;
    if(pthread_create(&thread->m_thread,NULL,execute,x)!=0){
        return -1;
    }
    return 1;

}
int addThread(Thread thread,int x){
    if(thread==NULL){
        return -1;
    }
    if(x==1){
        thread->m_static++;
        thread->m_count++;
        return 1;
    }
    if(x==2){
        thread->m_dynamic++;
        thread->m_count++;
        return 1;
    }
    thread->m_count++;
    return 1;

}


void* execute(void* args) {
    if (args == NULL) {
        return NULL;
    }
    Args x = args;
    Thread thread = x->thread;
    Queue w_queue = x->wait_queue;
    Queue r_queue = x->run_queue;
    if (thread == NULL || w_queue == NULL || r_queue == NULL) {
        return NULL;
    }
    Node temp = NULL;
    while (1) {
        pthread_mutex_lock(&waiting_q_lock);

        while (w_queue->members_num == 0) {
            pthread_cond_wait(&empty_queue, &waiting_q_lock);
        }
        temp=dequeue(w_queue);
        
      
         enqueue(r_queue, temp->r);
           

        struct timeval t;
        gettimeofday(&t, NULL);

        Request r=NULL;
        get_Request(temp,&r);
         
          

        timersub(&t,&r->ArrivalTime,&r->dispachTime);
        int fd_r = get_fd(r);
        pthread_mutex_unlock(&waiting_q_lock);


        requestHandle(r, thread, &w_queue, &waiting_q_lock);
// not sure if needed, assumed accept in server.c does open
         close(fd_r);
       

        Node t_delete = NULL;

        pthread_mutex_lock(&waiting_q_lock);

        dequeue_fd(r_queue,fd_r);
        
        pthread_cond_signal(&full_queue);

        pthread_mutex_unlock(&waiting_q_lock);
        



    }
}
void getargs(int *port, int argc, char *argv[],int* threads_num,int* queue_size,char* sched)
{
    if (argc <5) {
	fprintf(stderr, "Usage: %s <port>\n", argv[0]);
	exit(1);
    }
    *port = atoi(argv[1]);
    *threads_num= atoi(argv[2]);
    *queue_size=atoi(argv[3]);
    for(int i=0;i<7;i++){
        if(argv[4][i]=='\0'){
            break;
        }
        sched[i]=argv[4][i];

    }

    if(*threads_num<1|| *queue_size<1){
        exit(1);
    }

}
void overloadhandler(int fd,Queue w_queue,Queue  r_queue, char* sched, struct timeval t){
    if(strcmp(sched, "block") == 0)
    {
           
        while((w_queue->members_num +r_queue->members_num) >= w_queue->m_size)
        {
            
            pthread_cond_wait(&full_queue, &waiting_q_lock);


        }

        Request request = NULL;
        if(createRequest(fd, t, &request) == -1)
            exit(-1);
        if(enqueue(w_queue, request) == -1)
            exit(-1);

          pthread_cond_signal(&empty_queue);
        pthread_mutex_unlock(&waiting_q_lock);

return;
}

      if(strcmp(sched, "bf") == 0)
    {
           
        while((w_queue->members_num +r_queue->members_num) != 0)
        {
            
            pthread_cond_wait(&full_queue, &waiting_q_lock);


        }
     Close(fd);
        pthread_mutex_unlock(&waiting_q_lock);
        return;
    }


    if(strcmp(sched, "dt") == 0)
    {

        Close(fd);
        pthread_mutex_unlock(&waiting_q_lock);
        return;
    }
    if(strcmp(sched, "dh") == 0)
    {

 
        Node n=NULL;
        n=dequeue(w_queue);
         if(n==NULL){
        exit(-1);
            }
        close(n->r->fd);
        
         


        Request request = NULL;
        if(createRequest(fd, t, &request) == -1)
            exit(-1);

        if(enqueue(w_queue, request) == -1)
            exit(-1);

            pthread_cond_signal(&empty_queue);
           pthread_mutex_unlock(&waiting_q_lock);
                 
           return;
          
 
    }

    if(strcmp(sched, "random")==0){
       Node temp=w_queue->root;
     int n=0;
     n=((w_queue->members_num+1)/2);

         srand(time(NULL));
     while(n!=0){
         if(w_queue->members_num ==0){break;}
        int x= rand()%(w_queue->members_num); 
         
        while(x!=0 ){
         temp=temp->next;
          x--;
         }

          int f=temp->r->fd;
         dequeue_fd(w_queue,temp->r->fd);
           close (f);
           temp=w_queue->root;
            n--;

      }
       Request request = NULL;
        if(createRequest(fd, t, &request) == -1)
            exit(-1);
        if(enqueue(w_queue, request) == -1)
            exit(-1);

          pthread_cond_signal(&empty_queue);
        pthread_mutex_unlock(&waiting_q_lock);




          }

}

int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen,threads_num,queue_size;
    char sched[8]={0};
    struct sockaddr_in clientaddr;
    Queue  running_queue=NULL;
    Queue waiting_queue=NULL;
    pthread_mutex_init(&waiting_q_lock,NULL);
    pthread_cond_init(&empty_queue,NULL);
    pthread_cond_init(&full_queue,NULL);
    getargs(&port, argc, argv,&threads_num,&queue_size,sched);
    if(init_queue(&running_queue,threads_num)==-1){
        return -1;
    }
    if(init_queue(&waiting_queue,(queue_size))==-1){
        return -1;
    } 
    for(int i=0;i<threads_num;i++){
        Thread thread=NULL;
        if(createThread(i,&thread,running_queue,waiting_queue)==-1){
           return -1;
        }
    }

    //

    listenfd = Open_listenfd(port);

    while (1) {
	clientlen = sizeof(clientaddr);
	connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);

    struct timeval  arrive;
        gettimeofday(&arrive, NULL);

       

        pthread_mutex_lock(&waiting_q_lock);
        if(running_queue->members_num + waiting_queue->members_num >= queue_size) {
            overloadhandler(connfd,waiting_queue,running_queue,sched,arrive);
        }
        else{
               
            Request r=NULL;
            if(createRequest(connfd,arrive,&r)==-1){
                return -1;
            }
             


                if(enqueue(waiting_queue,r)==-1){
                    return-1;

            }
                   // to let threads know there is an available request-Arrival:
               pthread_cond_signal(&empty_queue);

            pthread_mutex_unlock(&waiting_q_lock);
           
             
           




        }

	// 
	// HW3: In general, don't handle the request in the main thread.
	// Save the relevant info in a buffer and have one of the worker threads ``
	// do the work. 
	//
    //
    // HW3: Create some threads...

    }

}



//
// Created by Yosef Sabbah on 08/03/2024.
//





    


 
