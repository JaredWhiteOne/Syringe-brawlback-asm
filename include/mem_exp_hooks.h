#pragma once
#include <MEM/mem_heapCommon.h>

namespace MemExpHooks {
    static MEMHeapHandle mainHeap;
    void initializeMemory(void* heapAddress, u32 heapSize);
    void* mallocExp(size_t size);
    void freeExp(void* ptr);
    u32 getFreeSize(MEMHeapHandle heap, int alignment);
}