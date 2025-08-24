
#include <unistd.h>

const int MAX = 100000000;

void* smalloc(size_t size)
{
    if(size == 0 || size > MAX) {
        return NULL;
    }
    void* x = sbrk(size);
    if(*((int*)x) == -1) {
        return NULL;
    }
    return x;

}
