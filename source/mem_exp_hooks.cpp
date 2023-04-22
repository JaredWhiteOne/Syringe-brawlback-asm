#include "mem_exp_hooks.h"
#include <mem/mem_expHeap.h>

namespace MemExpHooks {
    void initializeMemory(void* heapAddress, u32 heapSize) {
        mainHeap = MEMCreateExpHeapEx(heapAddress, heapSize, 0);
    }
    void* mallocExp(size_t size) {
        return MEMAllocFromExpHeapEx(mainHeap, size, 4);
    }
    void freeExp(void* ptr) {
        MEMFreeToExpHeap(mainHeap, ptr);
    }
}