#include <unistd.h>
#include <string.h>

const size_t MAX_ALLOC = 100000000;

typedef struct BlockHeader {
    size_t blockSize;
    bool isFree;
    BlockHeader* nextBlock;
    BlockHeader* prevBlock;
} BlockHeader;

BlockHeader* head = nullptr;
size_t numOfAllocatedBlocks = 0;
size_t numOfAllocatedBytes = 0;
size_t numOfFreeBlocks = 0;
size_t numOfFreeBytes = 0;

void* smalloc(size_t size) {
   if (size == 0 || size > MAX_ALLOC) {
        return nullptr;
    }

    BlockHeader* iter = head;
    while (iter != nullptr) {
        if (iter->blockSize >= size && iter->isFree) {
            numOfFreeBlocks--;
            numOfFreeBytes -= iter->blockSize;
            iter->isFree = false;
            return (void*)((char*)iter + sizeof(BlockHeader));
        }
        iter = iter->nextBlock;
    }

    void* mem = sbrk(size + sizeof(BlockHeader));
    if (mem == (void*)-1) {
        return nullptr;
    }

    BlockHeader* newBlock = (BlockHeader*)mem;
    newBlock->blockSize = size;
    newBlock->isFree = false;
    newBlock->nextBlock = nullptr;

    if (head == nullptr) {
        head = newBlock;
        newBlock->prevBlock = nullptr;
    } else {
        BlockHeader* last = head;
        while (last->nextBlock != nullptr) {
            last = last->nextBlock;
        }
        last->nextBlock = newBlock;
        newBlock->prevBlock = last;
    }

    numOfAllocatedBlocks++;
    numOfAllocatedBytes += size;
    return (void*)((char*)newBlock + sizeof(BlockHeader));
}

void* scalloc(size_t num, size_t size) {
    void* memory = smalloc(num * size);
    if (memory == nullptr) {
        return nullptr;
    }
    memset(memory, 0, num * size);
    return memory;
}

void sfree(void* ptr) {
    if (ptr == nullptr) {
        return;
    }

    BlockHeader* blockToFree = (BlockHeader*)((char*)ptr - sizeof(BlockHeader));
    if (blockToFree->isFree) {
        return;
    }

    blockToFree->isFree = true;
    numOfFreeBytes += blockToFree->blockSize;
    numOfFreeBlocks++;
}

void* srealloc(void* oldPtr, size_t size) {
    if (size == 0 || size > MAX_ALLOC) {
        return nullptr;
    }

    if (oldPtr == nullptr) {
        return smalloc(size);
    }

    BlockHeader* oldBlock = (BlockHeader*)((char*)oldPtr - sizeof(BlockHeader));
    if (size <= oldBlock->blockSize) {
        return oldPtr;
    } else {
        void* newMemory = smalloc(size);
        if (newMemory == nullptr) {
            return nullptr;
        }
        memmove(newMemory, oldPtr, oldBlock->blockSize);
        sfree(oldPtr);
        return newMemory;
    }
}

size_t _num_free_blocks() {
    return numOfFreeBlocks;
}

size_t _num_free_bytes() {
    return numOfFreeBytes;
}

size_t _num_allocated_blocks() {
    return numOfAllocatedBlocks;
}

size_t _num_allocated_bytes() {
    return numOfAllocatedBytes;
}

size_t _num_meta_data_bytes() {
    return sizeof(BlockHeader) * numOfAllocatedBlocks;
}

size_t _size_meta_data() {
    return sizeof(BlockHeader);
}
