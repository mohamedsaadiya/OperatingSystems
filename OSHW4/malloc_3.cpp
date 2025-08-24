#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/mman.h>
using namespace std;

struct MallocMetadata
{
    size_t size;
    bool isFree;
    MallocMetadata *next;
    MallocMetadata *prev;
};
const int MAX_ORDER = 11;
const int MAX_NO_MAP_BLOCK = 128 * 1024;
MallocMetadata *mappedList = nullptr;
MallocMetadata *ordered_array[MAX_ORDER];
size_t num_free_blocks = 0;
size_t num_free_bytes = 0;
size_t num_allocated_blocks = 0;
size_t num_allocated_bytes = 0;
size_t num_meta_data_bytes = 0;

static bool is_first = true;

int get_order(int blockSize) {
    int order = 0;
    while (128 << order < blockSize) {
        ++order;
    }
    return order;
}

MallocMetadata *get_buddy(MallocMetadata* block)
{
    int level = get_order(block->size + sizeof(MallocMetadata));
    uintptr_t block_address = reinterpret_cast<uintptr_t>(block);
    uintptr_t buddy_address = block_address ^ (static_cast<uintptr_t>(128) << level);
    MallocMetadata* buddy_block = reinterpret_cast<MallocMetadata*>(buddy_address);
    return buddy_block;
}
void *first_allocation()
{
    void *temp = sbrk(0);
    long long adress = (long long)temp;
    int reminder = adress % (32 * 1024 * 128);
    if (sbrk(32 * 1024 * 128 - reminder) == (void *)(-1)){
        return nullptr;
    }
    void *pstart = sbrk(32 * 1024 * 128);
    if (pstart == (void *)(-1))
        return nullptr;
    char *start = (char *)pstart;
    if ((void *)start == (void *)(-1))
        return (void *)(-1);
    for (int i = 0; i < MAX_ORDER; i++)
    {
        ordered_array[i] = nullptr;
    }
    ordered_array[MAX_ORDER - 1] = (MallocMetadata *)start;

    MallocMetadata *it = (MallocMetadata *)start;
    it->size = 128 * 1024;
    it->prev = nullptr;
    it->isFree = true;
    num_free_blocks = 32;
    num_allocated_blocks = 32;
    num_free_bytes = 1024 * 128 * 32 - 32 * sizeof(MallocMetadata);
    num_allocated_bytes = 1024 * 128 * 32 - 32 * sizeof(MallocMetadata);
    num_meta_data_bytes = 32 * sizeof(MallocMetadata);
    for (int i = 1; i < 32; i++)
    {
        start += 1024 * 128;
        MallocMetadata *temp = (MallocMetadata *)(start);
        if (i == 31)
            temp->next = NULL;
        temp->prev = it;
        it->next = temp;
        temp->size = 128 * 1024 - sizeof(MallocMetadata);
        temp->isFree = true;
        it = temp;
    }
    return pstart;
}

void *map_block(size_t size)
{
    void* mem = mmap(nullptr, size + sizeof(MallocMetadata), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mem == MAP_FAILED) {
        return nullptr;
    }

    num_allocated_blocks++;
    num_meta_data_bytes += sizeof(MallocMetadata);
    num_allocated_bytes += size;

    MallocMetadata* meta = reinterpret_cast<MallocMetadata*>(mem);
    meta->next = mappedList;
    meta->prev = nullptr;
    meta->isFree = false;
    meta->size = size;

    if (mappedList) {
        mappedList->prev = meta;
    }

    mappedList = meta;
    return static_cast<char*>(mem) + sizeof(MallocMetadata);
}

void *allocate(int order)
{
    MallocMetadata *current = ordered_array[order];
    ordered_array[order] = current->next;
    if (current->next)
        current->next->prev = current->prev;
    current->prev = NULL;
    current->isFree = false;
    num_free_blocks--;
    num_free_bytes -= current->size;
    return (char *)current + sizeof(MallocMetadata);
}

