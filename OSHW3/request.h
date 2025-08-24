#ifndef __REQUEST_H__
#include "Queue.h"

typedef struct thread{
    int m_id;
    int m_count;
    int m_static;
    int m_dynamic;
    pthread_t m_thread;
}* Thread;
void requestHandle(Request request, Thread t, Queue* wait_queue, pthread_mutex_t *queue_lock);

#endif