void slice(int order)
{
    int temp = order;

    while (!ordered_array[temp] && temp <= 10)
        temp++;
    if (temp == MAX_ORDER && !ordered_array[temp])
    {
        return;
    }
    while (temp != order && ordered_array[temp])
    {
        num_free_blocks++;
        num_allocated_blocks++;
        num_allocated_bytes -= sizeof(MallocMetadata);
        num_free_bytes -= sizeof(MallocMetadata);
        num_meta_data_bytes += sizeof(MallocMetadata);
        
        MallocMetadata *tempData = ordered_array[temp];
        tempData->size = pow(2, temp - 1) * 128 - sizeof(MallocMetadata); 
        MallocMetadata *myBuddy = get_buddy(tempData);
        myBuddy->size = tempData->size;
        MallocMetadata *big = ordered_array[temp];
        ordered_array[temp] = big->next;
        if (big->next)
            big->next->prev = NULL;
        ordered_array[temp - 1] = tempData;
        ordered_array[temp - 1]->next = myBuddy;
        myBuddy->prev = tempData;
        tempData->prev = NULL;
        tempData->isFree = true;
        myBuddy->next = NULL;
        myBuddy->isFree = true;
        temp = temp - 1;
    }
}

void *smalloc(size_t size)
{

    if (is_first)
    {
        if ((void *)(-1) == first_allocation())
            return NULL;
        is_first = false;
    }
    if (size == 0 || size > 100000000)
        return NULL;
    if (size + sizeof(MallocMetadata) > MAX_NO_MAP_BLOCK)
    {
        return map_block(size);
    }
    int order = get_order(size + sizeof(MallocMetadata));
    MallocMetadata *current = ordered_array[order];
    if (!current)
    {
        slice(order);
    }
    if (!ordered_array[order])
        return NULL;
    return allocate(order);
}

void *scalloc(size_t num, size_t size)
{
    int newsize = num * size;
    void *address = smalloc(newsize);
    if (!address)
        return NULL;

    memset(address, 0, newsize);
    return address;
}

void addToArray(int order, MallocMetadata *temp)
{
    MallocMetadata *cur = ordered_array[order];

    if (!cur)
    {
        ordered_array[order] = temp;
        temp->next = NULL;
        temp->prev = NULL;
        return;
    }
    if (cur > temp)
    {
        ordered_array[order] = temp;
        temp->next = cur;
        cur->prev = temp;
        temp->prev = NULL;
        return;
    }
    while ((cur < temp) && cur && cur->next)
    {
        cur = cur->next;
    }

    if ((!(cur->next)) && (cur < temp))
    {
        temp->prev = cur;
        cur->next = temp;
        temp->next = NULL;
        return;
    }


    cur->prev->next = temp;
    temp->prev = cur->prev;
    temp->next = cur;
    cur->prev = temp;
}
void join(MallocMetadata *temp, int order)
{
    MallocMetadata *myBuddy = get_buddy(temp);
    if (!myBuddy || myBuddy->isFree == false || order == MAX_ORDER - 1 || myBuddy->size != temp->size)
    {
        addToArray(order, temp);
        return;
    }
    if (myBuddy->prev)
    {
        myBuddy->prev->next = myBuddy->next;
    }
    else
    {
        ordered_array[order] = myBuddy->next;
    }
    if (myBuddy->next)
    {
        myBuddy->next->prev = myBuddy->prev;
    }
    if (myBuddy < temp)
    {
        temp = myBuddy;
    }
    temp->size = pow(2, order + 1) * 128 - sizeof(MallocMetadata);
    temp->isFree = true;
    num_free_bytes += sizeof(MallocMetadata);
    num_free_blocks--;
    num_meta_data_bytes -= sizeof(MallocMetadata);
    num_allocated_blocks--;
    num_allocated_bytes += sizeof(MallocMetadata);
    join(temp, order + 1);
}

void sfree(void* ptr) {
    if (!ptr) {
        return;
    }

    MallocMetadata* block = reinterpret_cast<MallocMetadata*>(static_cast<char*>(ptr) - sizeof(MallocMetadata));
    if (block->isFree) {
        return;
    }

    if (block->size + sizeof(MallocMetadata) > MAX_NO_MAP_BLOCK) {
        num_allocated_blocks--;
        num_allocated_bytes -= block->size;
        num_meta_data_bytes -= sizeof(MallocMetadata);

        if (block->next) {
            block->next->prev = block->prev;
        }

        if (block->prev) {
            block->prev->next = block->next;
        } else {
            mappedList = block->next;
        }

        munmap(block, block->size + sizeof(MallocMetadata));
        return;
    }

    num_free_bytes += block->size;
    int blockOrder = get_order(block->size + sizeof(MallocMetadata));
    MallocMetadata* buddy = get_buddy(block);

    if (buddy && buddy->isFree && blockOrder != MAX_ORDER - 1 && buddy->size == block->size) {
        block->isFree = true;
        join(block, blockOrder);
    } else {
        block->isFree = true;
        addToArray(blockOrder, block);
    }

    num_free_blocks++;
}


int can_merge(MallocMetadata *block, int requiredSize)
{
    MallocMetadata *buddyBlock = get_buddy(block);
    int mergeCount = 1;
    int blockOrder = get_order(block->size);
    int buddyOrder = get_order(buddyBlock->size);

    while (buddyBlock != nullptr && buddyBlock->isFree && buddyOrder == blockOrder)
    {
        int potentialSize = (1 << (blockOrder + 1)) * 128 - sizeof(MallocMetadata);
        if (potentialSize >= requiredSize)
        {
            return mergeCount;
        }

        blockOrder++;
        mergeCount++;

        uintptr_t baseAddr;
        if (block < buddyBlock)
        {
            baseAddr = reinterpret_cast<uintptr_t>(block);
        }
        else
        {
            baseAddr = reinterpret_cast<uintptr_t>(buddyBlock);
        }

        uintptr_t newAddr = baseAddr ^ static_cast<uintptr_t>((1 << blockOrder) * 128);
        buddyBlock = reinterpret_cast<MallocMetadata *>(newAddr);
        buddyOrder = get_order(buddyBlock->size);
    }

    return -1;
}

void joinLimited(MallocMetadata *block, int level, int remainingMerges)
{
    MallocMetadata *buddy = get_buddy(block);

    if (buddy == nullptr || !buddy->isFree || level >= MAX_ORDER - 1 || buddy->size != block->size || remainingMerges == 0)
    {
        return;
    }

    if (buddy->prev != nullptr)
    {
        buddy->prev->next = buddy->next;
    }
    else
    {
        ordered_array[level] = buddy->next;
    }

    if (buddy->next != nullptr)
    {
        buddy->next->prev = buddy->prev;
    }

    block = (buddy < block) ? buddy : block;

    block->size = (1 << (level + 1)) * 128 - sizeof(MallocMetadata);
    block->isFree = false;

    num_free_bytes += sizeof(MallocMetadata);
    num_free_blocks--;
    num_meta_data_bytes -= sizeof(MallocMetadata);
    num_allocated_blocks--;
    num_allocated_bytes += sizeof(MallocMetadata);

    joinLimited(block, level + 1, remainingMerges - 1);
}

void* srealloc(void* oldPtr, size_t newSize) {
    if (newSize == 0 || newSize > 100000000) {
        return nullptr;
    }

    if (!oldPtr) {
        return smalloc(newSize);
    }

    MallocMetadata* currentBlock = reinterpret_cast<MallocMetadata*>(static_cast<char*>(oldPtr) - sizeof(MallocMetadata));

    if (currentBlock->size + sizeof(MallocMetadata) > MAX_NO_MAP_BLOCK) { 
        if (newSize == currentBlock->size) {
            return oldPtr;
        }
        void* newMem = smalloc(newSize);
        if (!newMem) {
            return nullptr;
        }
        memmove(newMem, oldPtr, currentBlock->size); 
        sfree(oldPtr);
        return newMem;
    }

    if (newSize <= currentBlock->size) {
        return oldPtr;
    }

    int mergeCount = can_merge(currentBlock, newSize);
    if (mergeCount != -1) {
        int blockOrder = get_order(currentBlock->size + sizeof(MallocMetadata));
        num_free_bytes += currentBlock->size;

        MallocMetadata* buddyBlock = get_buddy(currentBlock);
        joinLimited(currentBlock, blockOrder, mergeCount);

        if (buddyBlock < currentBlock) {
            memmove(static_cast<char*>(reinterpret_cast<void*>(buddyBlock)) + sizeof(MallocMetadata), oldPtr, currentBlock->size);
            num_free_bytes -= buddyBlock->size;
            return static_cast<char*>(reinterpret_cast<void*>(buddyBlock)) + sizeof(MallocMetadata);
        }

        num_free_bytes -= currentBlock->size;
        return oldPtr;
    }

    void* newBlock = smalloc(newSize);
    if (!newBlock) {
        return nullptr;
    }

    memmove(newBlock, oldPtr, currentBlock->size);
    sfree(oldPtr);
    return newBlock;
}    

size_t _num_free_blocks()
{
    return num_free_blocks;
}

size_t _num_free_bytes()
{
    return num_free_bytes;
}
size_t _num_allocated_blocks()
{
    return num_allocated_blocks;
}
size_t _num_allocated_bytes()
{
    return num_allocated_bytes;
}
size_t _num_meta_data_bytes()
{
    return num_meta_data_bytes;
}
size_t _size_meta_data()
{
    return sizeof(MallocMetadata);
}
